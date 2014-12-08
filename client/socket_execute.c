/* As socket execute client:
 * 1) client starts and run into busy wait
 * 2) keep send execute request packet to server
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>

#include "../lib/unprtt.h"

#define EXECUTE_SERV 1
#include "../packet/packet.h"

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

#define NTHREADS 20 
pthread_t connect_id[NTHREADS]; // Thread pool

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

/*two global viriables defined in packet.h
* client request will set this two viriables from request reply*/

pthread_mutex_t execute_mutex;

//#define TIMESTAMP 1

static struct rtt_info   rttinfo;
static int      rttinit = 0;
static sigjmp_buf jmpbuf;

static void sig_alrm(int signo){
    siglongjmp(jmpbuf,1);
}

SplitStr *terms; //the terms to be search

int i_thread = 0;

/*handle the server connection*/
void connectServer(void *arg){
//void connectServer(OptionsStruct *exec_options){

    OptionsStruct *exec_options;
    exec_options = (OptionsStruct *)malloc(sizeof(OptionsStruct));
    exec_options = (OptionsStruct *)arg;

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
    while(wait_count < 3 && (host = gethostbyname(exec_options->remote_ipstr)) == NULL ) { // got the remote server
        //perror("gethostbyname");
        //exit(-1);
        sleep(1);
        wait_count ++;
    };

    if(host == NULL ) { // got the remote server
        //fprintf(stdout, "No server found, please try again.\n");
        return -1;
    }

    //printf("exec_remote_ipstr=%s.\n", exec_remote_ipstr);

    /*create socket*/
    clientfd = Socket(AF_INET, SOCK_DGRAM, 0);
       
    /*parameters for sockaddr_in*/
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(atoi(exec_options->remote_port));
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
    //fprintf(stderr, "\n(TS:%d) Start marshaling from the input file %s.\n\n", time1, exec_options->option4);
    sequence_execute = Execute_stub(exec_options, addrstr, exec_options->remote_ipstr, trans_id); // msg to be sent out
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
            //fprintf(stderr, "Sending execute packet with seq %d to Server %s.\n", seq_cursor->cur.Data.seq, seq_cursor->cur.receiver_id);
            seq_cursor->cur.Data.ts = rtt_ts(&rttinfo);
            sendExecuteServ(clientfd, sockaddr, &(seq_cursor->cur));
        }
        seq_cursor = seq_cursor->next;
    }

    alarm(rtt_start(&rttinfo));     /* calc timeout value & start timer */
    if(sigsetjmp(jmpbuf,1)!=0){
        if(rtt_timeout(&rttinfo)<0){
            rttinit = 0;    /* reinit in case we're called again */
            //fprintf(stderr, "No response from server.\n");
            return(0);
        }
        //fprintf(stderr, "Waiting for ACK time out, resend the packet.\n");
        seq_cursor = sequence_execute->next; //reset seq cursor
        goto sendagain;
    }
        
    do{
        //printf("==execlient 2==\n");
        /*wait for the bulk ack*/
        //fprintf(stderr, "Waiting for ACK packet...\n");
        packet_ack = recvExecuteAck(clientfd);
    }while(strcmp(packet_ack->packet_type, "110") != 0);

    alarm(0);                       /* stop SIGALRM timer */
           /* 4calculate & store new RTT estimator values */
    rtt_stop(&rttinfo, rtt_ts(&rttinfo) - packet_ack->Data.ts);

    //printf("==execlient 3==\n");
    //fprintf(stderr, "\n(TS:%d) Got bulk packet ack from Server %s: acknowledged %d execute packet(s). \n", 
    //        getTimeStamp(), packet_ack->sender_id, packet_ack->Data.seq);

    /*receive the left packets of the result*/
    if(packet_ack->Data.end_flag == 1){
        //printf("==execlient 4==\n");
        do{
            //printf("==execlient 5==\n");
            packet_reply = recvExecuteResult(clientfd); //receive and insert result into executePacketSeq
            //fprintf(stderr, "Got packet result with seq %d from Server %s.\n", packet_reply->Data.seq, packet_reply->sender_id);
            //appendListSeq(executeResultSeq, *packet_reply);
        }while(isEndTransaction(executeResultSeq, packet_reply->Data.transaction_id) != 1);

        //printf("==execlient 6==\n");
        //fprintf(stderr, "\n(TS:%d) Sending bulk ack to Server %s: %d packet(s) received.\n", 
        //      getTimeStamp(), packet_reply->receiver_id, packet_reply->Data.seq + 1);

        sendExecuteAck(clientfd, sockaddr, packet_reply);

        /*if result data is fle, then write result into file*/
        if(packet_ack->Data.data_is_file_or_dir == 0){
            //fprintf(stderr, "\n(TS:%d) Writing result to output file %s.\n", getTimeStamp(), exec_options->option5);
            writeResultSeq(executeResultSeq, exec_options->option5); //option5 is the output file
        }
        else if(packet_ack->Data.data_is_file_or_dir == 1){
            /*client decide which action to execute*/
            snprintf(exec_options->option4, sizeof(exec_options->option4), packet_reply->Data.para_data.data_str);
        }

        int time2 = getTimeStamp();
        //fprintf(stderr, "\n(TS:%d) Congratulations! RPC %s completed successfully.\n\n", 
        //       time2, packet_ack->Data.procedure_name);

        #ifdef TIMESTAMP
        fprintf(stderr, "**************************\n");
        fprintf(stderr, "** Time elapsed %d us. **\n", time2 - time1);
        fprintf(stderr, "**************************\n");
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

int sortJobTracker(OptionsStruct *exec_options){
    /*clear temp directory before execution*/
    char temp_dir[128];
    snprintf(temp_dir, sizeof(temp_dir), "../.%s", exec_options->action);
    struct stat st = {0};
    if (stat(temp_dir, &st) != -1) {
        rmrf(temp_dir);
    }

    struct timeval t_stamp;

    char new_dir[128];
    char src_dir[128];
    //int i_thread = 0; // Thread iterator
    snprintf(src_dir, sizeof(src_dir), exec_options->option4);
    int sN;
    for (sN=0;sN < requested_servers->response_number;sN++){
        if (i_thread == NTHREADS){
            i_thread = 0;
        }

        t_stamp = getUTimeStamp();
        snprintf(new_dir, sizeof(new_dir), "%s___%d_%d", src_dir, t_stamp.tv_sec, t_stamp.tv_usec);
        copy_dir(src_dir, new_dir);

        snprintf(exec_options->option4, sizeof(exec_options->option4),
                 "%s", new_dir); //exec_pkt->Data.para_data.data_str
        snprintf(exec_options->remote_ipstr, sizeof(exec_options->remote_ipstr),
                 "%s", requested_servers->portMapperTable[sN].server_ip); //socket host
        snprintf(exec_options->remote_port, sizeof(exec_options->remote_port),
                 "%s", requested_servers->portMapperTable[sN].port_number); // socket port

        exec_options->server_no = sN;
        exec_options->server_number = requested_servers->response_number;

        fprintf(stderr, "Sorting directory %s on server %s (%d of %d).\n", exec_options->option4,
                exec_options->remote_ipstr, sN+1, requested_servers->response_number);

        /*connectServer(exec_options);*/
        pthread_create(&connect_id[i_thread], NULL, &connectServer, (void **)exec_options);
        pthread_join(connect_id[i_thread], NULL);
    }

    return 0;
}

int searchJobTracker(OptionsStruct *exec_options, SplitStr *s_terms){
    /*clear temp directory before execution*/
    char temp_dir[128];
    snprintf(temp_dir, sizeof(temp_dir), "../.%s", exec_options->action);
    struct stat st = {0};
    if (stat(temp_dir, &st) != -1) {
        rmrf(temp_dir);
    }

    /*go through the files in the directory*/
    DIR *in_dp;
    struct dirent *in_ep;

    int hash_index = 0;
    //int i_thread = 0; // Thread iterator
    char file_path[128];
    char input_dir[128];
    snprintf(input_dir, sizeof(input_dir), exec_options->option4);

    char result_file[128];
    char output_file[128];
    int iN;
    for(iN=0;iN < s_terms->count;iN++){
        fprintf(stderr, "Searching for term: %s...\n", s_terms->terms[iN]);
        in_dp = opendir (input_dir);
        if (in_dp == NULL){
            perror ("Couldn't open the directory");
            return -1;
        }

        int file_found = 0;
        while (in_ep = readdir (in_dp)){
            if (i_thread == NTHREADS){
                i_thread = 0;
            }

            //fprintf(stderr, "in_ep->d_name=%s.\n", in_ep->d_name);
            /*skip all the none-text file*/
            if(strcmp(getStrAfterDelimiter(in_ep->d_name, '.'), "txt") != 0){
                continue;
            }

            if(s_terms->terms[iN][0] == in_ep->d_name[0]){
                file_found = 1;
                hash_index = iN % requested_servers->response_number;
                snprintf(file_path, sizeof(file_path), "%s/%s", input_dir, in_ep->d_name);

                snprintf(exec_options->term, sizeof(exec_options->term), s_terms->terms[iN]);
                snprintf(exec_options->option4, sizeof(exec_options->option4),
                         "%s", file_path); //exec_pkt->Data.para_data.data_str
                snprintf(exec_options->remote_ipstr, sizeof(exec_options->remote_ipstr),
                         "%s", requested_servers->portMapperTable[hash_index].server_ip); //socket host
                snprintf(exec_options->remote_port, sizeof(exec_options->remote_port),
                         "%s", requested_servers->portMapperTable[hash_index].port_number); // socket port
    
                fprintf(stderr, "Searching in file: %s.\n", exec_options->option4);
                //fprintf(stderr, "exec_options->option4=%s.\n", exec_options->option4);
    
                /*connectServer(exec_options);*/
                pthread_create(&connect_id[i_thread], NULL, &connectServer, (void **)exec_options);
                pthread_join(connect_id[i_thread], NULL);
    
                //fprintf(stderr, "exec_options->option4=%s.\n", exec_options->option4);
                i_thread++;
                break;
            }
        }
        (void) closedir (in_dp);

        snprintf(output_file, sizeof(result_file), "%s/%s.txt", exec_options->option5, s_terms->terms[iN]); // the result from index

        /*check the result from remote*/
        if(file_found == 1){
            snprintf(result_file, sizeof(result_file), "../.%s_%s/%s.txt", addrstr, SINGLE, s_terms->terms[iN]); // the result from index
            if(copy_file(result_file, output_file) != 0)
                fprintf(stderr, "Error copying file: %s!\n", output_file);
        }
        else{
            char w_line[128];
            snprintf(w_line, sizeof(w_line), "Sorry, term \"%s\" found nowhere.\n", s_terms->terms[iN]);
            writeFile(w_line, strlen(w_line), output_file, "w");
        }
    }

    return 0;
}

int indexJobTracker(OptionsStruct *exec_options){
    /*clear temp directory before execution*/
    char temp_dir[128];
    snprintf(temp_dir, sizeof(temp_dir), "../.%s", exec_options->action);
    struct stat st = {0};
    if (stat(temp_dir, &st) != -1) {
        rmrf(temp_dir);
    }

    /*go through the files in the directory*/
    DIR *in_dp;
    struct dirent *in_ep;

    in_dp = opendir (exec_options->option4);
    int file_count = 0;
    int hash_index = 0;
    //int i_thread = 0; // Thread iterator
    char file_path[128];
    char input_dir[128];
    snprintf(input_dir, sizeof(input_dir), exec_options->option4);
    if (in_dp != NULL){
        while (in_ep = readdir (in_dp)){
            if (i_thread == NTHREADS){
                i_thread = 0;
            }

            //fprintf(stderr, "in_ep->d_name=%s.\n", in_ep->d_name);
            /*skip all the none-text file*/
            if(strcmp(getStrAfterDelimiter(in_ep->d_name, '.'), "txt") != 0){
                continue;
            }
            hash_index = file_count % requested_servers->response_number;
            snprintf(file_path, sizeof(file_path), "%s/%s", input_dir, in_ep->d_name);
            snprintf(exec_options->option4, sizeof(exec_options->option4),
                     "%s", file_path); //exec_pkt->Data.para_data.data_str
            snprintf(exec_options->remote_ipstr, sizeof(exec_options->remote_ipstr),
                     "%s", requested_servers->portMapperTable[hash_index].server_ip); //socket host
            snprintf(exec_options->remote_port, sizeof(exec_options->remote_port),
                     "%s", requested_servers->portMapperTable[hash_index].port_number); // socket port

            fprintf(stderr, "Handling file: %s.\n", exec_options->option4);
            //fprintf(stderr, "exec_options->option4=%s.\n", exec_options->option4);

            /*connectServer(exec_options);*/
            pthread_create(&connect_id[i_thread], NULL, &connectServer, (void **)exec_options);
            pthread_join(connect_id[i_thread], NULL);

            //fprintf(stderr, "exec_options->option4=%s.\n", exec_options->option4);
            file_count ++;
            i_thread++;
        }
        (void) closedir (in_dp);
    }
    else
        perror ("Couldn't open the directory");

    return 0;
}

/* thread for lsrp-client */
void *execlient(void *arg){
    //int i_thread = 0;

    OptionsStruct *exec_options;
    exec_options = (OptionsStruct *)malloc(sizeof(OptionsStruct));
    exec_options = (OptionsStruct *)arg;

    /*wait until the client request get the result*/
    int wait_count = 0;

    /*do NOT run multiple command execute within 5 seconds*/
    while(wait_count < 3000 && requested_servers->response_number <= 0) {
        usleep(1000);
        wait_count ++;
    };

    if(requested_servers->response_number <= 0){
        exit(1);
    }

    /*create the output directory if not exists*/
    struct stat st = {0};
    if (stat(exec_options->option5, &st) == -1) {
        mkdir(exec_options->option5, 0700);
    }

    //fprintf(stderr, "requested_servers->response_number=%d.\n", requested_servers->response_number);

    /*client decide which action to execute*/
    if(strcmp(exec_options->option3, INDEX) == 0){
        fprintf(stderr, "###### Start spliting: %s ######\n", exec_options->option4);
        /*split input file into specific bloks*/
        snprintf(exec_options->action, sizeof(exec_options->action), "%s", SPLIT);
        indexJobTracker(exec_options);

        fprintf(stderr, "###### Start wordcounting: %s ######\n", exec_options->option4);
        /*collect word count in each file*/
        snprintf(exec_options->action, sizeof(exec_options->action), "%s", WORDCOUNT);
        indexJobTracker(exec_options);

        fprintf(stderr, "###### Start sorting: %s ######\n", exec_options->option4);
        /*sort the file to put the same term in the same file (AlphaBeta)*/
        snprintf(exec_options->action, sizeof(exec_options->action), "%s", SORT);
        sortJobTracker(exec_options);

        fprintf(stderr, "###### Start reducing: %s ######\n", exec_options->option4);
        /*reduce the file to merge the same term (master inverted index) (AlphaBeta)*/
        snprintf(exec_options->action, sizeof(exec_options->action), "%s", MII); // the result of master inverted index
        indexJobTracker(exec_options);

        fprintf(stderr, "###### Indexing task done, check your result please: %s. ######\n", exec_options->option5);
    }
    else if(strcmp(exec_options->option3, SEARCH) == 0){
        fprintf(stderr, "###### Start searching term(s): %s ######\n", exec_options->option6);

        /*turn the terms into array*/
        terms = (SplitStr *)malloc(sizeof(SplitStr));
        str2array(terms, exec_options->option6, ' ');  //split by space

        /*start the searching job, handle with one term*/
        snprintf(exec_options->action, sizeof(exec_options->action), "%s", SINGLE);
        searchJobTracker(exec_options, terms);

        /*start the searching job, merge the result*/
        //snprintf(exec_options->action, sizeof(exec_options->action), "%s", MERGE);
        //searchJobTracker(exec_options, terms);
        fprintf(stderr, "###### Searching task done, check your result please: %s. ######\n", exec_options->option5);
    }

    pthread_exit(0);
}

// start a client thread to execute the services from server
int executeServices(OptionsStruct *exec_options){
    char logmsg[128];
    pthread_t execlientid;
    pthread_create(&execlientid, NULL, &execlient, (void **)exec_options);
    pthread_join(execlientid, NULL);
    snprintf(logmsg, sizeof(logmsg), "(client): socket client started, to be communicated with port mapper.\n");
    logging(LOGFILE, logmsg); 

    return 0;
}
