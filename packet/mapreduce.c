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
    snprintf(result_options->option4, sizeof(result_options->option4), "../.%s", received_packet->Data.exec_action);

    struct stat st = {0};
    if (stat(result_options->option4, &st) == -1) {
        mkdir(result_options->option4, 0700);
    }

    if(strcmp(received_packet->Data.exec_action, SPLIT) == 0){
        fprintf(stdout, "Spliting file %s.\n", received_packet->Data.para_data.data_str);
        snprintf(logmsg, sizeof(logmsg), "Hi, I got file %s. I will do splitting...\n", 
                 received_packet->Data.para_data.data_str);
        logging(LOGFILE, logmsg);

        /*all the split file will be store in the directory .Split */
        Split(received_packet->Data.para_data.data_str, result_options->option4);
    }
    else if(strcmp(received_packet->Data.exec_action, WORDCOUNT) == 0){
        fprintf(stdout, "Wordcounting file %s.\n", received_packet->Data.para_data.data_str);
        snprintf(logmsg, sizeof(logmsg), "Hi, I got file %s. I will do wordcounting...\n", 
                 received_packet->Data.para_data.data_str);
        logging(LOGFILE, logmsg);
    }
    else if(strcmp(received_packet->Data.exec_action, SORT) == 0){
        snprintf(logmsg, sizeof(logmsg), "Hi, I got file %s. I will do sorting...\n", 
                 received_packet->Data.para_data.data_str);
        logging(LOGFILE, logmsg);
    }
    else if(strcmp(received_packet->Data.exec_action, REDUCE) == 0){
        snprintf(logmsg, sizeof(logmsg), "Hi, I got file %s. I will do reducing...\n", 
                 received_packet->Data.para_data.data_str);
        logging(LOGFILE, logmsg);
    }
    else if(strcmp(received_packet->Data.exec_action, SEARCH) == 0){
        snprintf(logmsg, sizeof(logmsg), "Hi, I will do searching...\n");
        logging(LOGFILE, logmsg);
    }

    return 0;
}

/* Use below file fucntion to split file
 * int writeFile(char *str, int size, char *file, char *writeMode);
 * int readFile(char *str, int size, char *file);
*/

int Split(char *file, char *target_dir){
    char split_str[SPLIT_BLOCK];

    FILE *split_fp;

    if(access(file, F_OK) < 0) {
        snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if ((split_fp = fopen(file,"r")) < 0){
        snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    struct timeval t_stamp;
    char target_file[1024];
    int count;
    while((count = fread(split_str, 1, SPLIT_BLOCK, split_fp)) > 0){
        int f_index = count;
        while(count == SPLIT_BLOCK && (split_str[f_index] != ' ' && split_str[f_index] != '\n')){
            f_index--;
        }
        fseek(split_fp, f_index-count, SEEK_CUR);
        t_stamp = getUTimeStamp();
        /*please use .txt to be the file extension (checking in client/socket_execute.c:jobTracker())*/
        snprintf(target_file, sizeof(target_file), "%s/%s___%d.%d.txt", target_dir, 
                 getStrAfterDelimiter(file, '/'), t_stamp.tv_sec, t_stamp.tv_usec);
        snprintf(logmsg, sizeof(logmsg), "Split: write into part file: %s\n", target_file);logging(LOGFILE, logmsg);
        writeFile(split_str, f_index, target_file, "w");
    }

    fclose(split_fp);

    return 0;
}
