#include "TestClient.h"

TestClient::TestClient(std::string input_file, int num_thread, std::string host, int port)
    : m_inputFile(input_file), m_numThread(num_thread), m_host(host), m_port(port){

    }

TestClient::~TestClient(){

}

/*
 *@brief start the TCP clients in different threads concurrently
 * */
void TestClient::start(){
    std::vector<MessageT> input_data;
    readAndParseInputFile(input_data); 

    ThreadPool threads;
    std::size_t const chunk_size = input_data.size() / m_numThread;
    std::size_t start_size = 0;
    for(auto i=1; i<= m_numThread; i++){
        std::shared_ptr<Client> client(new (std::nothrow) Client(m_host, m_port));
        if(i == m_numThread){
            std::vector<MessageT> chunk(input_data.begin() + start_size, input_data.end());
            client->setInputData(chunk);
        } else{
            std::vector<MessageT> chunk(input_data.begin() + start_size,
                    input_data.begin() + start_size + chunk_size);
            client->setInputData(chunk);
        }

        start_size += chunk_size;

        std::shared_ptr<std::thread> th_obj(new (std::nothrow) std::thread(&Client::run, client));
        threads.push_back(std::make_pair(th_obj, client));
    }

    for(auto i=0; i< m_numThread; i++){
        (threads[i].first)->join();
    }

    this->saveResults(threads);
}

/*
 *@brief case insensitive comparison of two string
 *@param string1
 *@param string2
 *@return true for equal, false for not equal
 * */
bool TestClient::caseInSensStringCompare(std::string str1, std::string str2){
    return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), [](char & c1, char & c2){
                            return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
                                }));
}

/*
 *@brief reading the input data from a file and parse it make the request format
 *@param a out parameeter that contains the dictionary request info parsed from the given input file
 * */
void TestClient::readAndParseInputFile(std::vector<MessageT>& input_data){
    std::ifstream infile(m_inputFile.c_str());
    std::string line;
    while(!infile.eof()){
        std::getline(infile, line);
        char *saveptr = NULL;
        char *str = const_cast<char *>(line.c_str());
        char *token = strtok_r(str, ",", &saveptr);
        std::vector<std::string> line_field;
        while(token != NULL){
            std::string field = Helper::trim(token);
            line_field.push_back(field);
            token = strtok_r(NULL, ",", &saveptr);
        }

        MessageT msg;
        if(line_field.size()){
            if(caseInSensStringCompare(line_field[0], "GET") && line_field.size() >= 2){
                msg.type = MessageType::GET;
                msg.key = line_field[1];
            } else if(caseInSensStringCompare(line_field[0], "SET") && line_field.size() >= 3){
                msg.type = MessageType::SET;
                msg.key = line_field[1];
                msg.value = line_field[2];
            } else if(caseInSensStringCompare(line_field[0], "STAT")){
                msg.type = MessageType::STAT;
            }
        }

        input_data.push_back(msg);
    }
}

/*
 *@brief returning different request type in string format
 *@param request type in enum
 *@return request type in string
 * */
std::string TestClient::getReqType(Payload_RequestType type){
    std::string ops="";
    switch(type){
        case Payload_RequestType_GET:
            ops = "GET";
            break;
        case Payload_RequestType_SET:
            ops = "SET";
            break;
        case Payload_RequestType_STAT:
            ops = "STAT";
            break;
    }

    return ops;
}

bool TestClient::compareByTimestamp(const ResultData &a, const ResultData &b)
{
    return a.timestamp < b.timestamp;
}

/*
 *brief save the results in a file
 *@param pool of threads
 * */
void TestClient::saveResults(const ThreadPool pool){
    std::vector<ResultData> results;
    uint32_t avgLatency = 0;
    for(auto i = 0; i < pool.size(); i++){
        auto result = (pool[i].second)->getResults();
        results.insert(results.end(), result.begin(), result.end());

        if(avgLatency == 0){
            avgLatency = (pool[i].second)->getAverageLatency();
        } else{
            avgLatency = (avgLatency + (pool[i].second)->getAverageLatency())/2;
        }
    }

    if(!results.size()){
        printf("No result to print\n");
        return;
    }
    time_t seconds = time(NULL);
    auto file_name = "./results/results_" + std::to_string(seconds) + ".csv";
    std::ofstream outfile;
    outfile.open(file_name.c_str(), std::ios_base::app);
    std::string data="";
    uint64_t count = 0;

    std::sort(results.begin(), results.end(), compareByTimestamp);
    for(auto i = 0; i < results.size(); i++){
        std::string line("");
        Payload payload = results[i].payload;
        line = line + std::to_string(results[i].timestamp);
        if(payload.has_requesttype()){
            line = line + "," + getReqType(payload.requesttype());
        } else{
            continue;
        }

        if(payload.has_key()){
            line = line + "," + payload.key();
        }

        if(payload.has_value()){
            line = line + "," + payload.value();
        } else if(payload.requesttype() == Payload_RequestType_GET){
            if(payload.has_reason()){
                line = line + "," + payload.reason();
            }
        }

        if(payload.has_stats()){
            auto stat_val = payload.stats();
            if(stat_val.has_numgetreq()){
                line = line + "," + std::to_string(stat_val.numgetreq());
            }

            if(stat_val.has_numsuccgetreq()){
                line = line + "," + std::to_string(stat_val.numsuccgetreq());
            }

            if(stat_val.has_numfailgetreq()){
                line = line + "," + std::to_string(stat_val.numfailgetreq());
            }
        }

        line = line + "\n";
        data = data + line;
        count++;

        if(count == MAX_LINE_WRITE){
            outfile << data;
            count = 0;
            data = "";
        }
    }

    if(!data.empty())
        outfile << data;

    std::string avg_latency = "\n\n    Avergage Latency: " + std::to_string(avgLatency) + " microseconds";
    outfile << avg_latency;

    outfile.close();
}

/*
 *@brief prints the help message
 * */
void printUsage(){
    printf("Usage: ./msgclient -t <NUM_WORKER> -i <server IP address> -p <server port> -f <input_file>\n");
    printf("Example: ./msgclient -t 4 -i 127.0.0.1 -p 6000 -f ./data/input_data.csv\n");
    printf( "       -t   --job          Number of worker threads\n"
            "       -i   --dest_ip      server IP address\n"
            "       -p   --port         server port\n"
            "       -f   --file         input file for different operations\n");
}

/*
 *@brief the main function
 * */
int main(int argc, char** argv){
    std::string host(""), input_file("");
    int port = 0;
    int num_thread = 0;
    int next_option;
    const char* const short_options = "ht:i:p:f:";
    const struct option long_options[] = {
        { "help",       0, NULL, 'h' },
        { "thread",     1, NULL, 't' },
        { "dest_ip",    1, NULL, 'i' },
        { "port",       1, NULL, 'p' },
        { "file",       1, NULL, 'f' },
        { NULL,         0, NULL,  0  }
    };

    while((next_option = getopt_long (argc, argv, short_options, long_options, NULL)) != -1){
        switch (next_option){
            case 'h':
                printUsage();
                break;
            case 't':
                num_thread = atoi(optarg);
                break;
            case 'i':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);;
                break;
            case 'f':
                input_file = optarg;
                break;
            default:
                printf("Unexpected error while parsing options!\n");
                return 1;
        }
    }while (next_option >= 0);


    if(argc < 9){
        printUsage();
        return 1;
    }

    TestClient test_client(input_file, num_thread, host, port);
    test_client.start();
    return 0;
}
