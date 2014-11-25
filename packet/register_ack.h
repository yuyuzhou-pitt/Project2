#ifndef __REGISTER_ACK_H__
#define __REGISTER_ACK_H__

#define REGISTER_ACK  1
#include "packet.h"

Packet *genRegisterReply(Packet *register_req, int dup_numbers);
int sendRegisterReply(int sockfd, Packet *packet_req, int dup_numbers);

#endif
