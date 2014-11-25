#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define EXECUTE_REPLY 1
#include "packet.h"

#include "checksum.h"
#include "../packet/execute.h"
#include "../packet/execute_reply.h"
#include "../lib/libscientific.h"
#include "../lib/liblog.h"
#include "../lib/libterminal.h"
#include "../lib/libsocket.h"
#include "../lib/libfile.h"
#include "../lib/libmath.h"

#include "linkseq.h"

/*for server, generate execute reply*/
Packet_Seq *genExecuteReply(OptionsStruct *result_options, char *client_ip, char *server_ip, char *trans_id){

    char result_file[32];
    memset(result_file, 0, sizeof(result_file));
    strcpy(result_file, "../");
    strcat(result_file, EXECUTE_RESULT_FILE);

    /*generate result sequence*/
    FILE *out_fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if ((out_fp = fopen(result_file,"r")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", result_file);
        logging(LOGFILE, logmsg);
        //return result_seq;
        return executeResultSeq;
        //return -1;
    }

    Packet execReply;
    //execReply = (Packet *)malloc(sizeof(Packet));
    snprintf(execReply.sender_id, sizeof(execReply.sender_id), "%s", server_ip);
    snprintf(execReply.receiver_id, sizeof(execReply.receiver_id), "%s", client_ip);
    snprintf(execReply.packet_type, sizeof(execReply.packet_type), "%s", "101"); // execute reply type

    snprintf(execReply.Data.program_name, sizeof(execReply.Data.program_name), "%s", result_options->option1);
    snprintf(execReply.Data.version_number, sizeof(execReply.Data.version_number), "%s", result_options->option2);
    snprintf(execReply.Data.procedure_name, sizeof(execReply.Data.procedure_name), "%s", result_options->option3);

    srand(time(NULL));
    execReply.Data.transaction_id = rand(); // share the same transaction_id in the seq

    /*store the transcation id, in case there are multiple secquence in the link*/
    snprintf(trans_id, 32, "%d", execReply.Data.transaction_id);

    execReply.Data.end_flag = 0; // default is 0
    execReply.Data.respons_type = 0; // default is int (0)

    int lineIndex = 0;
    int arrayIndex = 0;
    int nPerLine = 0; // the number of integers per line
    int pktSeq = 0; // start from 0

    int isEOF = 0; //read the end of the file
    int start_n = 0; //prevent data_int over flow, control the convert size in str2IntArray
    int data_int_full = 0; //packet data_int is full, ready to enqueue
    char line_no_n[10240]; //the length per line
    while ((read = getline(&line, &len, out_fp)) != -1 || isEOF == 0) {
        snprintf(line_no_n, strlen(line), "%s", line);
        /*provide the chance to create packet if is end of file*/
        if(read == -1){
            execReply.Data.end_flag = 1;
            isEOF = 1;
        }

        /* add split integers into temp int array*/
        nPerLine = str2IntArray(execReply.Data.res_data.data_int, INTMTU, arrayIndex, line_no_n, start_n);
        if(arrayIndex + nPerLine > INTMTU){
            start_n = INTMTU - arrayIndex; // next start index for line
            arrayIndex = 0; // start from begining for data_int
            data_int_full = 1;
            if(out_fp){
                fseek(out_fp, -read-1, SEEK_CUR); // seek backward 30 bytes from the current pos
            }
        }
        else if(arrayIndex + nPerLine == INTMTU){
            if (nPerLine != 0) start_n = 0; // reset line start index
            arrayIndex = 0; // start from begining for data_int
            data_int_full = 1;
        }
        else{
            if (nPerLine != 0){
                arrayIndex = arrayIndex + nPerLine - start_n;
                start_n = 0; // reset line start index
            }
            data_int_full = 0; //reset full flag
        }

        /* create packet if:
         * a. is end of file, or
         * b. the execReply.Data.res_data.data_int is full */
        if(isEOF == 1 || data_int_full == 1){
            execReply.Data.seq = pktSeq ++;
            /*checksum*/
            snprintf(execReply.PacketChecksum, sizeof(execReply.PacketChecksum),
                     "%d", chksum_crc32((unsigned char*) &execReply, sizeof(execReply)));

            /*enqueue the packet*/
            appendListSeq(executeResultSeq, execReply);
            //appendListSeq(result_seq, execReply);

            /*reset the index, for the next packet*/
            arrayIndex = 0;
        }

        /*read the parameter 1 dimension length*/
        if(lineIndex == 0){
            execReply.Data.respons_dimension = nPerLine;
            execReply.Data.res_dimen1_len = execReply.Data.res_data.data_int[arrayIndex-2];
            execReply.Data.res_dimen2_len = execReply.Data.res_data.data_int[arrayIndex-1];
        }
        lineIndex++;
    }

    unlinkFile(result_file);

    //return 0;
    int fcerr;
    if(out_fp){
        if( (fcerr = fclose(out_fp)) != 0 ){
            char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "closefile: Failed to close file: %s. Error <%d>.\n", result_file, fcerr);
            logging(LOGFILE, logmsg);
        }
    }

    return executeResultSeq;
    //return result_seq;
}

/*for server, send execute reply*/
int sendExecuteReply(int sockfd, struct sockaddr_in sockaddr, Packet *packet){
   return Sendto(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL, sockaddr, sizeof(sockaddr));
}
