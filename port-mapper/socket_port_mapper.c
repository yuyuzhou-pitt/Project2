/* Multi-thread version socket.
 *  - assign port automatically
 *  - timeout after given time
 *  - accept multi-client connection
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include "socket_port_mapper.h"
#include "../packet/linkload.h"
#include "../packet/register_ack.h"
#include "../lib/libsocket.h"
#include "../lib/getaddrinfo.h"
#include "../lib/liblog.h"
#include "../lib/libfile.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define MAX_QUE_CONN_NM 1024
#define TIMER 3600  //Timeout value in seconds
#define CONTINUE 1 
#define NTHREADS 5

pthread_t threadid[NTHREADS]; // Thread pool
int counter = 0;

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)
int port; // local socket port

pthread_mutex_t mutex;

/*thread for new client connection, arg is the client_fd*/
void *port_mapper_thread(void *arg){
    int sockfd = (int *)arg;

    //LoadLink *link;
    //link = threadArgs->loadLink;

    char logmsg[128]; 

    /* There are 5 types of Packets to be exchanged via port-mapper:
     * 1) Register service (from server to port mapper)         (000)
     * 2) Register acknowledge (from port mapper to server)     (001)
     * 3) Hello Packets (from server to port mapper)            (111)
     * 4) Request server location (from client to portmapper)   (010)
     * 5) Response server location (from portmapper to client)  (011)
     * */

    struct timeval *tmpcost, cost, timer; // use high quality timer to calculate the ping cost
    struct timezone tzp;

    /* keep receiving Data from server or client */
    //while(1){

    gettimeofday(&timer, &tzp);

    /*lock the send back in case it interrupt by other threads*/
    pthread_mutex_lock(&mutex);

    Packet *packet_recv; // MUST use pointer to fit different Packet
    packet_recv = (Packet *)calloc(1, sizeof(Packet));

    //printf("==port-mapper 1==");
    Recv(sockfd, packet_recv, sizeof(Packet), MSG_NOSIGNAL);

    //printf("==port-mapper 2==");
    /* Register service */
    if(strcmp(packet_recv->packet_type, "000") == 0){
        int dup_register = 0;
        //snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): packet_recv type: %s\n", pthread_self(), packet_recv->packet_type);
        //logging(LOGFILE, logmsg);
        dup_register = writePortMapperTable(packet_recv, PORT_MAPPER_TABLE_FILE);
        sendRegisterReply(sockfd, packet_recv, dup_register);
    }
    /* Hello packet */
    else if(strcmp(packet_recv->packet_type, "111") == 0){
        snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): packet_recv type: %s\n", pthread_self(), packet_recv->packet_type);
        logging(LOGFILE, logmsg);
        //sendHello(sockfd, router, threadParam->port);
    }
    /* Request server location */
    else if(strcmp(packet_recv->packet_type, "010") == 0 ){
        snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): packet_recv type: %s\n", pthread_self(), packet_recv->packet_type);
        logging(LOGFILE, logmsg);
        /*record the chosen response into loadlink*/
        sendRequestReply(sockfd, packet_recv);
    }
    //printf("==port-mapper 3==");

    pthread_mutex_unlock(&mutex);

    shutdown(sockfd, SHUT_RDWR);
    //snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): served request, exiting thread\n", pthread_self());
    //logging(LOGFILE, logmsg);

    //pthread_exit(0);
    free(packet_recv);
    pthread_exit((void *)link);
    //return (void *) threadArgs->loadLink;
}

void *sock_port_mapper(void *arg){

    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    int sockfd, client_fd, desc_ready;
    char logmsg[128];

    sin_size=sizeof(client_sockaddr);

    /* Data structure to handle timeout */
    struct timeval before, timer, *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set, test_set;
    int maxfd, nready, client[FD_SETSIZE];

    /* create socket */
    sockfd = Socket(AF_INET,SOCK_STREAM,0);

    /* set parameters for sockaddr_in */
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    Bind(sockfd, server_sockaddr);

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    writePortMapper(port, addrstr, PORT_MAPPER_FILE);
    
    snprintf(logmsg, sizeof(logmsg), "sockserver: Server %s (%s) is setup on port: %d\n", addrstr, hostname, port);
    logging(LOGFILE, logmsg);
    Listen(sockfd, MAX_QUE_CONN_NM);
   
    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)
    int iThread = 0; // Thread iterator

    /* Set up the I/O for the socket, nonblocking */
    maxfd = sockfd;
    int k;
    for(k=0;k<FD_SETSIZE;k++){
        client[k] = -1;
    }
    FD_ZERO(&ready_set);
    FD_ZERO(&test_set);
    FD_SET(sockfd, &test_set);
  
    /* Initialize the timeval struct to TIMER seconds */
    timer.tv_sec = TIMER;
    timer.tv_usec = 0;
    tvptr = &timer;

    /* Set up the time out by getting the time of the day from the system */
    gettimeofday(&before, &tzp);

    int status;
    status=CONTINUE;
    while (status==CONTINUE){
        if (iThread == NTHREADS){
            iThread = 0;
        }

        memcpy(&ready_set, &test_set, sizeof(test_set));
        nready = select(maxfd+1, &ready_set, NULL, NULL, tvptr);

        switch(nready){
            case -1:
                printf("sockserver: errno: %d.\n", errno);
                perror("\nSELECT: unexpected error occured.\n");
                logging(LOGFILE, "\nSELECT: unexpected error occured.\n");

                /* remove bad fd */
                for(k=0;k<FD_SETSIZE;k++){
                    if(client[k] > 0){
                        struct stat tStat;
                        if (-1 == fstat(client[k], &tStat)){
                            printf("fstat %d error:%s", sockfd, strerror(errno));
                            FD_CLR(client[k], &ready_set);
                        }
                    }
                }
                status=-1;
                break;
            case 0:
                /* timeout occuired */
                printf("sockserver: TIMEOUT... %d.\n", errno);
                status=-1;
                break;
            default:
                if (FD_ISSET(sockfd, &ready_set)){
                    //snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Listening socket is readable\n", pthread_self());
                    //logging(LOGFILE, logmsg);
                    /* wait for connection */
                    client_fd = Accept(sockfd, client_sockaddr, sin_size);
                    for(k=0;k<FD_SETSIZE;k++){
                        if(client[k] < 0){
                            client[k] = client_fd;
                        }
                    }

                    FD_SET(client_fd, &test_set);
                    if (client_fd > maxfd) maxfd = client_fd;
                    snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Descriptor %d is readable\n",  pthread_self(), client_fd);
                    logging(LOGFILE, logmsg);
                    pthread_create(&threadid[iThread], &attr, &port_mapper_thread, (void *)client_fd);
                    pthread_join(threadid[iThread], NULL);
                    iThread++;
                }// end if (FD_ISSET(i, &ready_set))
        }// end switch
        usleep(100);
    } // end while (status==CONTINUE)
    close(sockfd);
    unlinkPortFile(addrstr);

    return 0;
}
