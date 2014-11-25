/* As socket execute client:
 * 1) client starts and run into busy wait
 * 2) keep send execute request packet to server
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>

#include "../lib/unprtt.h"

#define EXECUTE_SERV 1
#include "../packet/packet.h"

//#define TIMESTAMP 1
#include "../lib/libfile.h"

#include "client_stub_send.h"
#include "../packet/linkseq.h"
#include "../packet/execute.h"
#include "../packet/execute_ack.h"
#include "../packet/execute_result.h"
#include "../lib/libsocket.h"
#include "../lib/liblog.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"
#include "../lib/getaddrinfo.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024
#define RESEND_TIMEOUT 1000   // wait 100ms, if no ack, resend packets.
#define RESEND_MAX_TIMES 20   // resend up to 20 times, that is 2 second

#define WINDOW_SIZE 4 // windows size for sliding window

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

/*two global viriables defined in packet.h
* client request will set this two viriables from request reply*/

pthread_mutex_t execute_mutex;

#define TIMESTAMP 1

static struct rtt_info   rttinfo;
static int      rttinit = 0;
static sigjmp_buf jmpbuf;

static void sig_alrm(int signo){
  siglongjmp(jmpbuf,1);
}

/* thread for lsrp-client */
void *execlient(void *arg){

    struct OptionsStruct *exec_options;
    exec_options = (struct OptionsStruct *)malloc(sizeof(struct OptionsStruct));
    exec_options = (struct OptionsStruct *) arg;

    int clientfd; //,sendbytes,recvbytes;
    struct hostent *host;
    struct sockaddr_in sockaddr;

    /*get local hostname and ip*/
    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h

    //int counter = 0;
    //int ack_flag = 0;

    /*wait until the client request get the result*/
    int wait_count = 0;

    /*do NOT run multiple command execute within 3 seconds*/
    while(wait_count < 3 && (host = gethostbyname(exec_remote_ipstr)) == NULL ) { // got the remote server
        //perror("gethostbyname");
        //exit(-1);
        sleep(1);
        wait_count ++;
    };

    if(host == NULL ) { // got the remote server
        //fprintf(stdout, "No server found, please try again.\n");
        pthread_exit(0);
    }

    //printf("exec_remote_ipstr=%s.\n", exec_remote_ipstr);

    /*create socket*/
    clientfd = Socket(AF_INET, SOCK_DGRAM, 0);
       
    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(exec_remote_port));
    sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);

/*
 * Packet type can be determined in 3 bits as shown below:
 * Register service (from server to port mapper)         (000)
 * Register acknowledge (from port mapper to server)     (001)
 * Hello Packets (from server to port mapper)            (111)
 * Request server location (from client to portmapper)   (010)
 * Response server location (from portmapper to client)  (011)
 * Execute service (from client to server)               (100)
 * Execute service response (from server to client)      (101)
 * Execute service acknowledge (from server to client)   (110)
 * */

    /*TODO: move the packet_ack related code into packet directory */

    Packet_Seq *sequence_execute, *seq_cursor;
    Packet *packet_ack, *packet_reply;
    packet_ack = (Packet *)calloc(1, sizeof(Packet));
    packet_reply = (Packet *)calloc(1, sizeof(Packet));

    //int i;

    /* generate packet_req, provide:
     * - program_name, version_number (exec_options)
     * - sender_ip (addrstr)
     * - portmapper_ip (remote_ipstr)*/

    char trans_id[32]; //record the transaction id, in case multiple sequences
    int time1 = getTimeStamp();
    printf("\n(TS:%d) Start marshaling from the input file %s.\n\n", time1, exec_options->option4);
    sequence_execute = Execute_stub(exec_options, addrstr, exec_remote_ipstr, trans_id); // msg to be sent out
    seq_cursor = sequence_execute->next;

    pthread_mutex_lock(&execute_mutex);

    //usleep(100);
    //int is_result = 0;
    /*keep sending if bulk ack failed*/

    if (rttinit == 0) {
        rtt_init(&rttinfo);             /* first time we're called */
        rttinit = 1;
        rtt_d_flag = 1;
    }

    signal(SIGALRM, sig_alrm);
    rtt_newpack(&rttinfo);          /* initialize for this packet */

