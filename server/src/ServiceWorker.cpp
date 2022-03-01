#include <unistd.h>
#include <thread>
#include <memory>

#include "ServiceWorker.h"

ServiceWorker::ServiceWorker(){
}

/*
 *@brief destructor to cleanup everything
 * */
ServiceWorker::~ServiceWorker(){
    //Removing the TCP session if any while the worker thread goes down
    m_tcpSessions.clear();
    printf("Destruction of ServiceWorker ended\n");
}

/*
 *@brief converting the address of a memory in string format
 *@param pointer to a TcpFlow object
 *@return returning address of an object in string format
 * */
std::string ServiceWorker::getAddrStr(TcpFlow *flow){
    const void * address = static_cast<const void*>(flow);
    std::stringstream ss;
    ss << address;
    return ss.str();
}

/*
 *@brief this fucntion breaks the even loop to exit the worker thread
 * */
void ServiceWorker::cleanState(){
    printf("Cleaning the tcp sessions\n");
    m_tcpSessions.clear();
}


/*
 *@brief this function is used to add client FD to event loop and creating the TcpFlow
 *@param client file descriptor(FD)
 * */
void ServiceWorker::processEvent(int fd){
    //printf("ServiceWorker::processEvent for FD %d\n", fd);
    if(m_processExiting){
        printf("process exiting\n");
        return;
    }

    TcpFlow *flow = new TcpFlow(std::bind(&ServiceWorker::deleteTcpFlow, this, std::placeholders::_1));
    std::string addr = getAddrStr(flow);
    flow->setAddr(addr);
    std::shared_ptr<TcpFlow> sp(flow);
    m_tcpSessions.insert({addr, sp});

    addEvent(&flow->m_clientEvent, fd, TcpFlow::data, flow);
}

/*
 *@brief this function is to delete a TCP flow from the flow table
 *@param tcpFlow pointer to tcp flow class
 * */
void ServiceWorker::deleteTcpFlow(std::string tcpFlow){
    //printf("ServiceWorker::deleteTcpFlow %s\n", tcpFlow.c_str());
    m_tcpSessions.erase(tcpFlow);
}
