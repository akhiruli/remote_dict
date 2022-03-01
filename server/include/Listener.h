#ifndef _LISTENER_H_
#define _LISTENER_H_

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>

#include "ServiceThread.h"

class Listener: public ServiceThread{
    private:
        std::string               m_ipAddr;
        uint16_t                  m_port;
        int                       m_fileDescriptor;
        std::function<void (int)> m_callback;
        struct event              m_notifEvent;
    public:
        Listener(std::string ip, uint16_t port, std::function<void (int)> cb);
        Listener(const Listener& ) = delete;
        ~Listener();
        Listener& operator=(const Listener&) = delete;

        void initApp() override;
        void cleanState() override;
        int prepareTcpSvr();
        static void onAccept(int fd, short ev, void *arg);
        void initWorkerManager();
};
#endif
