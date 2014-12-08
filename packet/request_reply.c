#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include "checksum.h"
#include "../lib/liblog.h"

#define REQUEST_REPLY 1
#include "packet.h"
#include "../packet/linkload.h"
#include "../packet/request.h"
#include "../lib/libsocket.h"

/*for port mapper, to generate request reply*/
Packet *genRequestReply(Packet *request_serv){
    Request_Reply *reqReply, request_reply; // MUST NOT be pointer as to be send remote

    reqReply = (Request_Reply *)calloc(1, sizeof(Request_Reply));
    reqReply = readPortMapperTable(request_serv, PORT_MAPPER_TABLE_FILE);

    int i;
    for(i=0;i< reqReply->response_number; i++){
        snprintf(request_reply.portMapperTable[i].server_ip, sizeof(request_reply.portMapperTable[i].server_ip), "%s", reqReply->portMapperTable[i].server_ip);
        snprintf(request_reply.portMapperTable[i].port_number, sizeof(request_reply.portMapperTable[i].port_number), "%s", reqReply->portMapperTable[i].port_number);
        snprintf(request_reply.portMapperTable[i].program_name, sizeof(request_reply.portMapperTable[i].program_name), "%s", reqReply->portMapperTable[i].program_name);
        snprintf(request_reply.portMapperTable[i].version_number, sizeof(request_reply.portMapperTable[i].version_number), "%s", reqReply->portMapperTable[i].version_number);
        snprintf(request_reply.portMapperTable[i].procedure_name, sizeof(request_reply.portMapperTable[i].procedure_name), "%s", reqReply->portMapperTable[i].procedure_name);
    }

    request_reply.response_number = reqReply->response_number;

    /*wrap into packet*/
    Packet *packet_reply;
    packet_reply = (Packet *)malloc(sizeof(Packet)); //Packet with Request_Service type Data

    snprintf(packet_reply->sender_id, sizeof(packet_reply->sender_id), "%s", request_serv->receiver_id);
    snprintf(packet_reply->receiver_id, sizeof(packet_reply->receiver_id), "%s", request_serv->sender_id);
    snprintf(packet_reply->packet_type, sizeof(packet_reply->packet_type), "%s", "011"); //Request_Reply

    packet_reply->Data = (Request_Reply) request_reply; // Data

    /*checksum*/
    snprintf(packet_reply->PacketChecksum, sizeof(packet_reply->PacketChecksum), "%d", chksum_crc32((unsigned char*) packet_reply, sizeof(*packet_reply)));

    free(reqReply);
    return packet_reply;
}

/*for port mapper, to send request reply*/
int sendRequestReply(int sockfd, Packet *packet_req){
    /* generate neighbors_reply reply according to configure file */
    Packet *packet_reply;
    packet_reply = (Packet *)malloc(sizeof(Packet)); //Packet with Request_Service type Data

    packet_reply = genRequestReply(packet_req); // msg to be sent back

    /*update the load balance link with the latest choose,
     *or add new record into the load balance link*/
    Request_Reply req_reply;
    req_reply = packet_reply->Data;
    if(packet_reply->Data.response_number > 0){
        packet_reply->Data.response_index = recordLoadBalance(req_reply);
        //printf("sendRequestReply.packet_reply->Data.response_index=%d\n", packet_reply->Data.response_index);
    }

    /*send the request reply packet*/
    return Send(sockfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);
}

Packet *recvRequestReply(int sockfd){
    Packet *packet_reply;
    packet_reply = (Packet *)malloc(sizeof(Packet));

    Recv(sockfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL);
    return packet_reply;
}

/*for client terminal, to print the requst*/
int printRequestReply(Packet *packet_reply, Request_Reply *requested_servers, char *exec_remote_ipstr, char *exec_remote_port){
    /*store the requested servers for execute using*/
    requested_servers->response_number = packet_reply->Data.response_number;

    /*print the using response only*/
    if(packet_reply->Data.response_number > 0){
        int index = packet_reply->Data.response_index;
        int i;
        fprintf(stdout, "Server_IP     |Port |Program_name     |Version|Procedure\n");
                //136.142.227.13|41668|ScientificLibrary|1      |Sort
        for(i=0;i < packet_reply->Data.response_number;i++){
            fprintf(stdout, "%s|%s|%s|%s      |%s\n", packet_reply->Data.portMapperTable[i].server_ip, \
                 packet_reply->Data.portMapperTable[i].port_number, \
                 packet_reply->Data.portMapperTable[i].program_name, \
                 packet_reply->Data.portMapperTable[i].version_number, \
                 packet_reply->Data.portMapperTable[i].procedure_name);

            snprintf(requested_servers->portMapperTable[i].server_ip, sizeof(requested_servers->portMapperTable[i].server_ip), 
                 "%s", packet_reply->Data.portMapperTable[i].server_ip);
            snprintf(requested_servers->portMapperTable[i].port_number, sizeof(requested_servers->portMapperTable[i].port_number), 
                 "%s", packet_reply->Data.portMapperTable[i].port_number);
        }

        snprintf(exec_remote_ipstr, sizeof(packet_reply->Data.portMapperTable[index].server_ip), 
                 "%s", packet_reply->Data.portMapperTable[index].server_ip);
        snprintf(exec_remote_port, sizeof(packet_reply->Data.portMapperTable[index].port_number), 
                 "%s", packet_reply->Data.portMapperTable[index].port_number);
        //fprintf(stdout, "\nChoose to use service on %s with port %s for load balance.\n", 
        //        exec_remote_ipstr, exec_remote_port);
    }
    else{
        fprintf(stdout,"No service available! Please use 'list' on Port Mapper to get the available services.\n");
    }
    return 0;
}
