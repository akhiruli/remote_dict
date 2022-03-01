#include "WorkerManager.h"

WorkerManager::WorkerManager(int threadCount, LoadDistributionType distType):
    m_threadCount(threadCount), m_index(0), m_distType(distType){
}
/*
 *@brief a destructor to clean all the stats of WorkerManager class
 * */
WorkerManager::~WorkerManager(){
    //printf("Destroyed of worker manager object\n");
}

/*
 *@brief it creates worker thread and the queue for inter thread communication
 * */
void WorkerManager::initWorkers(){
    //printf("WorkerManager::initWorkers count %d\n", m_threadCount);
    for(auto i = 0; i <m_threadCount; i++){
        std::shared_ptr<ServiceThread> svc_worker(new (std::nothrow) ServiceWorker());
        svc_worker->start();
        m_workers.push_back(svc_worker);
    }
}

/*
 *@brief send the client file descriptor to one of the worker thread
 *@param fd, file descriptor
 * */
void WorkerManager::dispatchFd(int fd){
    //printf("WorkerManager::dispatchFd\n");
    loadbalanceFd(fd);
}

/*
 *@brief load balncing the client FD to one of the worker thread based on the load balancing algorithm choosen
 *@param fd client file descriptor
 * */

void WorkerManager::loadbalanceFd(int fd){
    int index = getWorkerIndex();
    if(index < 0 || index >= m_threadCount){
        printf("Wrong worker index\n");
        return;
    }

    std::shared_ptr<ServiceThread> worker = m_workers[index];
    if(!worker){
        printf("worker is NULL\n");
        return;
    }

    worker->addFdToEvLoop(fd); // adding FD to event loop of one of the worker threads
}

/*
 *@brief finding the index of the nex worker thread to be chosen based on the algorithm selected
 *@return index of worker thread from the thread pool
 * */
uint32_t WorkerManager::getWorkerIndex(){
    uint32_t ret_index = 0;
    if(m_distType == ROUND_ROBIN){
        ++m_index;
        ret_index = m_index % m_threadCount;
        m_index = ret_index;
    } else if(m_distType == DIST_LESS_LOADED_WORKER){ //this case not being used currently
        int index = 0;
        int min = INT_MAX;
        for(unsigned int i = 0; i < m_workers.size(); i++){
            int queue_size = m_workers[i]->getCurrentFdQueueSize();
            if(queue_size < min){
                min = queue_size;
                index = i;
            }
        }
        ret_index = index;
    } else {
        printf("load balancing algorithm is missing %d\n", m_distType);
        ret_index = INT_MAX;
    }

    return ret_index;
}

/*
 *@brief stopping worker threads
 * */
void WorkerManager::stopWorkers(){
    printf("Stopping worker threads\n");
    for(auto worker : m_workers) {
        if(!worker->stop()){
            printf("Failed to stop worker thread\n");
        }
    }

    m_workers.clear();
}
