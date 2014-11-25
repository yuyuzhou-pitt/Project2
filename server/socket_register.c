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
#define REGISTER_SERV 1
#include "../packet/packet.h"
#include "../packet/hello.h"
#include "../packet/register.h"
#include "../lib/libsocket.h"
#include "../lib/getaddrinfo.h"
#include "../lib/liblog.h"
#include "../lib/libfile.h"
#include "../lib/libterminal.h"

#define PORT   0 //4321
#define BUFFER_SIZE 1024
#define READ_PORT_INTERVAL 20 // busy wait interval to read port files


char hostname[1024]; // local hostname and domain
char addrstr[100]; // local ip address (eth0)

typedef struct HelloArgs{
    struct sockaddr_in sockaddr;
    Packet *packet;
}HelloArgs;

pthread_mutex_t register_mutex;
pthread_mutex_t hello_mutex;

/* thread for hello packet */
void *sockhello(void *arg){

    HelloArgs *helloArgs;
    helloArgs = (HelloArgs *)malloc(sizeof(HelloArgs));
    helloArgs = (HelloArgs *)arg;

    int clientfd;

    while(1){
        clientfd = Socket(AF_INET, SOCK_STREAM, 0);
        Connect(clientfd, helloArgs->sockaddr, sizeof(helloArgs->sockaddr));
        pthread_mutex_lock(&hello_mutex); // Critical section start
        sendHello(clientfd, helloArgs->packet);
        pthread_mutex_unlock(&hello_mutex); // Critical section end
        
        sleep(HELLO_INTERVAL); //sleep interval time
    } //endof while(1)

}

/* thread for server registeration */
void *sockregister(void *arg){
    struct OptionsStruct *options;
    options = (struct OptionsStruct *)malloc(sizeof(struct OptionsStruct));
    options = (struct OptionsStruct *) arg;

    int clientfd,sendbytes,recvbytes;
    struct hostent *host;
    struct sockaddr_in sockaddr;

    getaddr(hostname, addrstr); //get hostname and ip, getaddrinfo.h
   
    /* try to get server hostname and port from the host files (.<servername>) */
    char remote_server[32];
    char remote_ipstr[32];
    char remote_portstr[6]; // to store the port

    memset(remote_ipstr, 0, sizeof(remote_ipstr));
    memset(remote_portstr, 0, sizeof(remote_portstr));

    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "server_lient(0x%x): busy wait for port mapper starting up...\n", pthread_self());
    logging(LOGFILE, logmsg);

    int portFound = 0; // check port file exists or not
    while(1){
        memset(remote_portstr, 0, sizeof(remote_portstr)); 
        /* Steps:
         * 1) to find port file .<port-mapper-ip> for port-mapper
         * 2) if port file do NOT exists, busy wait
         * 3) if port file exists, communicate port-mapper through this port
         */

        /* 1) go through all the direct_link_addr in cfg file */
        char templine[32];

        int iget;
        /* 2) if port file do NOT exists, busy wait */
        if((iget = getPortMapper(remote_portstr, remote_ipstr, PORT_MAPPER_FILE)) < 0 ){ // read port file

            sleep(READ_PORT_INTERVAL);
            printf("client(0x%x): No available port found, make sure port mapper started.\n", pthread_self());
            printf("client(0x%x): wait %d seconds to try again..\n", pthread_self(), READ_PORT_INTERVAL);

            //continue; // if file does not exists, continue
        }
        /* 3) if port file exists, communicate port-mapper through this port*/
        else if(strlen(remote_portstr) == 5){
            portFound = 1;
            break;
        }
    }

    /* There are 3 types of Packets to be exchanged via server for registering:
     * 1) Register service (from server to port mapper)         (000)
     * 2) Register acknowledge (from port mapper to server)     (001)
     * 3) Hello Packets (from server to port mapper)            (111)
     * */

        if(portFound == 1){
            if((host = gethostbyname(remote_ipstr)) == NULL ) { // got the remote server
                perror("gethostbyname");
                exit(-1);
            };

            /* connect to remote server */
            /*create socket*/
            clientfd = Socket(AF_INET, SOCK_STREAM, 0);

            /*parameters for sockaddr_in*/
            sockaddr.sin_family = AF_INET;
            sockaddr.sin_port = htons(atoi(remote_portstr));
            sockaddr.sin_addr = *((struct in_addr *)host->h_addr);
            bzero(&(sockaddr.sin_zero), 8);

            /*connect to server*/
            Connect(clientfd,sockaddr,sizeof(sockaddr));

        }

    pthread_mutex_lock(&register_mutex);

    Packet *packet_req, *packet_reply;

    /* generate packet_req, provide:
     * - program_name, version_number (options)
     * - sender_ip (addrstr)
     * - portmapper_ip (remote_ipstr)*/
    packet_req = genRegister(options, addrstr, remote_ipstr); // msg to be sent out
    send(clientfd, packet_req, sizeof(Packet), MSG_NOSIGNAL);

    packet_reply = (Packet *)malloc(sizeof(Packet));
    /* Receive neighbors_reply from remote side */
    Recv(clientfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);

    pthread_mutex_unlock(&register_mutex);

    /*packet_reply->Data.procedure_number should be packet_reply->Data.dup_numbers*/
    if(strcmp(packet_reply->packet_type, "001") == 0) {

        /*packet_reply->Data.dup_numbers = dup_number * 10 + server_exists;*/
        int server_exists = (packet_reply->Data.procedure_number) % 10;
        int dup_number = (packet_reply->Data.procedure_number) / 10;

        /*check is duplicated or not*/
        if (dup_number == 0){
            fprintf(stdout, "Congratulations, %d services rigistered successfully!\n",
                   packet_req->Data.procedure_number);
        }
        //else if (dup_number == packet_req->Data.procedure_number){
        //    fprintf(stdout, "Duplicated rigisteration! Check your commands!\n");
        //}
        else{
            fprintf(stdout, "Duplication found! %d of %d services rigistered successfully.\n",
                    packet_req->Data.procedure_number - dup_number,
                    packet_req->Data.procedure_number);
        }

        /*send regular hello for new server after register acknowledged*/
        if(server_exists == 0){
            HelloArgs *helloArgs;
            helloArgs = (HelloArgs *)malloc(sizeof(HelloArgs));

            helloArgs->sockaddr = sockaddr;
            helloArgs->packet = packet_reply;

            pthread_t sockhelloid;
            pthread_create(&sockhelloid, NULL, &sockhello, (void **)helloArgs);
            //pthread_join(sockhelloid, NULL);
        }
        else{
            fprintf(stdout, "Server %s has registerd before, no hello packet sent.\n", packet_req->sender_id);
        }

    } // endof if(strcmp(packet_reply

    free(packet_reply);
    free(options);
    close(clientfd);
    pthread_exit(0);
}

// start a client thread to register the services to port mapper table
int registerServices(struct OptionsStruct *options){
    char logmsg[128];
    snprintf(logmsg, sizeof(logmsg), "(server): socket client started, to be communicated with port mapper.\n");
    logging(LOGFILE, logmsg); 

    pthread_t sockregisterid;
    pthread_create(&sockregisterid, NULL, &sockregister, (void **)options);
    pthread_join(sockregisterid, NULL);

    return 0;
}
