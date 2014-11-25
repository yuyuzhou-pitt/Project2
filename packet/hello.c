#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "checksum.h"
#define HELLO_MSG 1
#include "packet.h"

#include "../lib/liblog.h"
#include "../lib/libsocket.h"

Packet *genHelloReq(Packet *packet){
    Hello_Msg hello_msg; // MUST NOT be pointer as to be send remote
    snprintf(hello_msg.port_number, sizeof(hello_msg.port_number), "%s", packet->Data.port_number);

    /*wrap into packet*/
    Packet *hello_packet;
    hello_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Hello_Msg type Data

    snprintf(hello_packet->sender_id, sizeof(hello_packet->sender_id), "%s", packet->receiver_id);
    snprintf(hello_packet->receiver_id, sizeof(hello_packet->receiver_id), "%s", packet->sender_id);
    snprintf(hello_packet->packet_type, sizeof(hello_packet->packet_type), "%s", "111"); //Hello type

    hello_packet->Data = (Hello_Msg) hello_msg; // reuse the Data
    //printf("PortID=%d", hello_packet->Data.PortID);

    /*checksum*/
    snprintf(hello_packet->PacketChecksum, sizeof(hello_packet->PacketChecksum), 
             "%d", chksum_crc32((unsigned char*) hello_packet, sizeof(*hello_packet)));

    return hello_packet;
}

int sendHello(int sockfd, Packet *packet){
    /* generate hellos_reply reply according to configure file */
    Packet *hello_packet;
    hello_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Hello_Msg type Data
    hello_packet = genHelloReq(packet); // msg to be sent back
    //printf("sendHello: hello_packet->PacketType = %s\n", hello_packet->PacketType);
    return Send(sockfd, hello_packet, sizeof(Packet), MSG_NOSIGNAL);
}
