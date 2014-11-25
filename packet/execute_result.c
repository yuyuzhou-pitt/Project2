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
#include "../lib/libscientific.h"

#include "linkseq.h"

/*for client, receiving the result reply, do not need sockaddr*/
Packet *recvExecuteResult(int sockfd){
    Packet *packet_reply;
    packet_reply = (Packet *)malloc(sizeof(Packet));

    int recvbytes;
    if ((recvbytes = recvfrom(sockfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL, NULL, NULL)) < 0){
        perror("recvfrom_server");
    }

    insertListSeq(executeResultSeq, *packet_reply);
    //appendListSeq(executeResultSeq, *packet_reply);
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Recvfrom message: %s\n", packet->sender_id);
    //logging(LOGFILE, logmsg);
    return packet_reply;
    //return RecvfromServer(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL); 
}   

/*for client, write return result into output file*/
int writeResultFile(char *result_file){
//int writeResultFile(Packet_Seq *exec_result_seq, char *result_file){

    Packet_Seq *r, *t;
    //r = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    //t = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    r = executeResultSeq->next;
    t = executeResultSeq->next;

    FILE *result_fp;
    result_fp = fopen(result_file,"w");

    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data 
    sciLibrary = programLibrary();

    int m, n, l;
    m = n = l = 0;
    int i=0, j;
    //int *a,*b,*c,*d;
    int choice = -1;
    /*set below parameters for once, then decide which procedure to calculate the result*/
    while(r->next!=r){ // in case it's an empty link
        if(choice == -1){

            if(strcmp(r->cur.Data.procedure_name,sciLibrary->procedure1)==0) choice=0; // Multiply
            else if(strcmp(r->cur.Data.procedure_name,sciLibrary->procedure2)==0) choice=1; //Sort
            else if(strcmp(r->cur.Data.procedure_name,sciLibrary->procedure3)==0) choice=2; //Min
            else if(strcmp(r->cur.Data.procedure_name,sciLibrary->procedure4)==0) choice=3; //Max

            //respons_dimension = r->cur.Data.respons_dimension;

            /*for general variables*/
            if(choice == 0){ // Multiply
                if (i == 0) m = r->cur.Data.res_dimen1_len;
                if (n == 0) n = r->cur.Data.res_dimen2_len;

                //c = (int *)malloc(m * n * sizeof(int));
            }
            else{ // Sort, Min, Max
                if (i == 0) n = r->cur.Data.res_dimen1_len;
                //d = (int *)malloc(n * sizeof(int));
            }
        }
        else{
            break;
        }

        r = r->next;
    }

    int d_index;
    /*for Multiply*/
    if(choice == 0){
        d_index = r->cur.Data.respons_dimension;
        fprintf(result_fp, "%d %d\n", m, n);
        /*the first 2 dimension array*/
        for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                if(t->next!=t){
                    //c[n*i+j] = t->cur.Data.res_data.data_int[d_index];
                    fprintf(result_fp,"%d ",t->cur.Data.res_data.data_int[d_index]);
                    if(d_index < INTMTU-1){
                        d_index++; //d_index++ <= INTMTU
                    }
                    else{
                        t = t->next;
                        d_index = 0; //reset data index
                    }
                }
            }
            fprintf(result_fp,"\n");
        }
    }
    else if(choice == 1 || choice == 2 || choice == 3){ // for Sort, Max, Min
        d_index = r->cur.Data.respons_dimension; // skip first 1 dimension value for the first packet
        fprintf(result_fp, "%d\n", n);
        for(i=0;i<n;i++){
            if(t->next!=t){
                //d[i] = t->cur.Data.res_data.data_int[d_index];
                fprintf(result_fp,"%d ",t->cur.Data.res_data.data_int[d_index]);
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

    free(sciLibrary);
    fclose(result_fp);
    return 0;
}   
