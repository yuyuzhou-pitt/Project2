#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define EXECUTE_SERV 1
#include "packet.h"
#include "linkseq.h"
#include "../lib/libscientific.h"

/*list initialization*/
int *initExecuteSeq(){
    
    executePacketSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    executePacketEnd = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    executeResultSeq = (Packet_Seq *)malloc(sizeof(Packet_Seq));
    executeResultEnd = (Packet_Seq *)malloc(sizeof(Packet_Seq));

    executePacketSeq->next = executePacketEnd;
    executePacketEnd->next = executePacketEnd;
    executePacketEnd->prev = executePacketSeq;
    executePacketSeq->prev = executePacketSeq;

    executeResultSeq->next = executeResultEnd;
    executeResultEnd->next = executeResultEnd;
    executeResultEnd->prev = executeResultSeq;
    executeResultSeq->prev = executeResultSeq;

    return 0;
}

/*list initialization*/
Packet_Seq *initListSeq(){
    Packet_Seq *head, *z;
    head = (Packet_Seq *)malloc(sizeof *head); //head->next->key is the first item
    z = (Packet_Seq *)malloc(sizeof *head); //dummy rear, to protect dequeue from an empty list

    head->next = z;
    z->next = z; 

    z->prev = head;
    head->prev = head;

    return head;
}

/*enqueue a node Packet to the end (between head and rear)*/
Packet_Seq *appendListSeq(Packet_Seq *head, Packet packet){
    Packet_Seq *t, *r, *z;
    r = head;
    z = head->next; // to find the rear node
    while(z->next!=z){
        r = r->next; // to find the node before rear node
        z = r->next;
    }

    t = (Packet_Seq *)malloc(sizeof *t);

    t->cur = packet;

    t->next = z;
    r->next = t;

    t->prev = r;
    z->prev = t;

    return head;
}

/*insert a node Packet according to the sequence number
 * 1. find the first node which has the bigger sequence number
 * 2. insert the node into the front the the found node*/
Packet_Seq *insertListSeq(Packet_Seq *head, Packet packet){
    Packet_Seq *t, *p, *r, *z;
    int Max_flag = 1;
    r = head; // first node is empty 
    z = head->next; // to find the rear node
    while(z->next!=z){
        r = r->next; // to find the node before rear node
        z = z->next;

        if (r->cur.Data.transaction_id == packet.Data.transaction_id && 
            r->cur.Data.seq == packet.Data.seq){
           //redundant packet
           Max_flag = 0; 
           break;
        }
        else if(r->cur.Data.transaction_id == packet.Data.transaction_id &&
                r->cur.Data.seq > packet.Data.seq){
            p = r->prev;
            t = (Packet_Seq *)malloc(sizeof *t);
        
            t->cur = packet;
        
            t->next = r;
            p->next = t;
        
            t->prev = p;
            r->prev = t;

            Max_flag = 0;
            break;
        }

    }

    if (Max_flag == 1){ // new packet has biggest sequence number. 
        t = (Packet_Seq *)malloc(sizeof *t);

        t->cur = packet;

        t->next = z;
        r->next = t;

        t->prev = r;
        z->prev = t;
    }

    return head;
}

Packet_Seq *findLastBlank(Packet_Seq *head)
{
    Packet_Seq *r, *z;
    r = head->next; // first node is empty 
    z = head->next; // to find the rear node

    if (r->cur.Data.seq != 0)
        return head; // fist _packet is missing.
    while(z->next!=z){
        z = r->next;
        if (r->cur.Data.seq != (z->cur.Data.seq - 1))
            return r;
        else r = r->next;
    }
    return r;
}

/*return 1 if:
 * 1. same transcation_id
 * 2. seq start from 0
 * 3. seq is in sequence
 * 4. end_flag found */
int isEndTransaction(Packet_Seq *head, int trans_id){
    Packet_Seq *r, *z;
    r = head; // first node is empty 
    z = head->next; // to find the rear node

    int isFirstNode = 0;
    int pktSeq = 0;
    while(z->next!=z){
        r = r->next;
        z = z->next;

        if(r->cur.Data.transaction_id == trans_id &&
           r->cur.Data.seq == pktSeq &&
           r->cur.Data.end_flag == 1){
            return 1;
        }
        if(isFirstNode == 0 &&
           r->cur.Data.transaction_id == trans_id){
            isFirstNode = 1;
            if(r->cur.Data.seq != 0){
                return 0;
            }
            pktSeq++;
        }
        else if(r->cur.Data.transaction_id == trans_id){
            if(r->cur.Data.seq != pktSeq){
                return 0;
            }
            pktSeq++;
        }
    }
    return 0;
}

/*for client, write return result into output file*/
int writeResultSeq(Packet_Seq *head, char *result_file){

    Packet_Seq *r, *t;

    r = head->next;
    t = head->next;

    FILE *result_fp;
    result_fp = fopen(result_file,"w");

    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct *)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data 
    sciLibrary = (*libraryPtr)();

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

            /*for general variables*/
            if(choice == 0){ // Multiply
                if (i == 0) m = r->cur.Data.para1_dimen1_len;
                if (n == 0) n = r->cur.Data.para1_dimen2_len;

            }
            else{ // Sort, Min, Max
                if (i == 0) n = r->cur.Data.para1_dimen1_len;
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
        d_index = r->cur.Data.para1_dimension;
        fprintf(result_fp, "%d %d\n", m, n);
        /*the first 2 dimension array*/
        for(i=0;i<m;i++){
            for(j=0;j<n;j++){
                if(t->next!=t){
                    fprintf(result_fp,"%d ",t->cur.Data.para_data.data_int[d_index]);
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
        d_index = r->cur.Data.para1_dimension; // skip first 1 dimension value for the first packet
        fprintf(result_fp, "%d\n", n);
        for(i=0;i<n;i++){
            if(t->next!=t){
                fprintf(result_fp,"%d ",t->cur.Data.para_data.data_int[d_index]);
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

int clearTransaction(Packet_Seq *head, int trans_id){
    Packet_Seq *r, *p, *z;
    r = head;
    z = head->next;
    while(z->next != z){
        r = r->next;
        z = z->next;
        if(r->cur.Data.transaction_id == trans_id){
            p = r->prev;
            p->next = r->next;
            z->prev = p;
        }
    }
    return 0;
}
