#ifndef __LIBSOCKET_H__
#define __LIBSOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "../packet/packet.h"

typedef struct ThreadSocket{
    int sockfd;
    struct sockaddr_in sockaddr;
}ThreadSocket;

int Socket(int family, int type, int protocal);
int Bind(int sockfd, struct sockaddr_in sockaddr);
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Listen(int sockfd, int max_que_comm_nm);
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size);
int Recvfrom(int sockfd, Packet *packet, int size, int flags, struct sockaddr *sockaddr, int sin_size);
int RecvfromServer(int sockfd, Packet *packet, int size, int flag);
int Sendto(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size);
int Recv(int sockfd, Packet *packet, int size, int flags);
int Send(int sockfd, Packet *packet, int size, int flag);
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size);

#endif
