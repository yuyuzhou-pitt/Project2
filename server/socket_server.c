/* Single-thread version datagram socket.
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

#include "../lib/unprtt.h"

#define EXECUTE_SERV 1
#include "../packet/packet.h"
#include "../packet/linkseq.h"

#include "socket_server.h"
#include "server_stub_receive.h"
#include "../packet/register.h"
#include "../packet/execute.h"
#include "../packet/execute_ack.h"
#include "../packet/execute_reply.h"
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
#define WINDOW_SIZE 4

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)
int port; // local socket port

pthread_t threadid[NTHREADS]; // Thread pool

struct thread_info{
    pthread_t thread_id;  /* ID returned by pthread_create()*/
    int thread_num;       /* Application-defined thread #*/
    char *argv_string;    /* From command-line argument */
};

pthread_mutex_t mutex;

static struct rtt_info   rttinfo;
static int rttinit = 0;

//Packet_Seq *executePacketSeq; //the execute send sequence
//Packet_Seq *executeResultSeq; //the execute reply sequence

/*thread for packet handle, ack scheme*/
void *server_thread(int sockfd, struct sockaddr_in server_sockaddr){
//void *server_thread(void *arg){

    /*
    int sockfd;
    struct sockaddr_in server_sockaddr;

    ThreadSocket *threadSocket;
    threadSocket = (ThreadSocket *)malloc(sizeof(ThreadSocket));
    threadSocket = (ThreadSocket *)arg;

    sockfd = threadSocket->sockfd;
    server_sockaddr = threadSocket->sockaddr;
    */

    struct sockaddr_in client_sockaddr;

    OptionsStruct *result_options;
    result_options = (OptionsStruct *)malloc(sizeof(OptionsStruct));
    char client_ip[100];
    char server_ip[100];

    char logmsg[128];
    int receiveFlag = 1;
    int sendFlag = 1; 
    int counter = 0;

    //Packet_Seq *incoming_head, *outgoing_head, *trimed_head;
    //incoming_head = initListSeq();
    //Packet_Seq *outgoing_head;
    //outgoing_head = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //outgoing_head = initListSeq();
    //trimed_head = incoming_head; //trimed header is used to avoid unessary link list scan. 

    if (rttinit == 0) {
        rtt_init(&rttinfo);             /* first time we're called */
        rttinit = 1;
        rtt_d_flag = 1;
    }

    /*lock the send back in case it interrupt by other threads*/
    pthread_mutex_lock(&mutex);

    Packet_Seq *result_seq, *send_seq;
    Packet packet_recv, *packet_reply, *packet_ack; // MUST use pointer to fit different Packet
    packet_ack = (Packet *)calloc(1, sizeof(Packet));
    //packet_recv = (Packet *)calloc(1, sizeof(Packet));

    //printf("==server_thread 1==\n");
    //Recvfrom(sockfd, &packet_recv, sizeof(Packet), MSG_NOSIGNAL, (struct sockaddr *)&client_sockaddr, sizeof(client_sockaddr));
    recvExecuteServ(sockfd, (struct sockaddr *)&client_sockaddr, &packet_recv);
    //printf("==server_thread 2==\n");

    //printf("Start communicating with Client %s.\n\n", packet_recv.sender_id);

    char trans_id[32]; //record the transaction id, in case multiple sequences
    Packet *execute_ack;// = genExecuteAck(trimed_head);
    int endTransaction = 0;
    if(strcmp(packet_recv.packet_type, "100") == 0){ //execute packet
        //printf("==server_thread 3==\n");
        //printf("Got execute packet with seq %d from Client %s.\n", packet_recv.Data.seq, packet_recv.sender_id);
        insertListSeq(executePacketSeq, packet_recv);
        //appendListSeq(executePacketSeq, packet_recv);
        //incoming_head = appendListSeq(executePacketSeq, packet_recv);
        if (isEndTransaction(executePacketSeq, packet_recv.Data.transaction_id) == 1){
            //printf("==server_thread 1==\n");
            endTransaction = 1;
        }
    }

    if(endTransaction == 1){
        //fprintf(stderr, "\nGot %d execute packet(s) from Client %s.\n", packet_recv.Data.seq + 1, packet_recv.sender_id);
        //printf("==server_thread 4==\n");
        //fprintf(stderr, "Sending bulk ack to Client %s: %d packet(s) received.\n", packet_recv.sender_id, packet_recv.Data.seq + 1);
        packet_recv.Data.ts = rtt_ts(&rttinfo);
        sendExecuteAck(sockfd, client_sockaddr, &packet_recv);

        /*calculate the result by demashing the received sequence and store the result into a file*/
        //fprintf(stderr, "\nStart de-marshaling the Data from the received packet(s).\n");
        int result_type;
        result_type = executeResult(result_options, client_ip, server_ip, executePacketSeq);
        //fprintf(stderr, "Calculating result ...\n");

        /*generate the result seq*/
        //fprintf(stderr, "Generating the result link...\n\n");
        if(result_type == 0){ // for client
            genExecuteReply(result_options, client_ip, server_ip, trans_id);
        }
        else if(result_type == 1){ // for minigoogle
            genMapReduceReply(result_options, client_ip, server_ip, trans_id);
        }

        /*keep sending until got ack from client*/
        int result_seq = 0;
        do{
            send_seq = executeResultSeq->next;
            while(send_seq->next != send_seq){
                if(send_seq->cur.Data.transaction_id == atoi(trans_id)){
                    //printf("Sending result packet with seq %d to Client %s.\n", send_seq->cur.Data.seq, send_seq->cur.sender_id);
                    send_seq->cur.Data.ts = rtt_ts(&rttinfo);
                    sendExecuteReply(sockfd, client_sockaddr, &(send_seq->cur));
                    result_seq = send_seq->cur.Data.seq; 
                }
                send_seq = send_seq->next;
            }

            //fprintf(stderr, "Sent %d result packet(s) to Client.\n", result_seq + 1);
            recvResultAck(sockfd, (struct sockaddr *)&client_sockaddr, packet_ack);

            //printf("==server_thread 6==\n");
        }while(strcmp(packet_ack->packet_type, "110") != 0);

        //fprintf(stderr, "\nGot result bulk ack from Client %s: acknowledged %d result packet(s).\n", 
        //        packet_ack->sender_id, packet_ack->Data.seq);
        //fprintf(stderr, "Congratulations! RPC %s completed successfully.\n", packet_ack->Data.procedure_name);
        //fprintf(stderr, "---------------\n");
        /*remove packet_reply transaction from executeResultSeq*/
        clearTransaction(executeResultSeq, atoi(trans_id));
        /*remove packet_execute (packet_ack) transaction from executePacketSeq*/
        clearTransaction(executePacketSeq, packet_recv.Data.transaction_id);
    }
    pthread_mutex_unlock(&mutex);

    free(result_options);
    //shutdown(sockfd, SHUT_RDWR);
    //snprintf(logmsg, sizeof(logmsg), "serverthread(0x%x): served request, exiting thread\n", pthread_self());
    //logging(LOGFILE, logmsg);
    //pthread_exit(0);
}

