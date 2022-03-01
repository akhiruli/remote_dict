#ifndef _SVC_WORKER_H_
#define _SVC_WORKER_H_

#include <unordered_map>

#include "TcpFlow.h"
#include <ServiceThread.h>

class ServiceWorker : public ServiceThread{
    using TcpSessions = std::unordered_map<std::string, std::shared_ptr<TcpFlow>>;
    private:
        TcpSessions   m_tcpSessions;
        std::string   getThreadId();
        std::string   getAddrStr(TcpFlow *);
    public:
        ServiceWorker();
        ~ServiceWorker();
        void processEvent(int fd) override;
        void deleteTcpFlow(std::string);
        void cleanState() override;
};
#endif
