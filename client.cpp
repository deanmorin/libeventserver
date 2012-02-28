#include <arpa/inet.h>
#include <errno.h>
#include <program_options.hpp>
#include <pthread.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "network.hpp"

namespace po = boost::program_options;
using namespace dm;

struct clientArgs {
    std::string host;
    int port;
    uint32_t size;
    int count;
};

void* requestData(void* args)
{
    struct clientArgs* ca = (clientArgs*) args;
    int i = 0;
    char requestMsg[REQUEST_SIZE];
    requestMsg[3] = (uint32_t) ((ca->size >> 24) & 0xFF);
    requestMsg[2] = (uint32_t) ((ca->size >> 16) & 0xFF);
    requestMsg[1] = (uint32_t) ((ca->size >>  8) & 0xFF);
    requestMsg[0] = (uint32_t) ((ca->size & 0xFF));
    char* responseMsg = new char[ca->size];
    int bytesToRead = ca->size;
    int flag = 0;

    int sock;
    struct sockaddr_in server;
    struct hostent* hp;
    sleep(1);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket() could not create socket");
        exit(1);
    }
    int arg = 1;
    // set so port can be resused imemediately after ctrl-c
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1) 
    {
        perror("setsockopt()");
        exit(1);
    }

    // set up address structure
    bzero((char*) &server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(ca->port);
    if (!(hp = gethostbyname(ca->host.c_str())))
    {
        std::cerr << "Error: unknown server address\n";
        exit(1);
        std::cerr << i << "\n";
    }
    memcpy(hp->h_addr, (char*) &server.sin_addr, hp->h_length);
    
    // connect
    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) == -1)
    {
        perror("connect() could not connect to server");
        exit(1);
    }

#ifdef __APPLE__
    int set = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void*) &set, sizeof(int)))
    {
        perror("setsockopt() failed");
        std::cerr << set << "\n";
    }
#else
    flag = MSG_NOSIGNAL;
#endif

    // transmit request and receive packets
    for (i = 0; i < ca->count; i++)
    {
        if (send(sock, requestMsg, REQUEST_SIZE, flag) < 0)
        {
            perror("send() failed");
            exit(1);
        }
        clearSocket(sock, responseMsg, bytesToRead);
    }
    close(sock);

    delete responseMsg;
    return NULL;
}

void runClients(struct clientArgs* args, int clients) 
{
    std::vector<pthread_t> threads;
    threads.resize(clients);
    int rtn = 0;
    int i = 0;
    
    for (i = 0; i < clients; i++)
    {
        if ((rtn = pthread_create(&threads[i], NULL, &requestData, 
                        (void*) args)))
        {
            if (rtn == EAGAIN)
            {
                std::cerr << "Could not create a new thread.\n";
            }
            perror("pthread_create()");
            exit(1);
        }
    }
    for (i = 0; i < clients; i++)
    {
        pthread_join(threads[i], NULL);
    }
#ifdef DEBUG
    std::cerr << "Finished\n";
#endif
}

int main(int argc, char** argv)
{
    int opt = 0;
    std::string option = "";
    int clients = 0;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("host,h", po::value<std::string>(&option)->default_value("localhost"),
         "host to connect to")
        ("port,p", po::value<int>(&opt)->default_value(32000),
         "port to use")
        ("message-size,s", po::value<int>(&opt)->default_value(1024), 
         "length of packets to request")
        ("message-count,c", po::value<int>(&opt)->default_value(25), 
         "number of packets to request")
        ("clients,x", po::value<int>(&opt)->default_value(512), 
         "number of clients to create")
        ("help", "show this message")
    ;

    po::variables_map vm;
    try 
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } 
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "\tuse --help to see program options\n";
        return 1;
    }
    
    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 0;
    }

    struct clientArgs* args = new clientArgs;

    args->host = vm["host"].as<std::string>();
    args->port = vm["port"].as<int>();
    args->size = vm["message-size"].as<int>();
    args->count = vm["message-count"].as<int>();
    clients = vm["clients"].as<int>();

    std::cout << "Host:\t\t\t" << args->host << "\n";
    std::cout << "Port:\t\t\t" << args->port << "\n";
    std::cout << "Message size:\t\t" << args->size << "\n";
    std::cout << "Message count:\t\t" << args->count << "\n";
    std::cout << "Number of clients:\t" << clients << "\n";

    runClients(args, clients);
    delete args;

    return 0;
}
