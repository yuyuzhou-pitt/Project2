#ifndef __EXECUTE_RESULT_H__
#define __EXECUTE_RESULT_H__

#define EXECUTE_REPLY 1

#include "packet.h"
#include "linkseq.h"

Packet *recvExecuteResult(int sockfd);
int writeResultFile(char *result_file);

#endif
