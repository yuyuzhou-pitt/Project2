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


//Packet_Seq *executePacketSeq; //global seq defined in linkseq.h

/*
union ParameterData{ //MTU = 512
      int data_int[256]; 
      float data_float [128];
      char str[512];
};

typedef struct Execute_Sevice{
    char version_number[8];
    char program_name[30];
    char procedure_name[30];
    int transaction_id; // the packet in the same seq share the same transaction_id, randomly assigned
    int num_parameter;
    int end_flag; // wheather this packet is the last one for one transaction. 
    int para1_type;
    int para1_dimension;
    int para1_dimen1_len;
    int para1_dimen2_len;
    int para2_type;
    int para2_dimension;
    int para2_dimen1_len;
    int para2_dimen2_len;
    union ParameterData para_data;
    int seq; //sequence number, start from 0 
}Execute_Serv;

typedef struct Packet{
    char sender_id[32];
    char receiver_id[32];
    //char netmask[32]; 
    char packet_type[3];
    Register_Serv Data;
    char PacketChecksum[32]; // crc32
}Packet;

typedef struct Packet_Seq{
    Packet *cur;
    struct Packet_Seq *prev;
    struct Packet_Seq *next;
}Packet_Seq;

Execute command format:
# execute   <program-name> <version-number> <procedure> <input-file> <output-file>
*/

Packet_Seq *genExecuteServ(OptionsStruct *options, char *client_ip, char *server_ip, char *trans_id){

    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data
    sciLibrary = (*libraryPtr)();

    Packet execPkt;
    //execPkt = (Packet *)malloc(sizeof(Packet));
    snprintf(execPkt.sender_id, sizeof(execPkt.sender_id), "%s", client_ip);
    snprintf(execPkt.receiver_id, sizeof(execPkt.receiver_id), "%s", server_ip);
    snprintf(execPkt.packet_type, sizeof(execPkt.packet_type), "%s", "100"); // execute service pakcet type

    snprintf(execPkt.Data.program_name, sizeof(execPkt.Data.program_name), "%s", options->option1);
    snprintf(execPkt.Data.version_number, sizeof(execPkt.Data.version_number), "%s", options->option2);
    snprintf(execPkt.Data.procedure_name, sizeof(execPkt.Data.procedure_name), "%s", options->option3);

    srand(time(NULL));
    execPkt.Data.transaction_id = rand(); // share the same transaction_id in the seq
    /*store the transcation id, in case there are multiple secquence in the link*/
    snprintf(trans_id, 32, "%d", execPkt.Data.transaction_id);
    execPkt.Data.num_parameter = 0; //Multiply has up to 2 parameters
    execPkt.Data.end_flag = 0; // default is 0
    execPkt.Data.para1_type = 0; // default is int (0)
    execPkt.Data.para2_type = 0; // default is int (0)

    int lineIndex = 0;
    int arrayIndex = 0; 
    int nPerLine = 0; // the number of integers per line
    int pktSeq = 0; // start from 0

    FILE *in_fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int fcerr = 0;

    if ((in_fp = fopen(options->option4,"r")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", options->option4);
        logging(LOGFILE, logmsg);
        return executePacketSeq;
    }

    if (strcmp(options->option3, sciLibrary->procedure1) == 0 || //Multiply
        /* example [3,4]*[4,2]
        * 3 4
        * 3 6 7 5 
        * 3 5 6 2 
        * 9 1 2 7 
        * 4 2
        * 0 9 
        * 3 6 
        * 0 6 
        * 2 6 
        */
        strcmp(options->option3, sciLibrary->procedure2) == 0 || //Sort
        strcmp(options->option3, sciLibrary->procedure3) == 0 || //Min
        strcmp(options->option3, sciLibrary->procedure4) == 0) { //Max
        /*example
        * 20
        * 83 86 77 15 93 35 86 92 49 21 62 27 90 59 63 26 40 26 72 36
        */ 

        int isEOF = 0; //read the end of the file
        int start_n = 0; //prevent data_int over flow, control the convert size in str2IntArray
        int data_int_full = 0; //packet data_int is full, ready to enqueue
        char line_no_n[10240]; //the length per line
        while ((read = getline(&line, &len, in_fp)) != -1 || isEOF == 0) {
            snprintf(line_no_n, strlen(line), "%s", line);
            //printf("%s", line);
            /*provide the chance to create packet if is end of file*/
            if(read == -1){
                execPkt.Data.end_flag = 1;
                isEOF = 1; 
            }

            /* add split integers into temp int array*/
            //printf("start_n=%d ", start_n);
            nPerLine = str2IntArray(execPkt.Data.para_data.data_int, INTMTU, arrayIndex, line_no_n, start_n);
            //printf("arrarIndex+nPerLine=%d+%d.\n", arrayIndex, nPerLine);


            /*read the parameter 1 dimension length*/
            if(lineIndex == 0){
                execPkt.Data.para1_dimension = nPerLine;
                execPkt.Data.para1_dimen1_len = execPkt.Data.para_data.data_int[arrayIndex]; // 0 here
                if(nPerLine == 2){
                    execPkt.Data.para1_dimen2_len = execPkt.Data.para_data.data_int[arrayIndex+1]; // 1 here
                }
                execPkt.Data.num_parameter++;
            }
            /*read the parameter 2 dimension length*/
            if(lineIndex == execPkt.Data.para1_dimen1_len + 1){
                execPkt.Data.para2_dimension = nPerLine;
                execPkt.Data.para2_dimen1_len = execPkt.Data.para_data.data_int[arrayIndex];
                if(nPerLine == 2){
                    execPkt.Data.para2_dimen2_len = execPkt.Data.para_data.data_int[arrayIndex+1];
                }
                execPkt.Data.num_parameter++;
            }

            /*para_data is full in the middle of line, read the line again for next packet*/
            if(arrayIndex + nPerLine > INTMTU){
                start_n = INTMTU - arrayIndex; // next start index for line
                arrayIndex = 0; // start from begining for data_int
                data_int_full = 1;
                if(in_fp){
                    fseek(in_fp, -read-1, SEEK_CUR); // seek backward 1 line from the current pos
                }
            }
            /*para_data is full in the end of line*/
            else if(arrayIndex + nPerLine == INTMTU){
                if (nPerLine != 0) {
                   start_n = 0; // reset line start index
                   lineIndex++;
                }
                arrayIndex = 0; // start from begining for data_int
                data_int_full = 1;
            }
            /*para_data is not full*/
            else{
                if (nPerLine != 0){ 
                    arrayIndex = arrayIndex + nPerLine - start_n;
                    start_n = 0; // reset line start index
                    lineIndex++;
                }
                data_int_full = 0; //reset full flag
            }

            /* create packet if:
             * a. is end of file, or
             * b. the execPkt.Data.para_data.data_int is full */
            if(isEOF == 1 || data_int_full == 1){
                /*DEBUG: print packet data_int*/
                /*int kkk;
                for(kkk=0;kkk<INTMTU;kkk++){
                    printf("%d ", execPkt.Data.para_data.data_int[kkk]);
                }
                */

                execPkt.Data.seq = pktSeq ++; 
                /*checksum*/
                snprintf(execPkt.PacketChecksum, sizeof(execPkt.PacketChecksum), 
                         "%d", chksum_crc32((unsigned char*) &execPkt, sizeof(execPkt)));

                /*enqueue the packet*/
                appendListSeq(executePacketSeq, execPkt);

                //free(execPkt.Data.para_data.data_int);
            }
        }

    }

    if(in_fp){
        if( (fcerr = fclose(in_fp)) != 0 ){
            char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "closefile: Failed to close file: %s. Error <%d>.\n", options->option4, fcerr);
            logging(LOGFILE, logmsg);
        }
    }

    free(sciLibrary);
    return executePacketSeq;
}

