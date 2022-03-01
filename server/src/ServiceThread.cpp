#include "ServiceThread.h"

std::mutex ServiceThread::m_threadStopMtx;
std::condition_variable ServiceThread::m_threadStopCondVar;

ServiceThread::ServiceThread(): m_processExiting(false), m_threadId(0){
    m_exited.store(false);
}

ServiceThread::~ServiceThread(){
}

/*
 *@brief this method starts a thread in detach mode
 * */
void ServiceThread::start(bool spawnThread){
    //printf("ServiceThread::start\n");
    this->m_spawnThread = spawnThread;
    m_threadCtx.reset(new (std::nothrow) ThreadContextT());
    if(!spawnThread){
        this->run();
    }else{
        std::shared_ptr<std::thread> th_obj(new (std::nothrow) std::thread(&ServiceThread::run, this));
        th_obj->detach();
    }
}

/*
 *@brief starting point of worker threads
 *@param queue a notfication queue to poll the client FD
 * */
void ServiceThread::run(){
    m_threadId = (long)pthread_self();
    printf("created a thread with threadId %ld\n", m_threadId);

    
    m_threadCtx->m_libEvent = new (std::nothrow) EventImpl(); 
    if(!m_threadCtx->m_libEvent){
        printf("Failed to initialize libevent\n");
        return;
    }

    if(!m_threadCtx->m_libEvent->initEv()){
        printf("Failed to initiate lib event\n");
        return;
    }

    EventImpl *libev = m_threadCtx->m_libEvent;

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, m_threadCtx->m_sockets) < 0) {
        printf("Failed opening stream socket pair with error %s\n", strerror(errno));
        return;
    }

    Helper::setnonblock(m_threadCtx->m_sockets[0]);
    Helper::setnonblock(m_threadCtx->m_sockets[1]);

    libev->attachFdtoEv(&m_threadCtx->m_socketPairEvent, m_threadCtx->m_sockets[1], ServiceThread::onEvent, this);
    libev->addToEv(&m_threadCtx->m_socketPairEvent);

    //This function call is for derived class, if it wants to do some job before event loop starts
    //For example, the TCP server threads wants to add a FD (TCP listener) to event  loop
    this->initApp();

    libev->dispatchEvloop();

    printf("Exited service thread %ld\n", m_threadId);

    m_exited.store(true);
    std::lock_guard<std::mutex> lock(m_threadStopMtx);
    m_threadStopCondVar.notify_one();
}

/*
 *@brief returning current client FD queue size
 *@return returning unsigned integer queue size
 * */
uint32_t ServiceThread::getCurrentFdQueueSize(){
    std::lock_guard<std::mutex> lock(m_threadCtx->m_threadCtxMtx);
    return (m_threadCtx->m_queue).size();
}

/*
 * @brief this method is for adding an event to the event loop. It should be called from same the thread
 * @param an event structure pointer
 * @param file descriptor to be added in the event loop
 * @param event handler function pointer
 * @param cookie to be returned as a part of the callback
 * */
void ServiceThread::addEvent(struct event *event, int fd,
        event_callback_fn cbk, void *arg){
    m_threadCtx->m_libEvent->attachFdtoEv(event, fd, cbk, arg);
    m_threadCtx->m_libEvent->addToEv(event);
}

/*
 *@brief deleting an event from the event loop. It should be called from the same thread
 *@param a libevent structure pointer
 * */
void ServiceThread::deleteEvent(struct event *l_event){
    m_threadCtx->m_libEvent->removeFromEv(l_event);
}

/*
 *@brief this method is used as a trigger to stop the thread
 *@return true for success, otherwise false
 * */
bool ServiceThread::stop(){
    char buf[1] = {'e'};
    bool ret = true;
    m_processExiting.store(true);
    //Using socketpair to wake up(by sending an event) the worker thread to break the loop (libevent)
    if(write(m_threadCtx->m_sockets[0], buf, sizeof(buf)) != 1){
        printf("Failed write in the socket pair while shutting down\n");
        ret = false;
    } else{
        std::unique_lock<std::mutex> lock(m_threadStopMtx); //no need to unlock the mutex as wait internally release the lock and make the thread to block
        m_threadStopCondVar.wait(lock, [&]{ return m_exited.load(); });
        printf("The service thread %ld is terminated\n", m_threadId);
    }

    return ret;
}

/*
 *@brief this fucntion breaks the event loop to exit the thread
 * */
void ServiceThread::stopThread(){
    m_threadCtx->m_libEvent->exitLoop();
}

/*
 *@brief event callback for the read event in the socket pair. This event gets generated when
 *tcp listener thread write in one end of the socket pair
 *@param file descriptor
 *@param event type
 *@param cookie
 * */
void ServiceThread::onEvent(int fd, short ev, void *arg){
    ServiceThread *_svc = reinterpret_cast<ServiceThread *>(arg);
    char buf[1];
    if(read(fd, buf, 1) != 1){
        printf("Can't read from libevent pipe with error %s\n", strerror(errno));
        return;
    }

    if(buf[0] == 'e'){
        printf("Received signal to terminate worker\n");
        _svc->cleanState();
        _svc->stopThread();
        return;
    }

    int file_descriptor = -1;
    {
        std::lock_guard<std::mutex> lock(_svc->getThreadCtx()->m_threadCtxMtx);
        while((_svc->getThreadCtx()->m_queue).size() > 0){
            file_descriptor = (_svc->getThreadCtx()->m_queue).front();
            //Actually we don't need lock for processEvent function. This function is used only for adding the FD to an event loop.
            //To make the code simple we kept the function under the lock as it does not have much overhead.
            _svc->processEvent(file_descriptor);
            (_svc->getThreadCtx()->m_queue).pop_front();
        }
    }

    return;
}
/*
 *@brief this method is used as a trigger to add an FD to a event loop of this thread from a different thread
 *@param client file descriptor
 *@return True for success, otherwise false
 * */
bool ServiceThread::addFdToEvLoop(int fd){
    char buf[1] = {'c'};

    //Using socketpair to wake up (by sending an event) the worker thread to pick the client FD from the queue
    std::lock_guard<std::mutex> lock(m_threadCtx->m_threadCtxMtx);

    if(write(m_threadCtx->m_sockets[0], buf, sizeof(buf)) != 1){
        printf("Failed write in the socket pair to wake the worker thread"
                " up that picks the client FD from the queu with error %s\n", strerror(errno));
        close(fd);
        return false;
    }

    (m_threadCtx->m_queue).push_back(fd);
    return true;
}

