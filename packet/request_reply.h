#ifndef __REQUEST_REPLY_H__
#define __REQUEST_REPLY_H__

#define REQUEST_REPLY 1
#include "packet.h"
Packet *genRequestReply(Packet *request_serv);
int sendRequestReply(int sockfd, Packet *packet_req);
int printRequestReply(Packet *packet_reply, char *exec_remote_ipstr, char *exec_remote_port);
Packet *recvRequestReply(int sockfd);

#endif
