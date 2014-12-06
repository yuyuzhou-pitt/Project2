#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "../lib/libterminal.h"

#include "linkload.h"

/*list initialization*/
int *initLoadSeq(){
    loadBalanceLinkHead = (LoadLink *)malloc(sizeof(LoadLink)); //head->next->key is the first term
    loadBalanceLinkEnd = (LoadLink *)malloc(sizeof(LoadLink)); //dummy rear, to protect dequeue from an empty list

    loadBalanceLinkHead->next = loadBalanceLinkEnd;
    loadBalanceLinkEnd->next = loadBalanceLinkEnd; 

    return 0;
}

/*list initialization*/
LoadLink *initlist(){
    LoadLink *head, *z;
    head = (LoadLink *)malloc(sizeof *head); //head->next->key is the first term
    z = (LoadLink *)malloc(sizeof *head); //dummy rear, to protect dequeue from an empty list
    //head = (LoadLink *)calloc(1, sizeof *head); //head->next->key is the first term
    //z = (LoadLink *)calloc(1, sizeof *head); //dummy rear, to protect dequeue from an empty list
    head->next = z;
    z->next = z; 
    return head;
}

/*enqueue a node LoadLink to the end (between head and rear)*/
LoadLink *enqueue(LoadLink *head, PortMapperTable portMapperTable){
    LoadLink *t, *r, *z;
    r = head;
    z = head->next; // to find the rear LoadLink
    while(z->next!=z){
        r = r->next; // to find the LoadLink before rear LoadLink
        z = r->next;
    }

    t = (LoadLink *)malloc(sizeof *t);
    //t = (LoadLink *)calloc(1, sizeof *t);

    t->portMapperTable = portMapperTable;
    t->next = z;
    r->next = t; //add the LoadLink to the end

    return head;
}

/*get the list size*/
int listsize(LoadLink *head){
    int len = 0;
    LoadLink *r;
    r = head->next;
    while(r->next!=r){
        r = r->next;
        len++;
    }
    return len;
}

/*update the load balance link with the latest choose,
 *or add new record into the load balance link*/
int recordLoadBalance(Request_Reply req_reply){
    int index = 0;
    int size = req_reply.response_number;
    int inList = 0;
    /*go over the linked list*/
    LoadLink *r;
    r = loadBalanceLinkHead->next;
    while(r->next!=r){
        /*find the matched node, and then update the link node*/
        if(strcmp(req_reply.portMapperTable[0].program_name, r->portMapperTable.program_name) == 0 &&
           strcmp(req_reply.portMapperTable[0].version_number, r->portMapperTable.version_number) == 0 &&
           strcmp(req_reply.portMapperTable[0].procedure_name, r->portMapperTable.procedure_name) == 0){
             /*update response_index to use next index*/
             r->portMapperTable.loadbalance_index = (r->portMapperTable.loadbalance_index + 1) % size;
             index = r->portMapperTable.loadbalance_index;
             //printf("r->response_index=%d\n", r->response_index);
             /*update the stored server_ip and port_number*/
             snprintf(r->portMapperTable.server_ip, sizeof(req_reply.portMapperTable[index].server_ip),
                      "%s", req_reply.portMapperTable[index].server_ip);
             snprintf(r->portMapperTable.port_number, sizeof(req_reply.portMapperTable[index].port_number), 
                      "%s", req_reply.portMapperTable[index].port_number);

             inList = 1;
             break;
        }

        r = r->next;
    }
    /*add new node into the list, use the fisrt record (index=0) by default*/
    if(inList == 0){
        loadBalanceLinkHead = enqueue(loadBalanceLinkHead, req_reply.portMapperTable[0]);
    }

    return index;
}
