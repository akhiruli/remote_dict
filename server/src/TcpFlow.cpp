#include "TcpFlow.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

TcpFlow::TcpFlow(std::function<void (std::string)> cb): m_deleteCbk(cb), m_clientFd(-1),
    m_libEvent(nullptr), m_pollAgain(false), m_data(nullptr), m_payloadLen(0),
    m_lenTillnow(0){
        m_flowStartTime = std::chrono::high_resolution_clock::now();
}

/*
 *@brief TcpFlow destructor to cleanup tcp flow class related data
 * */
TcpFlow::~TcpFlow(){
    //printf("Destroying TCP flow \n");
    m_libEvent->removeFromEv(&m_clientEvent);;
    close(m_clientFd);

    this->printFlowCompletionTime();
}

/*
 *@brief This function prints total duration of the flow
 * */
void TcpFlow::printFlowCompletionTime(){
    ChronoTimePointT end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> time_span = end_time - m_flowStartTime;

    printf("[STAT: flow completion time=%lf milliseconds]\n", time_span.count());
}

/*
 *@brief function callback to get notification whenever client FD has some data available to read
 *@param fd file descriptor
 *@param ev event type
 *@param arg a cookie
 * */
void TcpFlow::data(int fd, short ev, void *arg){
    if(ev != EV_READ){
        printf("TcpFlow::data: Wrong event %d is received\n", ev);
        return;
    }

    //printf("received read event for FD %d in worker thread %lu\n",fd,  pthread_self());

    TcpFlow *flow = reinterpret_cast<TcpFlow *>(arg);
    flow->m_clientFd = fd;

    for(auto i = 0; i < NUM_READ_TRY; i++){
        Byte buffer[BUFLEN] = {'\0'};
        int len = -1;
        len = read(fd, buffer, BUFLEN);
        if(len == 0){
            //printf("Client disconnected with fd %d\n", fd);
            try{
                flow->m_deleteCbk(flow->getAddr());
            } catch(const std::bad_function_call& e){
                printf("Cbk is not able to invoked with error %s\n", e.what());
            }

            return;
        } else if(len < 0){
            if(errno != EAGAIN){
                printf("TcpFlow::data: Socket failure, disconnecting client: %s\n",
                        strerror(errno));
                flow->m_pollAgain = false;
                try{
                    flow->m_deleteCbk(flow->getAddr());
                } catch(const std::bad_function_call& e){
                    printf("Cbk is not able to invoked with error %s\n", e.what());
                }
                return;
            }
            else if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR){
                break;
            }else if(errno == EAGAIN){
                break;
            }
        } else{
            flow->m_lenTillnow += len;
            if(flow->m_data){
                memcpy(flow->m_data + flow->m_currlen,  buffer, len);
                flow->m_currlen += len;
            }else{
                if(flow->m_lenTillnow >= 4){
                    uint32_t *p_len = (uint32_t *)buffer;
                    flow->m_payloadLen = *p_len;
                    flow->m_data = (Byte *)calloc(1, flow->m_payloadLen);
                    if(flow->m_lenTillnow > 4){
                        memcpy(flow->m_data,  buffer + 4, (len-4));
                        flow->m_currlen = len - 4;
                    }
                } else{
                    printf("Unhandled case\n");
                //Not handling this case, assuming we will read atleast 4 bytes if available            
                }
            }
        }
    }

    if(flow->m_lenTillnow >= flow->m_payloadLen){
        flow->parse();
    }

    return;
}

/*
 *@brief deserialize the buffer to a protobuf and performs the required function
 * */
void TcpFlow::parse(){
    m_payload.ParseFromArray(m_data, m_payloadLen);
    if(m_payload.has_requesttype()){
        Payload_RequestType type = m_payload.requesttype();
        switch(type){
            case Payload_RequestType_GET:
                handleGetRequest();
                break;
            case Payload_RequestType_SET:
                handleSetRequest();
                break;
            case Payload_RequestType_STAT:
                handleStatRequest();
                break;
            default:
                printf("Wrong message type\n");
                this->m_deleteCbk(m_selfAddr);
        }
        this->sendResponse();
    } else{
        printf("Request type not provided in the request\n");
        this->m_deleteCbk(m_selfAddr);
    }
}

/*
 *@brief handles the GET request
 * */
void TcpFlow::handleGetRequest(){
    Dictionary *dict = Dictionary::getInstance();
    Stats *stats = Stats::getInstance();
    std::string value;

    stats->incrGetReq();
    if(dict->get(m_payload.key(), value)){
        m_payload.set_value(value);
        stats->incrGetReqSucc();
    } else{
        stats->incrGetReqFail();
        m_payload.set_reason(value); //reason is filled if GET operations failed
    }
}

/*
 *@brief handles the SET request
 * */
void TcpFlow::handleSetRequest(){
    Dictionary *dict = Dictionary::getInstance();
    if(!dict->set(m_payload.key(), m_payload.value())){
        m_payload.set_reason("failed");
    }
}

/*
 *@brief handles the STAT request
 * */
void TcpFlow::handleStatRequest(){
    Stats *stat = Stats::getInstance();
    Payload::Stat *stat_val = m_payload.mutable_stats(); 
    stat_val->set_numgetreq(stat->getNumGetReq());
    stat_val->set_numsuccgetreq(stat->getNumGetReqSucc());
    stat_val->set_numfailgetreq(stat->getNumGetReqFail());
}

/*
 *@brief sending response to client
 * */
void TcpFlow::sendResponse(){
    uint32_t size = m_payload.ByteSizeLong() + 1;
    Byte *buffer = (Byte* )calloc(1, size);
    m_payload.SerializeToArray(buffer, size);
    this->socketWriteData(buffer, size);
}


/*
 *@brief writing data to client socket FD
 *@param buf response buffer
 *@param len response buffer len
 * */
int TcpFlow::socketWriteData(Byte *buf, size_t buf_len)
{
    int bytes =0;
    while( buf_len > 0 ){
        int n = write(m_clientFd, buf, buf_len);
        if( n < 0 ){
            if( errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR ){
                return -1;
            }
            continue;
        }
        buf_len -= n;
        buf += n;
        bytes += n;
    }
    return bytes;
}
