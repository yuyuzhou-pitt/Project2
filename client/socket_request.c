/* As socket client:
 * 1) client starts and run into busy wait
 * 2) to get port-mapper IP:Port for PORT_MAPPER_FILE
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
#define REQUEST_SERV 1
#include "../packet/packet.h"
#include "../packet/request.h"
#include "../packet/request_reply.h"
#include "../lib/libsocket.h"
#include "../lib/liblog.h"
#include "../lib/libfile.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"
#include "../lib/getaddrinfo.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024
#define READ_PORT_INTERVAL 20 // busy wait interval to read port files

#define HELLO_INTERVAL 40 // in seconds

char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

/*two global viriables defined in packet.h
 * * client request will set this two viriables from request reply*/
char exec_remote_ipstr[1024];
char exec_remote_port[6];

pthread_mutex_t request_mutex;

/* thread for lsrp-client */
void *requestclient(void *arg){
    struct OptionsStruct *reqeust_options;
    reqeust_options = (struct OptionsStruct *)malloc(sizeof(struct OptionsStruct));
    reqeust_options = (struct OptionsStruct *) arg;

    int clientfd;
    struct hostent *host;
    struct sockaddr_in sockaddr;

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
   
    /* try to get server hostname and port from the port mapper files (.port_mapper) */
    char remote_ipstr[32];
    char remote_portstr[6]; // to store the port

    memset(remote_ipstr, 0, sizeof(remote_ipstr)); 
    memset(remote_portstr, 0, sizeof(remote_portstr)); 
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "client(0x%x): busy wait for port mapper starting up...\n", pthread_self());
    logging(LOGFILE, logmsg);

    int portFound = 0; // check port file exists or not
    while(1){
        /* Steps:
         * 1) to find port file .port_mapper for port-mapper
         * 2) if port file do NOT exists, busy wait
         * 3) if port file exists, communicate port-mapper through this port
         */

        /* 1) go through all the direct_link_addr in cfg file */

        int iget;
        /* 2) if port file do NOT exists, busy wait */
        if((iget = getPortMapper(remote_portstr, remote_ipstr, PORT_MAPPER_FILE)) < 0 ){ // read port file
            sleep(READ_PORT_INTERVAL);
            printf("client(0x%x): No available port found, make sure port mapper started.\n", pthread_self());
            printf("client(0x%x): wait %d seconds to try again..\n", pthread_self(), READ_PORT_INTERVAL);
          
            continue; // if file does not exists, continue
        }
        /* 3) if port file exists, communicate port-mapper through this port*/
        else if(strlen(remote_portstr) == 5){
            portFound = 1;
        }

        if(portFound == 1){
            if((host = gethostbyname(remote_ipstr)) == NULL ) { // got the remote server
                perror("gethostbyname");
                exit(-1);
            };

            /*create socket*/
            clientfd = Socket(AF_INET, SOCK_STREAM, 0);
        
            /*parameters for sockaddr_in*/
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_port = htons(atoi(remote_portstr));
            sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
            bzero(&(sockaddr.sin_zero), 8);         
        
            /*connect to server*/
            Connect(clientfd,sockaddr,sizeof(sockaddr));

            break; //end the for loop
        }
    
        if(strlen(remote_portstr) == 5){
            break; // end the while loop
        }
    }

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

    Packet *packet_req, *packet_reply;
    packet_req = (Packet *)malloc(sizeof(Packet));

    pthread_mutex_lock(&request_mutex);
    /* generate packet_req, provide:
     * - program_name, version_number (reqeust_options)
     * - sender_ip (addrstr)
     * - portmapper_ip (remote_ipstr)*/
    packet_req = genRequest(reqeust_options, addrstr, remote_ipstr); // msg to be sent out
    send(clientfd, packet_req, sizeof(Packet), MSG_NOSIGNAL);

    packet_reply = (Packet *)malloc(sizeof(Packet));
    //Recv(clientfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);
    packet_reply = recvRequestReply(clientfd);

    pthread_mutex_unlock(&request_mutex);
    // got response
    if(strcmp(packet_reply->packet_type, "011") == 0){
        /*print response only when reuqest service only*/
        //#ifdef REQUEST_SERV
        printRequestReply(packet_reply, exec_remote_ipstr, exec_remote_port);        
        //fprintf(stdout, "\nChoose to use service on %s with port %s for load balance.\n\n",
        //        exec_remote_ipstr, exec_remote_port);

        //#endif
    } // endof if(strcmp(packet_reply

    free(packet_req);
    free(packet_reply);
    //free(reqeust_options);
    close(clientfd);
    pthread_exit(0);
}

// start a client thread to request the services from port mapper table
int requestServices(struct OptionsStruct *reqeust_options){
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "(client): socket client started, to be communicated with port mapper.\n");
    logging(LOGFILE, logmsg); 

    pthread_t sockclientid;
    pthread_create(&sockclientid, NULL, &requestclient, (void **)reqeust_options);
    pthread_join(sockclientid, NULL);

    return 0;
}
