#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define EXECUTE_SERV 1
#include "packet.h"

#include "checksum.h"
#include "../lib/liblog.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"
#include "../lib/libmath.h"
#include "../lib/libsocket.h"
#include "../lib/libfile.h"
//#include "../lib/libmapreduce.h"
#include "../lib/libmr.h"

#include "linkseq.h"

char logmsg[128];

/*the function to call MapReduce*/
int callMapReduce(OptionsStruct *result_options, char *client_ip, char *server_ip, Packet *received_packet){
    snprintf(client_ip, sizeof(received_packet->sender_id), "%s", received_packet->sender_id);
    snprintf(server_ip, sizeof(received_packet->receiver_id), "%s", received_packet->receiver_id);

    snprintf(result_options->option1, sizeof(result_options->option1), "%s", received_packet->Data.program_name);
    snprintf(result_options->option2, sizeof(result_options->option2), "%s", received_packet->Data.version_number);
    snprintf(result_options->option3, sizeof(result_options->option3), "%s", received_packet->Data.procedure_name);
    /*exec_reply_pkt->Data.res_data.data_str*/
    snprintf(result_options->option4, sizeof(result_options->option4), "../.%s_%s", client_ip, received_packet->Data.exec_action);

    struct stat st = {0};
    if (stat(result_options->option4, &st) == -1) {
        mkdir(result_options->option4, 0700);
    }

    /*index action*/
    if(strcmp(received_packet->Data.exec_action, SPLIT) == 0){
        fprintf(stdout, "Spliting file %s.\n", received_packet->Data.para_data.data_str);

        /*all the split file will be store in the directory .Split */
        Split(received_packet->Data.para_data.data_str, result_options->option4);
    }
    else if(strcmp(received_packet->Data.exec_action, WORDCOUNT) == 0){
        fprintf(stdout, "Wordcounting file %s.\n", received_packet->Data.para_data.data_str);

        /*all the wordcount file will be store in the directory .Wordcount*/
        Wordcount(received_packet->Data.para_data.data_str, result_options->option4);
    }
    else if(strcmp(received_packet->Data.exec_action, SORT) == 0){
        fprintf(stdout, "Sorting directory %s on server %d of %d.\n", 
                received_packet->Data.para_data.data_str, (received_packet->Data.server_no)+1, 
                received_packet->Data.server_number);

        /*all the wordcount file will be store in the directory .Wordcount*/
        WordSort(received_packet->Data.para_data.data_str, result_options->option4, 
             received_packet->Data.server_number, received_packet->Data.server_no);
    }
    else if(strcmp(received_packet->Data.exec_action, MII) == 0){
        fprintf(stdout, "Sorting file %s.\n", received_packet->Data.para_data.data_str);

        /*all the reduced file will be store in the output directory*/
        Reduce(received_packet->Data.para_data.data_str, received_packet->Data.output_dir);
    }
    /*search action*/
    else if(strcmp(received_packet->Data.exec_action, SINGLE) == 0){
        fprintf(stdout, "Searching term \"%s\" in file %s.\n",  received_packet->Data.search_term, 
                received_packet->Data.para_data.data_str);

        /*all the search result file will be store in the temp directory .Single*/
        Search(received_packet->Data.para_data.data_str, received_packet->Data.search_term, result_options->option4);
    }
    else if(strcmp(received_packet->Data.exec_action, MERGE) == 0){
        fprintf(stdout, "Searching term \"%s\" in file %s.\n",  received_packet->Data.search_term, 
                received_packet->Data.para_data.data_str);
    }

    return 0;
}

