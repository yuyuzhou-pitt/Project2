/* SRPC Client, it plays two roles:
 *  1. Request client info from SRPC port mapper for certain service
 *  2. Call procedure from SRPC client
 *
 * Steps to build and run:
 * $ make
 * $ ./client
 * 1. # client request <function-name>
 *    Desc: Request client info from port mapper for certain service.
 * 2. # client execute <function_name> <input_file> <output_file>
 *    Desc: Allow users to select which remote procedure call they want to make.
 * 3. # client help
 *    Desc: Print help message
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#define EXECUTE_SERV 1
#include "../packet/packet.h" //for Packet_Seq, use Execute_Serv to be the packet type

#include "socket_request.h"
#include "socket_execute.h"
#include "../server/socket_register.h"
#include "../lib/libsocket.h"
#include "../lib/getaddrinfo.h"
#include "../lib/liblog.h"
#include "../lib/libfile.h"
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
        fprintf(stderr, "USAGE: ./client (no input_options required)\n");
        exit(1);
    }

    libraryPtr = getLibraryPtr(); // configurable library function

    /* start SRPC terminal */
    fprintf(stdout, "(client): please track log file for detail: %s\n", LOGFILE);
    fprintf(stdout, "\n== WELCOME TO SRPC TERMINAL FOR CLIENT! ==\n");
    fprintf(stdout, "\nEnter the commands 'help' for usage.\n\n");
 
    /*initial the global seq for execute client thread*/
    //executePacketSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //executeResultSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //executePacketSeq = initListSeq();
    //executeResultSeq = initListSeq();
    initExecuteSeq();
    /*initial the load balance link for client request service use*/
    initLoadSeq();

    //int rc;
    char command[256]; //, input_options[3][32];
    char command_no_n[256]; //, input_options[3][32];
    memset(command_no_n, 0, sizeof(command_no_n));
    OptionsStruct *input_options;
    input_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data
    sciLibrary = (*libraryPtr)();

    int terminal = 1;
    int printPrompt = 1;
    char sub_command[32]; // sub-command
    while(terminal == 1){
        //fprintf(stdout, "(client: %s)# ", router->router_id);
        if (printPrompt == 1) {
            fprintf(stdout, "(client)# ");
        }
	fflush(stdin);
	if (fgets(command, sizeof(command), stdin) != NULL ){
            snprintf(command_no_n, strlen(command), "%s", command);

            //printf("strlen(command_no_n)=%d.\n", strlen(command_no_n));

            if(strlen(command_no_n) == 0){
                continue;
            }

            input_options = command2struct(command_no_n);
            //printf("input_options->command=%s.\n", input_options->command);
            
            snprintf(sub_command, sizeof(sub_command), "%s", input_options->command);

            if(strcmp(sub_command, "(null)") == 0){
                snprintf(sub_command, sizeof(sub_command), "null");
            }
            if(strcmp(sub_command, "quit") == 0){
                /* TODO: clean threads before quit terminal */
                if((terminal = quit()) == 0){
                    break; // or exit(0);
                }
                // do not print head
                printPrompt = 0;
                continue;
            }
            else if(strcmp(sub_command, "help") == 0){
                helpClient();
            }
            else if(strcmp(sub_command, "request") == 0){
                /* register format:
                 * # request   <program-name> <version-number> <procedure>
                 */
                if(strcmp(input_options->option1, sciLibrary->program_name) != 0){
                    fprintf(stdout, "Hey, please try to request our flagship program: %s.\n", sciLibrary->program_name);
                    continue;
                }
                else if(strcmp(input_options->option2, "1") != 0 && strcmp(input_options->option2, "2") != 0){
                    fprintf(stdout, "Sorry, the supported version number is 1 or 2.\n");
                    continue;
                }

                int pN;
                int pMatch = 0;
                for(pN=0; pN < sciLibrary->procedure_number; pN++){
                    if(strcmp(input_options->option3, sciLibrary->procedures[pN]) == 0){
                        pMatch = 1;
                    }
                }
                if(pMatch == 0){
                    fprintf(stdout, "Sorry, the procedure %s does not support.\n", input_options->option3);
                    continue;
                }

                requestServices(input_options);

            }
            else if(strcmp(sub_command, "execute") == 0){
                //fprintf(stdout, "sub_command=%s.\n", sub_command);
               /* execute format:
                * # execute   <program-name> <version-number> <procedure> <input-file> <output-file>
                */
                if(strcmp(input_options->option1, sciLibrary->program_name) != 0){
                    fprintf(stdout, "Hey, please try to request our flagship program: %s.\n", sciLibrary->program_name);
                    continue;
                }
                else if(strcmp(input_options->option2, "1") != 0 && strcmp(input_options->option2, "2") != 0){
                    fprintf(stdout, "Sorry, the supported version number is 1 or 2.\n");
                    continue;
                }

                int pN;
                int pMatch = 0;
                for(pN=0; pN < sciLibrary->procedure_number; pN++){
                    if(strcmp(input_options->option3, sciLibrary->procedures[pN]) == 0){
                        pMatch = 1;
                    }
                }
                if(pMatch == 0){
                    fprintf(stdout, "Sorry, the procedure %s does not support.\n", input_options->option3);
                    continue;
                }

                if ( access(input_options->option4,F_OK) != 0 && 
                          access(input_options->option4,R_OK) != 0){
                    fprintf(stdout, "Sorry, File %s does not exist or does not have read permission.\n", input_options->option4);
                    continue;
                }
                else if(checkSpecialChar(input_options->option5) == 1){
                    fprintf(stdout, "Sorry, File name %s is invalid.\n", input_options->option5);
                    continue;
                }

                /*request first before execute the service*/
                requestServices(input_options);
                executeServices(input_options);

            }
            else if(strcmp(sub_command, "null") != 0){
                fprintf(stderr, "ERROR: command not found: %s.\n", sub_command);
                helpClient();
            }
            //print prompt by default
            printPrompt = 1;

        }
        else{
            perror("Not all fields are assigned");
            break;
        }

    }

    free(sciLibrary);
    free(input_options);
    free(executeResultSeq);
    free(executePacketSeq);
    return 0;
}
