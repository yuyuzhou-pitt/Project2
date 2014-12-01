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
#include <time.h>
#include <dirent.h>

#define EXECUTE_SERV 1
#include "packet.h"

#include "checksum.h"
#include "../lib/liblog.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"
#include "../lib/libmath.h"
#include "../lib/libsocket.h"
#include "../lib/libfile.h"

#include "linkseq.h"

/*the function to call MapReduce*/
int callMapReduce(OptionsStruct *result_options, char *client_ip, char *server_ip, Packet *received_packet){
    snprintf(client_ip, sizeof(received_packet->sender_id), "%s", received_packet->sender_id);
    snprintf(server_ip, sizeof(received_packet->receiver_id), "%s", received_packet->receiver_id);

    snprintf(result_options->option1, sizeof(result_options->option1), "%s", received_packet->Data.program_name);
    snprintf(result_options->option2, sizeof(result_options->option2), "%s", received_packet->Data.version_number);
    snprintf(result_options->option3, sizeof(result_options->option3), "%s", received_packet->Data.procedure_name);
    snprintf(result_options->option4, sizeof(result_options->option4), "%s", received_packet->Data.para_data.data_str);

    if(strcmp(received_packet->Data.exec_action, "Split") == 0){
        sprintf(stdout, "Hi, I will do splitting...\n");
    }
    else if(strcmp(received_packet->Data.exec_action, "Index") == 0){
        sprintf(stdout, "Hi, I will do indexing...\n");
    }
    else if(strcmp(received_packet->Data.exec_action, "Search") == 0){
        sprintf(stdout, "Hi, I will do searching...\n");
    }

    return 0;
}
