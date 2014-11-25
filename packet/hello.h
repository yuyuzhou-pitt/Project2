#ifndef __HELLO_H__
#define __HELLO_H__

#define HELLO_MSG 1
#include "packet.h"

Packet *genHelloReq(Packet *packet);
int sendHello(int sockfd, Packet *packet);

#endif