int calcResult(OptionsStruct *result_options, char *client_ip, char *server_ip, Packet_Seq *recv_exec_seq){
    char result_file[32];
    memset(result_file, 0, sizeof(result_file));

    strcpy(result_file, "../");
    strcat(result_file, EXECUTE_RESULT_FILE);

    FILE *result_fp;
    result_fp = fopen(result_file,"wb");

    int choice = -1;

    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data 
    sciLibrary = (*libraryPtr)();

    Packet_Seq *t, *r;
    t = recv_exec_seq;
    r = recv_exec_seq->next;

    int num_parameter;
    int para1_dimension;
    int para2_dimension;


    int m, n, l;
    m = n = l = 0;
    int i=0, j;
    int *a,*b,*c,*d;
    /*set below parameters for once, then decide which procedure to calculate the result*/
    // use the last node, in case 
    // 1. it's an empty link
    // 2. the para2_dimen2_len is not set yet
    while(r->next!=r){
        t = t->next;
        r = r->next;
    }

    if(choice == -1){
        snprintf(client_ip, sizeof(r->cur.sender_id), "%s", t->cur.sender_id);
        snprintf(server_ip, sizeof(r->cur.receiver_id), "%s", t->cur.receiver_id);

        snprintf(result_options->option1, sizeof(result_options->option1), "%s", t->cur.Data.program_name);
        snprintf(result_options->option2, sizeof(result_options->option2), "%s", t->cur.Data.version_number);
        snprintf(result_options->option3, sizeof(result_options->option3), "%s", t->cur.Data.procedure_name);

        if(strcmp(t->cur.Data.procedure_name,sciLibrary->procedure1)==0) choice=0; // Multiply
        else if(strcmp(t->cur.Data.procedure_name,sciLibrary->procedure2)==0) choice=1; //Sort
        else if(strcmp(t->cur.Data.procedure_name,sciLibrary->procedure3)==0) choice=2; //Min
        else if(strcmp(t->cur.Data.procedure_name,sciLibrary->procedure4)==0) choice=3; //Max

        num_parameter = t->cur.Data.num_parameter;
        para1_dimension = t->cur.Data.para1_dimension;
        para2_dimension = t->cur.Data.para2_dimension;

        /*for general variables*/
        if(choice == 0){ // Multiply
            if (i == 0) m = t->cur.Data.para1_dimen1_len;
            if (n == 0) n = t->cur.Data.para1_dimen2_len;
            if (l == 0) l = t->cur.Data.para2_dimen2_len;

            a = (int *)malloc(m * n * sizeof(int));
            b = (int *)malloc(n * l * sizeof(int));
        }
        else{ // Sort, Min, Max
            if (i == 0) n = t->cur.Data.para1_dimen1_len;
            d = (int *)malloc(n * sizeof(int));
        }
    }

    /*read data from packet*/
    //Packet_Seq *t;
    //reset t before using
    t = recv_exec_seq->next;

    int d_index;
    /*for Multiply*/
    if(choice == 0){
        d_index = para1_dimension; // skip first two dimension value for the first packet
        /*the first 2 dimension array*/
        for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                if(t->next!=t){
                    //a+n*i+j = &(t->cur.Data.para_data.data_int[d_index]);
                    a[n*i+j] = t->cur.Data.para_data.data_int[d_index];
                    if(d_index < INTMTU-1){ 
                        d_index++; //d_index++ <= INTMTU
                    }
                    else{
                        t = t->next;
                        d_index = 0; //reset data index
                    }
                }
            }
        }
        //printf("==first para ==");

        /*DEBUG: print a*/
        /*for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                //fprintf(stdout, "%d ", a[n*i+j]);
                fprintf(stdout, "%d ", *(a+n*i+j));
            }
        }*/

        /*set the second 2 dimension array*/
        d_index = (d_index + para2_dimension) % INTMTU; // skip second two dimension value, mode in case it bigger than INTMTU
        if(d_index + para2_dimension >= INTMTU){
            t = t->next;
        }

        for(i=0;i<n;i++){
            for(j=0;j<l;j++){
                if(t->next!=t){
                    b[l*i+j] = t->cur.Data.para_data.data_int[d_index];
                    //b+l*i+j = &(t->cur.Data.para_data.data_int[d_index]);

                    if(d_index < INTMTU-1){
                        d_index++;
                    }
                    else{
                        t = t->next;
                        d_index = 0; //reset data index
                    }
                }
            }
        }

        //printf("==second para ==");

        /*DEBUG: print b*/
        /*for(i=0;i<n;i++){
            for(j=0;j<l;j++){
                printf("%d,", b[l*i+j]);
            }
        }
        printf("==para end==");
        */

    }
    /*for Sort, Min, Max*/
    else{
        d_index = para1_dimension; // skip first two dimension value for the first packet
        for(i=0;i<n;i++){
            if(t->next!=t){
                d[i] = t->cur.Data.para_data.data_int[d_index];

                if(d_index < INTMTU-1){
                    d_index++;
                }
                else{
                    t = t->next;
                    d_index = 0; //reset data index
                }
            }
        }
        /*DEBUG: print a*/
        for(i=0;i<n;i++){
            printf("%d ", d[i]);
        }

    }

    /*write calculating result into out file*/
    switch(choice){
        case 0: // matrix multiply
            c = (int *)malloc(m * l * sizeof(int));
            memset(c, 0, sizeof(*c));
            c = Multiply(a,b,c,m,n,l);
            fprintf(result_fp, "%d %d\n", m, l);
            printmatrix(result_fp,c,m,l);
            //fwrite(c, sizeof(int), m*l, result_fp);
            free(c);
            break;
        case 1: // sort array
            Sort(d, n);
            printarray(result_fp, d, n);
            break;
        case 2: // min
            fprintf(result_fp, "%d", Min(d, n));
            break;
        case 3: // max
            fprintf(result_fp, "%d", Max(d, n));
            break;
        default:
            break;
    } //endof switch

    if(choice == 0){ // Multiply
        free(a);
        free(b);
    }
    else{
        free(d);
    }
    free(sciLibrary);
    fclose(result_fp);
    return 0;
}

/*send the seq out*/
int sendExecuteServ(int sockfd, struct sockaddr_in sockaddr, Packet *packet){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL, (struct sockaddr *)&sockaddr, sizeof(sockaddr))) < 0){
        perror("sendto");
        return -1;
    }

    return sendbytes;

    //return Sendto(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL, sockaddr, sizeof(sockaddr));
}

int recvExecuteServ(int sockfd, struct sockaddr *sockaddr, Packet *packet){
    return Recvfrom(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL, sockaddr, sizeof(sockaddr));
}
