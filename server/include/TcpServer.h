#ifndef _H_TCP_SVR_H_
#define _H_TCP_SVR_H_

#include <memory>
#include <thread>

#include <ServiceThread.h>
#include <WorkerManager.h>
#include <Listener.h>

class TcpServer{
    private:
        std::shared_ptr<ServiceThread>       m_tcpListener;
        std::shared_ptr<WorkerManager>       m_eventDispatcher;
        uint16_t                             m_numWorker;
        std::string                          m_ipAddr;
        uint16_t                             m_port;
        bool                                 m_bloomFilterEnable;
    public:
        TcpServer(uint16_t num_workers, std::string ip, uint16_t port, bool b_enable);
        TcpServer(const TcpServer&) = delete;
        ~TcpServer();
        void start();
        void ServerCallback(int client_fd);
        void initWorkerManager();
        void shutdown();
};
#endif