sendagain:
    /*send the whole sequence all by once*/
    while(seq_cursor->next != seq_cursor){
        if(seq_cursor->cur.Data.transaction_id == atoi(trans_id)){
            printf("Sending execute packet with seq %d to Server %s.\n", seq_cursor->cur.Data.seq, seq_cursor->cur.receiver_id);
            seq_cursor->cur.Data.ts = rtt_ts(&rttinfo);
            sendExecuteServ(clientfd, sockaddr, &(seq_cursor->cur));
        }
        seq_cursor = seq_cursor->next;
    }

    alarm(rtt_start(&rttinfo));     /* calc timeout value & start timer */
    if(sigsetjmp(jmpbuf,1)!=0){
        if(rtt_timeout(&rttinfo)<0){
            rttinit = 0;    /* reinit in case we're called again */
            printf("No response from server.\n");
            return(0);
        }
        printf("Waiting for ACK time out, resend the packet.\n");
        seq_cursor = sequence_execute->next; //reset seq cursor
        goto sendagain;
    }
        
    do{
        //printf("==execlient 2==\n");
        /*wait for the bulk ack*/
        printf("Waiting for ACK packet...\n");
        packet_ack = recvExecuteAck(clientfd);
    }while(strcmp(packet_ack->packet_type, "110") != 0);

    alarm(0);                       /* stop SIGALRM timer */
           /* 4calculate & store new RTT estimator values */
    rtt_stop(&rttinfo, rtt_ts(&rttinfo) - packet_ack->Data.ts);

    //printf("==execlient 3==\n");
    printf("\n(TS:%d) Got bulk packet ack from Server %s: acknowledged %d execute packet(s). \n", 
            getTimeStamp(), packet_ack->sender_id, packet_ack->Data.seq);

    /*receive the left packets of the result*/
    if(packet_ack->Data.end_flag == 1){
        //printf("==execlient 4==\n");
        do{
            //printf("==execlient 5==\n");
            packet_reply = recvExecuteResult(clientfd); //receive and insert result into executePacketSeq
            printf("Got packet result with seq %d from Server %s.\n", packet_reply->Data.seq, packet_reply->sender_id);
            //appendListSeq(executeResultSeq, *packet_reply);
        }while(isEndTransaction(executeResultSeq, packet_reply->Data.transaction_id) != 1);

        //printf("==execlient 6==\n");
        printf("\n(TS:%d) Sending bulk ack to Server %s: %d packet(s) received.\n", 
               getTimeStamp(), packet_reply->receiver_id, packet_reply->Data.seq + 1);

        sendExecuteAck(clientfd, sockaddr, packet_reply);

        printf("\n(TS:%d) Writing result to output file %s.\n", getTimeStamp(), exec_options->option5);
        writeResultSeq(executeResultSeq, exec_options->option5); //option5 is the output file

        int time2 = getTimeStamp();
        printf("\n(TS:%d) Congratulations! RPC %s completed successfully.\n\n", 
               time2, packet_ack->Data.procedure_name);

        #ifdef TIMESTAMP
        printf("**************************\n");
        printf("** Time elapsed %d us. **\n", time2 - time1);
        printf("**************************\n");
        #endif

        /*remove packet_reply transaction from executeResultSeq*/
        clearTransaction(executeResultSeq, packet_reply->Data.transaction_id);
        /*remove packet_execute (packet_ack) transaction from executePacketSeq*/
        clearTransaction(executePacketSeq, atoi(trans_id));
    }
    
    pthread_mutex_unlock(&execute_mutex);

    //free(packet_reply);
    //free(packet_ack);
    //free(exec_options);
    close(clientfd);
    pthread_exit(0);
}

// start a client thread to execute the services from server
int executeServices(struct OptionsStruct *exec_options){
    char logmsg[128];
    pthread_t execlientid;
    pthread_create(&execlientid, NULL, &execlient, (void **)exec_options);
    pthread_join(execlientid, NULL);
    snprintf(logmsg, sizeof(logmsg), "(client): socket client started, to be communicated with port mapper.\n");
    logging(LOGFILE, logmsg); 

    return 0;
}