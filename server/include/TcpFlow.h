#ifndef _TCP_FLOW_H_
#define _TCP_FLOW_H_

#include <chrono>
#include <functional>
#include <iostream>

#include <EventImpl.h>
#include "messagebody.pb.h"
#include "Stats.h"
#include "Dictionary.h"

#define NUM_READ_TRY 50
#define BUFLEN 1200

using Byte = unsigned char;
using ChronoTimePointT = std::chrono::high_resolution_clock::time_point;

class TcpFlow{
    private:
        std::function<void (std::string)> m_deleteCbk;
        int                            m_clientFd;
        std::string                    m_selfAddr;
        ChronoTimePointT               m_flowStartTime;
        Payload                        m_payload;
        void printFlowCompletionTime();
        void handleGetRequest();
        void  handleSetRequest();
        void handleStatRequest();
    public:
        EventImpl         *m_libEvent;
        struct event      m_clientEvent;
        bool              m_pollAgain;
        Byte*             m_data;
        uint32_t          m_payloadLen;
        uint32_t          m_lenTillnow;
        uint32_t          m_currlen;
        TcpFlow(std::function<void (std::string)> cb);
        ~TcpFlow();
        static void data(int fd, short ev, void *arg);
        void parse();
        void sendResponse();
        int socketWriteData(Byte *buf, size_t buf_len);
        void setLibEvent(EventImpl *lib_event){
            this->m_libEvent = lib_event;
        }
        int getClientFd(){
            return m_clientFd;
        }

        EventImpl* getLibEvent(){
            return m_libEvent;
        }

        void setAddr(std::string addr){
            m_selfAddr = addr;
        }

        std::string getAddr(){
            return m_selfAddr;
        }
};

#endif
