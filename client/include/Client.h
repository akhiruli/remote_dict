#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string> 
#include <vector>
#include <thread>
#include <chrono>

#include "messagebody.pb.h"

#define MAX 80 
#define SA struct sockaddr 

using Byte = unsigned char;

enum class MessageType{
    GET = 1,
    SET = 2,
    STAT = 3
};

typedef struct Message{
    MessageType type;
    std::string key;
    std::string value;
    Message(){
        key = "";
        value = "";
    }
}MessageT;

typedef struct _resultData{
    Payload payload;
    uint64_t timestamp;
    _resultData(){
    timestamp = 0;
    }
}ResultData;

class Client{
    public:
        Client(std::string ip, uint32_t port);
        Client(const Client&) = delete;
        Client& operator=(const Client& ) = delete;

        ~Client();

        bool startClient();
        int sendData(MessageT& msg);
        void run();
        void setInputData(std::vector<MessageT> inputdata){
            this->m_inputdata = inputdata;
        }
        std::vector<ResultData> getResults(){
            return m_results;
        }
        uint32_t getAverageLatency(){
            return m_averageLatency;
        }
    private:
        int         m_fd;
        std::string m_ip;
        uint32_t    m_port;
        uint32_t    m_averageLatency;
        std::vector<MessageT> m_inputdata;
        std::vector<ResultData>  m_results;
};

#endif
