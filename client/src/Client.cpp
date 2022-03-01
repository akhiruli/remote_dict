#include <Client.h>
#include <chrono>

Client::Client(std::string ip, uint32_t port): m_ip(ip), m_port(port){
}

Client::~Client(){

}

/*
 *@brief entry point of the client thread, it sends request to the server
 * */
void Client::run(){
    printf("Starting the client worker %ld\n", (long)pthread_self());
    for(auto idx = 0; idx < m_inputdata.size(); idx++){
        if(!this->startClient()){
            continue;
        }
        this->sendData(m_inputdata[idx]);
    }

    printf("Done sending all the request to the server from the worker %ld\n", (long)pthread_self());
}

/*
 *@brief It starts the client and connects to the server
 *@return true for success and false for failure
 * */
bool Client::startClient(){
    struct sockaddr_in servaddr;

    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1) {
        printf("socket creation failed... error= %s\n", strerror(errno));
        return false;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(m_ip.c_str());
    servaddr.sin_port = htons(m_port);

    if (connect(m_fd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed... error= %s\n", strerror(errno));
        return false;
    }
    //else
      //  printf("connected to the server..\n");

    return true;
}

/*
 *@brief sending data to the server
 *@param message to be send to the server
 *@return 0 for success amnd -1 for failure
 * */
int Client::sendData(MessageT& msg){
    int write_len = 0;
    Payload payload;

    payload.set_requesttype(Payload_RequestType(msg.type));
    if(msg.type != MessageType::STAT)
        payload.set_key(msg.key);
    if(msg.type == MessageType::SET){
        if(msg.value.empty()){
            printf("For SET operation, you have to provide the value \n");
            return -1;
        }
        payload.set_value(msg.value);
    }

    uint32_t size = payload.ByteSizeLong();
    Byte *buffer = (Byte* )calloc(1, size);
    payload.SerializeToArray(buffer, size);
    Byte *send_buffer = (Byte* )calloc(1, size + 4);;
    memcpy(send_buffer, &size, 4);
    memcpy(send_buffer + 4, buffer, size);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int len =  write(m_fd, (Byte *)send_buffer, size + 4);

    if(len < 0){
        printf("Failed to send data to server with error %s\n", strerror(errno));
        if(write_len == 0)
            return -1;
        else
            return write_len;
    }

    write_len += len;
    free(buffer);
    free(send_buffer);

    char buff[MAX] = {'\0'};
    int read_len = read(m_fd, buff, sizeof(buff));
    if(read_len < 0){
        printf("Failed to read the response from server\n");
        return write_len;
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    Payload recv_data;
    recv_data.ParseFromArray(buff, read_len);

    ResultData data;
    data.payload = recv_data;
    data.timestamp = std::chrono::duration_cast<std::chrono::microseconds >(std::chrono::system_clock::now().time_since_epoch()).count();
    m_results.push_back(data);

    close(m_fd);

    uint32_t duration_micro = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    if(m_averageLatency == 0){
        m_averageLatency = duration_micro;
    } else{
        m_averageLatency = (m_averageLatency + duration_micro)/2;
    }
    return write_len;
}
