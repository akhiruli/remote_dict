#include "Listener.h"

Listener::Listener(std::string ip, uint16_t port, std::function<void (int)> cb):m_ipAddr(ip),
    m_port(port), m_fileDescriptor(-1), m_callback(cb){

    }

Listener::~Listener(){

}

/*
 *@brief this function starts the listener service
 * */
void Listener::initApp(){
    printf("Initializing TCP server\n");
    if(prepareTcpSvr()){
        printf("Failed to create socket\n");
        throw std::runtime_error("Failed to create listener socket");
    }

    addEvent(&m_notifEvent, m_fileDescriptor, onAccept, this);
}

/*
 *@brief perparing for the listener socket to create a TCP server
 * */
int Listener::prepareTcpSvr(){
    struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;

    m_fileDescriptor = socket(PF_INET, SOCK_STREAM, 0);
    if (m_fileDescriptor < 0) {
        printf("Listener::prepareTcpSvr: listen failed\n");
        return -1;
    }

    if (setsockopt(m_fileDescriptor, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on, sizeof(reuseaddr_on)) == -1){
        printf("Listener::prepareTcpSvr: setsockopt failed\n");
        return -1;
    }

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr(m_ipAddr.c_str());
    listen_addr.sin_port = htons(m_port);

    if (bind(m_fileDescriptor, (struct sockaddr *)&listen_addr,
                sizeof(listen_addr)) < 0){
        printf("Listener::prepareTcpSvr: bind failed\n");
        return -1;
    }

    printf("TCP server is listening with backlog %d\n", SOMAXCONN);
    if (listen(m_fileDescriptor, SOMAXCONN) < 0){
        printf("Listener::prepareTcpSvr: failed to listen\n");
        return -1;
    }

    /* Set the socket to non-blocking, this is essential in event
     * based programming with libevent. */
    if (Helper::setnonblock(m_fileDescriptor) < 0){
        printf("Listener::prepareTcpSvr: failed to set server socket to non-blocking\n");
        return -1;
    }

    return 0;
}

/*
 *@brief callback function after accepting a client FD
 *@param fd file dexriptor of a client
 *@param ev event type
 *@param arg a cookie
 * */
void Listener::onAccept(int fd, short ev, void *arg)
{
    if(ev != EV_READ){
        printf("Listener::onAccept: Wrong event %d is received\n", ev);
        return;
    }

    //printf("Listener::onAccept client with fd %d thread_id %ld\n", fd, (long)pthread_self());
    Listener *server = reinterpret_cast<Listener *>(arg);

    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* Accept the new connection. */
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        printf("Listener::onAccept:accept failed with error %s\n", strerror(errno));
        return;
    }

    /* Set the client socket to non-blocking mode. */
    if (Helper::setnonblock(client_fd) < 0){
        printf("Listener::onAccept:failed to set client socket non-blocking\n");
        return;
    }

    //printf("Accepted sokcet with fd %d\n", client_fd);

    server->m_callback(client_fd);

    return;
}

/*
 *@brief removes the listener socket from event loop and close the FD
 * */
void Listener::cleanState(){
    printf("closing TCP server FD %d\n", m_fileDescriptor);
    deleteEvent(&m_notifEvent);
    close(m_fileDescriptor);
}
