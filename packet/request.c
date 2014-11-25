#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include "checksum.h"

#define REQUEST_SERV 1
#include "packet.h"

#include "../lib/liblog.h"
#include "../lib/libfile.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"

Packet *genRequest(OptionsStruct *options, char *server_ip, char *port_mapper_ip){
    Request_Serv request_serv; // MUST NOT be pointer as to be send remote
    snprintf(request_serv.program_name, sizeof(request_serv.program_name), "%s", options->option1);
    snprintf(request_serv.version_number, sizeof(request_serv.version_number), "%s", options->option2);
    snprintf(request_serv.procedure_name, sizeof(request_serv.procedure_name), "%s", options->option3);

    /*wrap into packet*/
    Packet *request_packet;
    request_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Request_Service type Data

    snprintf(request_packet->sender_id, sizeof(request_packet->sender_id), "%s", server_ip);
    snprintf(request_packet->receiver_id, sizeof(request_packet->receiver_id), "%s", port_mapper_ip);
    snprintf(request_packet->packet_type, sizeof(request_packet->packet_type), "%s", "010"); 

    request_packet->Data = (Request_Serv) request_serv; // Data

    /*checksum*/
    snprintf(request_packet->PacketChecksum, sizeof(request_packet->PacketChecksum), "%d", chksum_crc32((unsigned char*) request_packet, sizeof(*request_packet)));

    return request_packet;
}

/* port_mapper_file is in the format of
 *   Server IP  | Sever Port | Program Name | Version | Procedure
 *   192.168.1,1| 54321      | ScientificLibrary | 1  | Multiply
*/
Request_Reply *readPortMapperTable(Packet *request_serv, char *file){
    Request_Reply *reqReply;
    reqReply = (Request_Reply *)calloc(1, sizeof(Request_Reply));

    reqReply->response_number = 0;

    char hostfile[32];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, "../");
    strcat(hostfile, file);


    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if(access(hostfile, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", hostfile);
        logging(LOGFILE, logmsg);
        return reqReply;
    }

    if ((fp = fopen(hostfile,"r")) < 0){
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", hostfile);
        logging(LOGFILE, logmsg);
        return reqReply;
    }

    int i = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        struct PortMapperTable *portMapperTable;
        portMapperTable = (struct PortMapperTable *)malloc(sizeof(struct PortMapperTable));
        portMapperTable = parseRequestString(line);
        //to find the register info which match the request program_name, version_number, and procedure_name
        if(strcmp(portMapperTable->program_name, request_serv->Data.program_name) == 0 &&
           strcmp(portMapperTable->version_number, request_serv->Data.version_number) == 0 &&
           strcmp(portMapperTable->procedure_name, request_serv->Data.procedure_name) == 0){
             //store the port mapper table record into request reply packet
             snprintf(reqReply->portMapperTable[i].server_ip, sizeof(reqReply->portMapperTable[i].server_ip), "%s", portMapperTable->server_ip);
             snprintf(reqReply->portMapperTable[i].port_number, sizeof(reqReply->portMapperTable[i].port_number), "%s", portMapperTable->port_number);
             snprintf(reqReply->portMapperTable[i].program_name, sizeof(reqReply->portMapperTable[i].program_name), "%s", portMapperTable->program_name);
             snprintf(reqReply->portMapperTable[i].version_number, sizeof(reqReply->portMapperTable[i].version_number), "%s", portMapperTable->version_number);
             snprintf(reqReply->portMapperTable[i].procedure_name, sizeof(reqReply->portMapperTable[i].procedure_name), "%s", portMapperTable->procedure_name);
             //printf("%s|%d|%s|%s|%s\n", portMapperTable->server_ip, portMapperTable->port_number,
             //        portMapperTable->program_name, portMapperTable->version_number, 
             //       portMapperTable->procedure_name);
             i++;
        }
        free(portMapperTable);
    }

    fclose(fp);
    if (line)
        free(line);

    char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "read register from port mapper table. \n");
    logging(LOGFILE, logmsg);

    reqReply->response_number = i;
    return reqReply; //return the records number
}
