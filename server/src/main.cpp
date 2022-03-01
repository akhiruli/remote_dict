#include <memory>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>

#include "TcpServer.h"

int done = 0;

void sighandler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT){
        printf("Received signal %d \n", signo);
        done = 1;
    }
}

void printUsage(){
    printf("Usage: ./msgclient -t <NUM_WORKER> -i <server IP address> -p <server port> -b <bloom filter flag (0 for disable and > 0 for enable)>\n");
    printf("Example: ./msgclient -t 4 -i 127.0.0.1 -p 6000\n");
    printf( "       -t   --job          Number of worker threads\n"
            "       -i   --dest_ip      server IP address\n"
            "       -p   --port         server port\n"
            "       -b   --bfilter      bloom filter flag (0 for disable and > 0 for enable)\n");
}


int main(int argc, char** argv){
    std::string host("");
    int port = 0;
    int num_thread = 0;
    int bloom_filter = 0;
    int next_option;
    const char* const short_options = "ht:i:p:b:";
    const struct option long_options[] = {
        { "help",       0, NULL, 'h' },
        { "thread",     1, NULL, 't' },
        { "dest_ip",    1, NULL, 'i' },
        { "port",       1, NULL, 'p' },
        { "bfilter",    1, NULL, 'b' },
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
                port = atoi(optarg);
                break;
            case 'b':
                bloom_filter = atoi(optarg);
                break;
            default:
                printf("Unexpected error while parsing options!\n");
                return 1;
        }
    }while (next_option >= 0);


    if(argc < 7){
        printUsage();
        return 1;
    }

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sighandler;

    sigaction(SIGTERM, &act, 0);
    sigaction(SIGINT,  &act, 0);

    TcpServer *tcpserver = new TcpServer(num_thread, host, port, (bloom_filter>0)?true:false);
    tcpserver->start();

    sleep(2);
    printf("Press ctrl-c, or send SIGTERM to process ID %d, to gracefully exit program.\n", getpid());
    printf("Started a TCP server with host %s, port %d, 1 listener and %d worker threads, bloom fliter enable %d\n", host.c_str(), port,num_thread, (bloom_filter>0)?true:false);

    while (!done)
    {
        sleep(1);
        //printf("I'm still waiting...\n");
    }

    tcpserver->shutdown();
    delete tcpserver;
    printf("All done!\n");

    return 0;
}
