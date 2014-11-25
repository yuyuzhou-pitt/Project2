#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "checksum.h"
#define EXECUTE_ACK 1
#include "packet.h"

#include "../lib/libsocket.h"
#include "../lib/liblog.h"
#include "linkseq.h"

Packet *genExecuteAck(Packet *execute_serv){
    /*wrap into packet*/
    Packet *execute_ack;

    execute_ack = (Packet *)malloc(sizeof(Packet)); //Packet with Register_Service type Data

    snprintf(execute_ack->sender_id, sizeof(execute_ack->sender_id), "%s", execute_serv->receiver_id);
    snprintf(execute_ack->receiver_id, sizeof(execute_ack->receiver_id), "%s", execute_serv->sender_id);
    snprintf(execute_ack->packet_type, sizeof(execute_ack->packet_type), "%s", "110"); //ack packet

    snprintf(execute_ack->Data.version_number, sizeof(execute_ack->Data.version_number), "%s", execute_serv->Data.version_number);
    snprintf(execute_ack->Data.program_name, sizeof(execute_ack->Data.program_name), "%s", execute_serv->Data.program_name);
    snprintf(execute_ack->Data.procedure_name, sizeof(execute_ack->Data.procedure_name), "%s", execute_serv->Data.procedure_name);
    execute_ack->Data.transaction_id = execute_serv->Data.transaction_id ;
    execute_ack->Data.end_flag = execute_serv->Data.end_flag;
    execute_ack->Data.seq = execute_serv->Data.seq + 1;

    /*checksum*/
    snprintf(execute_ack->PacketChecksum, sizeof(execute_ack->PacketChecksum), "%d", chksum_crc32(((unsigned char*) execute_ack) , sizeof(*execute_ack)));

    return execute_ack;
}

int sendExecuteAck(int sockfd, struct sockaddr_in sockaddr, Packet *packet){
   Packet *packet_reply;
   packet_reply = (Packet *)malloc(sizeof(Packet)); //Packet with Execute_Serv type Data

   packet_reply = genExecuteAck(packet); // msg to be sent back
   return Sendto(sockfd, packet_reply, sizeof(Packet), MSG_NOSIGNAL, sockaddr, sizeof(sockaddr));
}

/*for client, receiving the ack, do not need sockaddr*/
Packet *recvExecuteAck(int sockfd){
//int recvExecuteAck(int sockfd, Packet *packet){
    Packet *packet_ack;
    packet_ack = (Packet *)malloc(sizeof(Packet)); //Packet with Execute_Serv type Data

    RecvfromServer(sockfd, packet_ack, sizeof(Packet), MSG_NOSIGNAL); 
    return packet_ack;
}

int recvResultAck(int sockfd, struct sockaddr *sockaddr, Packet *packet){
    return Recvfrom(sockfd, packet, sizeof(Packet), MSG_NOSIGNAL, sockaddr, sizeof(sockaddr));
}
