#include "TcpServer.h"

TcpServer::TcpServer(uint16_t num_workers, std::string ip, uint16_t port, bool b_enable): 
    m_numWorker(num_workers), m_ipAddr(ip), m_port(port), m_bloomFilterEnable(b_enable){
}

TcpServer::~TcpServer(){
    //printf("Destroying TCP server\n");
}

void TcpServer::initWorkerManager(){
    m_eventDispatcher.reset(new WorkerManager(m_numWorker, WorkerManager::ROUND_ROBIN));
    m_eventDispatcher->initWorkers();
}

/*
 *@brief this function starts the TCP listener with libevent library used for polling FD
 * */
void TcpServer::start(){
    printf("TcpServer::start %ld\n", (long)pthread_self());
    initWorkerManager();

    try{
        ServiceThread *listener = new (std::nothrow) Listener(m_ipAddr, m_port,
                std::bind(&TcpServer::ServerCallback,
                this, std::placeholders::_1));
        assert(listener != nullptr);
        m_tcpListener.reset(listener);
        m_tcpListener->start();
    }catch(...){
        printf("failed to start TCP server\n");
        return;
    }

    Dictionary *dict = Dictionary::getInstance();
    dict-> enableBloomFilter(m_bloomFilterEnable);
}

/*
 *@brief dispatch the client FD to worker thread to handle a particular client
 *@param client file descriptor
 * */
void TcpServer::ServerCallback(int client_fd){
    //printf("TcpServer::ServerCallback\n");
    m_eventDispatcher->dispatchFd(client_fd);
}

void TcpServer::shutdown(){
    printf("shutting down TCP server ...\n");
    if(m_tcpListener){
        if(!m_tcpListener->stop()){
            printf("Failed to stop TCP server thread\n");
        }
    }

    if(m_eventDispatcher){
        m_eventDispatcher->stopWorkers();
    }
}
