/* minigoogle the comandline version of SRPC Client, it plays two roles:
 *  1. Request client info from SRPC port mapper for certain service
 *  2. Call procedure from SRPC client
 *
 * Steps to build and run:
 * $ make
 * $ ./minigoogle request <function-name>
 *    Desc: Request client info from port mapper for certain service.
 * $ ./minigoogle execute <function_name> <input_file> <output_file>
 *    Desc: Allow users to select which remote procedure call they want to make.
 * $ ./minigoogle help
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

int main(int argc, char *argv[]){

    if(argc < 2 || argc > 7){
    //if(argc < 2 || argc > 7 || strcmp(argv[0], "minigoogle") != 0 || strcmp(argv[0], "./minigoogle") != 0){
        helpMiniGoogle();
        exit(1);
    }

    /*initial the global seq for execute client thread*/
    initExecuteSeq();
    /*initial the load balance link for client request service use*/
    initLoadSeq();

    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data
    sciLibrary = programLibrary();

    OptionsStruct *input_options;
    //input_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));

    if(strcmp(argv[1], "help") == 0){
        helpMiniGoogle();
    }
    else if(strcmp(argv[1], "request") == 0){
        /* request format:
         * $ ./minigoogle request   <program-name> <version-number> <procedure>
         */

        if(argc != 5){
            helpMiniGoogle();
            exit(1);
        }

        input_options = argv2struct(argc, argv);

        if(strcmp(input_options->option1, sciLibrary->program_name) != 0){
        printf("input_options->option1=%s.\n", input_options->option1);
        printf("sciLibrary->program_name=%s.\n", sciLibrary->program_name);
            fprintf(stdout, "Hey, please try to request our flagship program: %s.\n", sciLibrary->program_name);
        }
        else if(strcmp(input_options->option2, "1") != 0 && strcmp(input_options->option2, "2") != 0){
            fprintf(stdout, "Sorry, the supported version number is 1 or 2.\n");
        }
        else if(strcmp(input_options->option3, sciLibrary->procedure1) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure2) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure3) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure4) != 0){
            fprintf(stdout, "Sorry, the supported procedure are %s, %s, %s, and %s.\n", sciLibrary->procedure1,
                    sciLibrary->procedure2, sciLibrary->procedure2, sciLibrary->procedure3);
        }
        else{
            requestServices(input_options);
        }
    }
    else if(strcmp(argv[1], "execute") == 0){
       /* execute format:
        * $ ./minigoogle execute   <program-name> <version-number> <procedure> <input-file> <output-file>
        */
        if(argc != 7){
            helpMiniGoogle();
            exit(1);
        }

        input_options = argv2struct(argc, argv);

        if(strcmp(input_options->option1, sciLibrary->program_name) != 0){
        printf("input_options->option1=%s.\n", input_options->option1);
        printf("sciLibrary->program_name=%s.\n", sciLibrary->program_name);
            fprintf(stdout, "Hey, please try to request our flagship program: %s.\n", sciLibrary->program_name);
        }
        else if(strcmp(input_options->option2, "1") != 0 && strcmp(input_options->option2, "2") != 0){
            fprintf(stdout, "Sorry, the supported version number is 1 or 2.\n");
        }
        else if(strcmp(input_options->option3, sciLibrary->procedure1) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure2) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure3) != 0 &&
                strcmp(input_options->option3, sciLibrary->procedure4) != 0){
            fprintf(stdout, "Sorry, the supported procedure are %s, %s, %s, and %s.\n", sciLibrary->procedure1,
                    sciLibrary->procedure2, sciLibrary->procedure2, sciLibrary->procedure3);
        }
        else if ( access(input_options->option4,F_OK) != 0 &&
                  access(input_options->option4,R_OK) != 0){
            fprintf(stdout, "Sorry, File %s does not exist or does not have read permission.\n", input_options->option4);
        }
        /*the result will write into the output file, which will be created if not exists*/
        else if(checkSpecialChar(input_options->option5) == 1){
            fprintf(stdout, "Sorry, File name %s is invalid.\n", input_options->option5);
        }
        else{
            /*request first before execute the service*/
            requestServices(input_options);
            executeServices(input_options);
        }
    }
    else{
        helpMiniGoogle();
    }

    //sleep(5);
    free(sciLibrary);
    free(executeResultSeq);
    free(executePacketSeq);
    return 0;
}
