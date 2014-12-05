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

RemoteProgram *(*libraryPtr)();

int main(int argc, char *argv[]){

    if(argc < 2 || argc > 8){
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

        //RemoteProgram *getLibraryPtr(char *program_name, char *version_number);
        libraryPtr = getLibraryPtr(input_options->option1, input_options->option2); // configurable library function

        if(libraryPtr == NULL){
            fprintf(stderr, "Wrong program name (%s) or version number (%s).\n",
                    input_options->option1, input_options->option2);
            helpMiniGoogle();
            exit(1);
        }

        /*get library detail when program name and version number are correct.*/
        sciLibrary = (*libraryPtr)();

        int pN;
        int pMatch = 0;
        for(pN=0; pN < sciLibrary->procedure_number; pN++){
            if(strcmp(input_options->option3, sciLibrary->procedures[pN]) == 0){
                pMatch = 1;
            }
        }
        if(pMatch == 0){
            fprintf(stderr, "Sorry, the procedure %s does not support.\n", input_options->option3);
            fprintf(stderr, "Please use one of the procedures: ");
            int pN;
            for(pN=0;pN < (sciLibrary->procedure_number)-1; pN++){
                fprintf(stderr, "%s, ", sciLibrary->procedures[pN]);
            }
            fprintf(stderr, "%s.\n", sciLibrary->procedures[(sciLibrary->procedure_number)-1]);
            exit(1);
        }

        /*proceed if no problem*/
        requestServices(input_options);
    }
    else if(strcmp(argv[1], "execute") == 0){
       /* execute format:
        * fprintf(stderr, "$ ./minigoogle execute MapReduceLibrary 1 Index ../input ../output\n");
        * fprintf(stderr, "$ ./minigoogle execute MapReduceLibrary 1 Search ../input ../output \"item1 item2\"");
        *
        * the argc number could be 7 or 8.
        */
        if(argc < 7 || argc > 8){
            fprintf(stderr, "Wrong sytax.\n");
            helpMiniGoogle();
            exit(1);
        }

        input_options = argv2struct(argc, argv);

        //RemoteProgram *getLibraryPtr(char *program_name, char *version_number);
        libraryPtr = getLibraryPtr(input_options->option1, input_options->option2); // configurable library function

        if(libraryPtr == NULL){
            fprintf(stderr, "Wrong program name (%s) or version number (%s).\n",
                    input_options->option1, input_options->option2);
            helpMiniGoogle();
            exit(1);
        }

        /*get library detail when program name and version number are correct.*/
        sciLibrary = (*libraryPtr)();

        int pN;
        int pMatch = 0;
        for(pN=0; pN < sciLibrary->procedure_number; pN++){
            if(strcmp(input_options->option3, sciLibrary->procedures[pN]) == 0){
                pMatch = 1;
            }
        }
        if(pMatch == 0){
            fprintf(stderr, "Sorry, the procedure (%s) does not support.\n", input_options->option3);
            fprintf(stderr, "Please use one of the procedures: ");
            int pN;
            for(pN=0;pN < (sciLibrary->procedure_number)-1; pN++){
                fprintf(stderr, "%s, ", sciLibrary->procedures[pN]);
            }
            fprintf(stderr, "%s.\n", sciLibrary->procedures[(sciLibrary->procedure_number)-1]);
            exit(1);
        }

        if (access(input_options->option4,F_OK) != 0 &&
                  access(input_options->option4,R_OK) != 0){
            fprintf(stderr, "Sorry, Source file or directory %s does not exist or does not have read permission.\n", input_options->option4);
            exit(1);
        }
        if(checkSpecialChar(input_options->option5) == 1){
            fprintf(stderr, "Sorry, Target file or directory %s is invalid.\n", input_options->option5);
            exit(1);
        }
        else if(access(input_options->option5,F_OK) == 0 ){
            fprintf(stderr, "Sorry, Target file or directory %s exists, please use a new one.\n", input_options->option5);
            exit(1);
        }

        /*request first before execute the service*/
        requestServices(input_options);
        executeServices(input_options); // execute Map Reduce services, for minigoogle
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