void *sockserver(void *arg){
    /*initial the global seq to store execute packet from client*/

    struct sockaddr_in server_sockaddr, client_sockaddr;
    int sin_size, recvbytes, sendbytes;
    int sockfd, client_fd, desc_ready;
    char logmsg[128];

    sin_size=sizeof(server_sockaddr);

    /* Data structure to handle timeout */
    struct timeval before, timer, *tvptr;
    struct timezone tzp;
    
    /* Data structure for the select I/O */
    fd_set ready_set, test_set;
    int maxfd, nready, client[FD_SETSIZE];

    /* create socket */
    sockfd = Socket(AF_INET,SOCK_DGRAM,0);

    /* set parameters for sockaddr_in */
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT); //0, assign port automatically in 1024 ~ 65535
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //0, got local IP automatically
    bzero(&(server_sockaddr.sin_zero), 8);
   
    int i = 1;//enable reuse the combination of local address and socket
    //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    Bind(sockfd, server_sockaddr);

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
    port = Getsockname(sockfd, server_sockaddr, sin_size);  /* Get the port number assigned*/
    writePort(port, addrstr);
   
    snprintf(logmsg, sizeof(logmsg), "sockserver: Server %s (%s) is setup on port: %d\n", addrstr, hostname, port);
    logging(LOGFILE, logmsg);
   
    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // When a thread is created detached, its thread ID and other resources can be reused as soon as the thread terminates.
    int iThread = 0; // Thread iterator
    struct thread_info *tinfo;
    tinfo = calloc(1, sizeof(struct thread_info));
    //pthread_create(&tinfo->thread_id, &attr, &serverthread, &tinfo);

    ThreadSocket *threadSocket;
    threadSocket = (ThreadSocket *)malloc(sizeof(ThreadSocket));
    threadSocket->sockfd = sockfd;
    threadSocket->sockaddr = server_sockaddr;

    int init = pthread_mutex_init(&mutex, NULL); // initialize the mutex
    if(init != 0)
    {
       fprintf(stderr, "mutex init failed \n");
       exit(1);
    }

    /* Set up the I/O for the socket, select mode */
    maxfd = sockfd;
    int k;
    for(k=0;k<FD_SETSIZE;k++){
        client[k] = -1;
    }
  
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

        FD_ZERO(&ready_set);
        FD_ZERO(&test_set);
        FD_SET(sockfd, &test_set);
        maxfd = sockfd;

        memcpy(&ready_set, &test_set, sizeof(test_set));
        nready = select(maxfd+1, &ready_set, NULL, NULL, tvptr);

        switch(nready){
            case -1:
                fprintf(stderr, "sockserver: errno: %d.\n", errno);
                perror("\nSELECT: unexpected error occured.\n");
                logging(LOGFILE, "\nSELECT: unexpected error occured.\n");

                /* remove bad fd */
                for(k=0;k<FD_SETSIZE;k++){
                    if(client[k] > 0){
                        struct stat tStat;
                        if (-1 == fstat(client[k], &tStat)){
                            fprintf(stderr, "fstat %d error:%s", sockfd, strerror(errno));
                            FD_CLR(client[k], &ready_set);
                        }
                    }
                }
                status=-1;
                break;
            case 0:
                /* timeout occuired */
                fprintf(stderr, "sockserver: TIMEOUT... %d.\n", errno);
                status=-1;
                break;
            default:
                if (FD_ISSET(sockfd, &ready_set)){

                    snprintf(logmsg, sizeof(logmsg), "sockserver(0x%x): Descriptor %d is readable\n",  pthread_self(), sockfd);
                    logging(LOGFILE, logmsg);
                    //pthread_create(&threadid[iThread], &attr, &server_thread, (void *)threadSocket);
                    //pthread_join(threadid[iThread], NULL);
                    //iThread++;
                    server_thread(sockfd, server_sockaddr);
                }// end if (FD_ISSET(i, &ready_set))
        }// end switch
    } // end while (status==CONTINUE)
    close(sockfd);
    unlinkPortFile(addrstr);

    return 0;
}
