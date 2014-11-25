#ifndef __EXECUTE_ACK_H__
#define __EXECUTE_ACK_H__

#define EXECUTE_ACK 1
#include "packet.h"
#include "linkseq.h"

Packet *genExecuteAck(Packet *packet);
int sendExecuteAck(int sockfd, struct sockaddr_in sockaddr, Packet *packet);
//int recvExecuteAck(int sockfd, Packet *packet);
Packet *recvExecuteAck(int sockfd);
int recvResultAck(int sockfd, struct sockaddr *sockaddr, Packet *packet);

#endif
