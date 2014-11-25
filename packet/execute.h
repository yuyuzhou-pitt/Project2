#ifndef __EXECUTE_SERV_H__
#define __EXECUTE_SERV_H__

#define EXECUTE_SERV 1
#include "packet.h"
#include "linkseq.h"
#include "../lib/libterminal.h"

Packet_Seq *genExecuteServ(OptionsStruct *options, char *client_ip, char *server_ip, char *trans_id);
int calcResult(OptionsStruct *result_options, char *client_ip, char *server_ip, Packet_Seq *recv_exec_seq);
int sendExecuteServ(int sockfd, struct sockaddr_in sockaddr, Packet *packet);
int recvExecuteServ(int sockfd, struct sockaddr *sockaddr, Packet *packet);

#endif
