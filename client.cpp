#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include "network.hpp"
namespace po = boost::program_options;
using namespace dm;

#define OUT_FILE        "response_times.csv"
#define FILE_BUFSIZE    10
#define MSG_PER_RECORD  1

double secondsDiff(const timeval& val1, const timeval& val2);

struct clientArgs {
    std::string host;
    int port;
    uint32_t size;
    int count;
    double timeout;
    std::ofstream out;
    pthread_mutex_t fileMutex;
};

void* requestData(void* args)
{
    std::pair<struct clientArgs*, int>* threadArgs 
            = (std::pair<struct clientArgs*, int>*) args;
    struct clientArgs* ca = threadArgs->first;
    int threadID = threadArgs->second;
    threadID += 1;
    delete threadArgs;
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

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        exit(sockError("socket()", 0));
    }
    int arg = 1;
    // set so port can be resused imemediately after ctrl-c
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg)) == -1) 
    {
        exit(sockError("setsockopt()", 0));
    }

    // set up address structure
    memset((char*) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(ca->port);
    if (!(hp = gethostbyname(ca->host.c_str())))
    {
        std::cerr << "Error: unknown server address\n";
        exit(1);
    }
    memcpy((char*) &server.sin_addr, hp->h_addr, hp->h_length);

    // connect
    if (connect(sock, (struct sockaddr*) &server, sizeof(server)))
    {
        exit(sockError("connect()", 0));
    }

#ifdef __APPLE__
    int set = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void*) &set, sizeof(int)))
    {
        exit(sockError("setsockopt()", 0));
    }
#else
    flag = MSG_NOSIGNAL;
#endif

    struct timeval startTime;
    if (gettimeofday(&startTime, NULL) == -1)
    {
        exit(sockError("gettimeofday()", 0));
    }

    struct timeval sendTime;
    struct timeval recvTime;
    double roundTrips[FILE_BUFSIZE];
    size_t firstMsgNo = 1;
	size_t lastMsgNo[FILE_BUFSIZE];
    size_t msgInBuf = 0;
    double requestTime = 0;

    // transmit request and receive packets
    for (i = 0; i < ca->count; i++)
    {
        if (i % MSG_PER_RECORD == 0) 
        {
            if (gettimeofday(&sendTime, NULL) == -1)
            {
                exit(sockError("gettimeofday()", 0));
            }
        }
        if (send(sock, requestMsg, REQUEST_SIZE, flag) < 0)
        {
            perror("send() failed");
            exit(1);
        }
        clearSocket(sock, responseMsg, bytesToRead);

        if (i % MSG_PER_RECORD == MSG_PER_RECORD - 1 || i == ca->count - 1)
        {
            if (gettimeofday(&recvTime, NULL) == -1)
            {
                exit(sockError("gettimeofday()", 0));
            }
            requestTime = secondsDiff(sendTime, recvTime);
            roundTrips[msgInBuf % FILE_BUFSIZE] = requestTime;
			lastMsgNo[msgInBuf % FILE_BUFSIZE] = i + 1;
            msgInBuf++;

            if (requestTime >= ca->timeout)
            {
                // quit since the last request took too long
                i = ca->count -1;
            }

            if (msgInBuf % FILE_BUFSIZE == 0
                || i == ca->count - 1)
            {
                pthread_mutex_lock(&ca->fileMutex);
                for (size_t j = 0; j < msgInBuf; j++)
                {
                    ca->out << threadID << "," << firstMsgNo << " to "
							<< lastMsgNo[j] << "," << ca->size << ","
							<< roundTrips[j] << "\n";
					firstMsgNo = lastMsgNo[j] + 1;
                }
                pthread_mutex_unlock(&ca->fileMutex);
                msgInBuf = 0;
            }
        }

    }
    close(sock);

    struct timeval endTime;
    if (gettimeofday(&endTime, NULL) == -1)
    {
        exit(sockError("gettimeofday()", 0));
    }

    double* timeToComplete = new double();
    *timeToComplete = secondsDiff(startTime, endTime);
#ifdef DEBUG
    std::cout << *timeToComplete << "\n";
#endif

    delete responseMsg;
    return timeToComplete;
}

void runClients(struct clientArgs* ca, int clients) 
{
    std::vector<pthread_t> threads;
    threads.resize(clients);
    struct rlimit rlim;

    // increase allowed threads per process
    getrlimit(RLIMIT_NPROC, &rlim);
    rlim.rlim_cur = rlim.rlim_max;
    setrlimit(RLIMIT_NPROC, &rlim);

    int rtn = 0;
    int i = 0;
    double* timeToComplete = 0;
    double totalTime = 0;
    double averageTime = 0;
    
    for (i = 0; i < clients; i++)
    {
        std::pair<struct clientArgs*, int>* args 
                = new std::pair<struct clientArgs*, int>();
        args->first = ca;
        args->second = i;

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
        usleep(1000);
    }
    for (i = 0; i < clients; i++)
    {
        pthread_join(threads[i], (void**) &timeToComplete);
        totalTime += *timeToComplete;
        delete timeToComplete;
    }
    averageTime = totalTime / i;
    std::cout << "Average connection time: " << averageTime << " seconds\n\n";
}

int main(int argc, char** argv)
{
    int opt = 0;
    double dopt = 0;
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
        ("message-count,c", po::value<int>(&opt)->default_value(250),
         "number of packets to request")
        ("clients,x", po::value<int>(&opt)->default_value(250),
         "number of clients to create")
        ("timeout,t", po::value<double>(&dopt)->default_value(3),
         "seconds in timeout")
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

    struct clientArgs args;

    args.host = vm["host"].as<std::string>();
    args.port = vm["port"].as<int>();
    args.size = vm["message-size"].as<int>();
    args.count = vm["message-count"].as<int>();
    args.timeout = vm["timeout"].as<double>();
    clients = vm["clients"].as<int>();

    std::cout << "Host:\t\t\t" << args.host << "\n";
    std::cout << "Port:\t\t\t" << args.port << "\n";
    std::cout << "Message size:\t\t" << args.size << "\n";
    std::cout << "Message count:\t\t" << args.count << "\n";
    std::cout << "Number of clients:\t" << clients << "\n";
    std::cout << "Seconds to wait:\t" << args.timeout << "\n";

    args.out.open(OUT_FILE);
    if (!args.out)
    {
        std::cerr << "unable to open \"" << OUT_FILE << "\"\n";
        exit(1);
    }
    args.out << "Thread ID,Message Count,Message Size,Seconds\n";

    if (pthread_mutex_init(&args.fileMutex, NULL))
    {
        std::cerr << "Error creating mutex\n";
        exit(1);
    }

    runClients(&args, clients);

    args.out.close();
    return 0;
}

/**
 * Finds the difference between val1 and val2 in seconds. Accurate to 
 * microseconds.
 * @return The difference in seconds.
 * @author Dean Morin
 */
double secondsDiff(const timeval& val1, const timeval& val2)
{
    long sec1 = val1.tv_sec;
    long sec2 = val2.tv_sec;
    long usec1 = val1.tv_usec;
    long usec2 = val2.tv_usec;

    return abs(sec2 - sec1) + abs(usec2 - usec1) * pow(10, -6);
}
