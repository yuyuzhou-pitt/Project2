/*
 * Wrap of socket fuctions.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include "liblog.h"
#include "libfile.h"
#include "../packet/packet.h"

/*wrap socket*/
int Socket(int family, int type, int protocal){
    int sockfd;
    if((sockfd = socket(family, type, protocal)) < 0){
        perror("socket");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Socket id = %d\n",sockfd);
    logging(LOGFILE, logmsg);

    return sockfd;
}

/*wrap bind*/
int Bind(int sockfd, struct sockaddr_in sockaddr){
    int n;
    if((n = bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr))) < 0){
        perror("bind");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Bind success!\n");
    logging(LOGFILE, logmsg);

    return n;
}

/*got assigned sock port*/
int Getsockname(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n, port;
    if((n = getsockname(sockfd, (struct sockaddr *)&sockaddr,&sin_size)) < 0){
        perror("getsockname");
        return -1;
    }
    
    port = ntohs(sockaddr.sin_port);
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "socket has port %d\n", port); /* Display port number */
    logging(LOGFILE, logmsg);

    return port;
}

/*wrap listen*/
int Listen(int sockfd, int max_que_comm_nm){
    int n;
    if((n = listen(sockfd, max_que_comm_nm) < 0)){
         perror("listen");
         return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Listening....\n");
    logging(LOGFILE, logmsg);
    return n;
}

/*wrap accept*/
int Accept(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int client_fd;
    if ((client_fd = accept(sockfd,(struct sockaddr *)&sockaddr, &sin_size)) < 0){
        perror("accept");
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "New incoming connection - %d\n", client_fd);
    //logging(LOGFILE, logmsg);
    return client_fd;
}

/*wrap recvfrom*/
int Recvfrom(int sockfd, Packet *packet, int size, int flag, struct sockaddr *sockaddr, int sin_size){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd, packet, size, flag, sockaddr, &sin_size)) < 0){
        perror("recvfrom");
        return -1;
    }    
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Recvfrom message: %s with type %s.\n", 
                               packet->sender_id, packet->packet_type);
    logging(LOGFILE, logmsg);
    return recvbytes;
}

/*wrap recvfrom server, sockaddr is NULL*/
int RecvfromServer(int sockfd, Packet *packet, int size, int flag){
    int recvbytes;
    if ((recvbytes = recvfrom(sockfd, packet, size, flag, NULL, NULL)) < 0){
        perror("recvfrom_server");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "RecvfromServer message: %s\n", packet->sender_id);
    logging(LOGFILE, logmsg);
    return recvbytes;
}

/*wrap sendto*/
int Sendto(int sockfd, Packet *packet, int size, int flag, struct sockaddr_in sockaddr, int sin_size){
    int sendbytes;
    if ((sendbytes = sendto(sockfd, packet, size, flag, (struct sockaddr *)&sockaddr, sin_size)) < 0){
        perror("sendto");
        return -1;
    }
    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Sendto message: %s with type %s.\n", 
                               packet->receiver_id, packet->packet_type);
    logging(LOGFILE, logmsg);
    return sendbytes;
}

/*wrap recv*/
int Recv(int sockfd, Packet *packet, int size, int flag){
    int recvbytes;
    if ((recvbytes = recv(sockfd, packet, size, flag)) < 0){
        perror("recv");
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Recv message from: %s\n", packet->sender_id);
    //logging(LOGFILE, logmsg);
    return recvbytes;
}

/*wrap send*/
int Send(int sockfd, Packet *packet, int size, int flag){
    int sendbytes;
    if ((sendbytes = send(sockfd, packet, size, flag)) < 0){
        perror("send");
        return -1;
    }
    //char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "Send message: %s\n",packet->sender_id);
    //logging(LOGFILE, logmsg);
    return sendbytes;
}

/*wrap connect*/
int Connect(int sockfd, struct sockaddr_in sockaddr, int sin_size){
    int n;
    if((n = connect(sockfd,(struct sockaddr *)&sockaddr,sin_size)) < 0){
        perror("connect");
        return -1;
    }
    return n;
}
