/* SRPC Server, it plays two roles:
 *  1. as socket server to provide services to client
 *  2. as socket client to register services into port mapper table
 *
 * Steps to build and run:
 * $ make
 * $ ./server
 * $ (server)$ register <program-name> <version-number>
 * $ (server)$ list
 *   Program Name | Version | Procedure | reg-or-not
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <ctype.h>

#define EXECUTE_SERV 1
#include "../packet/packet.h" //for Packet_Seq, use Execute_Serv to be the packet type

#include "socket_register.h"
#include "socket_server.h"
#include "../lib/libsocket.h"
#include "../lib/getaddrinfo.h"
#include "../lib/liblog.h"
#include "../lib/libterminal.h"
#include "../lib/libscientific.h"

#define PORT 0 //0 means assign a port randomly
#define BUFFER_SIZE  1024
#define NTHREADS 128

#include "../packet/linkseq.h"
#include "../packet/linkload.h"

/*global variables defined in packet.h*/
Packet_Seq *executePacketSeq; //the execute send sequence
Packet_Seq *executePacketEnd; //the execute send end
Packet_Seq *executeResultSeq; //the execute reply sequence
Packet_Seq *executeResultEnd; //the execute reply end

LoadLink *loadBalanceLinkHead; //to keep load balance
LoadLink *loadBalanceLinkEnd; //to keep load balance

RemoteProgram *(*libraryPtr)();

int main(int argc, char *argv[]){

    if(argc > 1){
        fprintf(stderr, "USAGE: ./server (no options required)\n");
        exit(1);
    }

    libraryPtr = getLibraryPtr(); // configurable library function

    //executePacketSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //executeResultSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //executePacketSeq = initListSeq();
    //executeResultSeq = initListSeq();
    initExecuteSeq();
    /*initial the load balance link for client request service use*/
    initLoadSeq();

    /* Thread attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_RR); // Round Robin scheduling for threads 
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Don't want threads (particualrly main)

    /* start socket server for client use */
    int sockfd;
    pthread_t sockserverid;
    pthread_create(&sockserverid, NULL, &sockserver, (void *)sockfd);
    fprintf(stdout, "(server): socket server start up.\n");

    /* start SRPC terminal */
    fprintf(stdout, "(server): please track log file for detail: %s\n", LOGFILE);
    fprintf(stdout, "\n== WELCOME TO SRPC TERMINAL FOR SERVER! ==\n");
    fprintf(stdout, "\nEnter the commands 'help' for usage.\n\n");
    
    int rc;
    char command[64]; //, options[3][32];
    char command_no_n[64]; //, options[3][32];
    struct OptionsStruct *options;
    options = (struct OptionsStruct *)malloc(sizeof(struct OptionsStruct));
    int terminal = 1;
    char *sub_command; // sub-command and options
    while(terminal == 1){
        //fprintf(stdout, "(server: %s)# ", router->router_id);
        fprintf(stdout, "(server)# ");
	fflush(stdin);
	if (fgets(command, sizeof(command), stdin) != NULL ){
            /*convert input as array, such as:
             * (server): register <program_name> <version_number>
             * stores in struct as:
             * command = "register"
             * option1 = "<program_name>"
             * option2 = "<version_number>"
             */
            snprintf(command_no_n, strlen(command), "%s", command);
            options = command2struct(command_no_n);
            //OptionsStruct optionStruct = 
            sub_command = options->command;

            if(strcmp(sub_command, "quit") == 0){
                /* TODO: clean threads before quit terminal */
                if((terminal = quit()) == 0){
                    break; // or exit(0);
                }
            }
            else if(strcmp(sub_command, "help") == 0){
                helpServer();
            }
            else if(strcmp(sub_command, "register") == 0){
                /* register format:
                 * # server register <program-name> <version-number> */
                struct Remote_Program_Struct *sciLibrary;
                sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data
                sciLibrary = (*libraryPtr)();
                if(strcmp(options->option1, sciLibrary->program_name) != 0){
                    fprintf(stdout, "Hey, please try to register our flagship program: %s.\n", sciLibrary->program_name);
                    free(sciLibrary);
                    continue;
                }
                else if(strcmp(options->option2, sciLibrary->version_number) != 0){
                    fprintf(stdout, "Sorry, the supported version number is %s.\n", sciLibrary->version_number);
                    continue;
                }
                else{
                    registerServices(options);
                }
            }
            else if(strcmp(sub_command, "list") == 0){
                //listServices();
            }
            else if(strcmp(sub_command, "(null)") != 0){
                fprintf(stderr, "ERROR: command not found: %s\n", sub_command);
                helpServer();
            }

        }
        else{
            fprintf(stderr, "Not all fields are assigned\n");
        }

    }

    free(options);
    free(executeResultSeq);
    free(executePacketSeq);
    //pthread_join(sockserverid, NULL);
    return 0;
}
