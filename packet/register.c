#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#define REGISTER_SERV 1
#include "packet.h"

#include "checksum.h"
#include "../lib/liblog.h"
#include "../lib/libfile.h"
#include "../lib/libscientific.h"
#include "../lib/libterminal.h"


/* Format for port mapper table:
 *   Server IP  | Sever Port | Program Name | Version | Procedure
 *   192.168.1,1| 54321      | ScientificLibrary | 1  | Multiply
*/

Packet *genRegister(OptionsStruct *options, char *server_ip, char *port_mapper_ip){
    struct Remote_Program_Struct *sciLibrary;
    sciLibrary = (struct Remote_Program_Struct*)malloc(sizeof(struct Remote_Program_Struct)); //Packet with Register_Service type Data
    sciLibrary = programLibrary();
    //int getPort(char *portstr, int size, char *ipfile)
    Register_Serv register_serv; // MUST NOT be pointer as to be send remote
    snprintf(register_serv.program_name, sizeof(register_serv.program_name), "%s", options->option1);
    snprintf(register_serv.version_number, sizeof(register_serv.version_number), "%s", options->option2);
    snprintf(register_serv.port_number, sizeof(register_serv.port_number), "%d", getPortNumber(server_ip));
    register_serv.procedure_number = 4;
    snprintf(register_serv.procedure_names[0], sizeof(register_serv.procedure_names[0]), "%s", sciLibrary->procedure1);
    snprintf(register_serv.procedure_names[1], sizeof(register_serv.procedure_names[1]), "%s", sciLibrary->procedure2);
    snprintf(register_serv.procedure_names[2], sizeof(register_serv.procedure_names[2]), "%s", sciLibrary->procedure3);
    snprintf(register_serv.procedure_names[3], sizeof(register_serv.procedure_names[3]), "%s", sciLibrary->procedure4);

    /*wrap into packet*/
    Packet *register_packet;
    register_packet = (Packet *)malloc(sizeof(Packet)); //Packet with Register_Service type Data

    snprintf(register_packet->sender_id, sizeof(register_packet->sender_id), "%s", server_ip);
    snprintf(register_packet->receiver_id, sizeof(register_packet->receiver_id), "%s", port_mapper_ip);
    snprintf(register_packet->packet_type, sizeof(register_packet->packet_type), "%s", "000"); // Register Packets (000)

    register_packet->Data = (Register_Serv) register_serv; // Data

    /*checksum*/
    snprintf(register_packet->PacketChecksum, sizeof(register_packet->PacketChecksum), 
             "%d", chksum_crc32((unsigned char*) register_packet, sizeof(*register_packet)));

    free(sciLibrary);
    return register_packet;
}

/* port_mapper_file is in the format of
 *   Server IP  | Sever Port | Program Name | Version | Procedure
 *   192.168.1,1| 54321      | ScientificLibrary | 1  | Multiply
*/
int writePortMapperTable(Packet *packet_recv, char *file){
    char registerString[128];
    struct PortMapperTable *portMapperTable;
    portMapperTable = (struct PortMapperTable *)malloc(sizeof(struct PortMapperTable));

    char hostfile[17];
    memset(hostfile, 0, sizeof(hostfile));

    strcpy(hostfile, "../");
    strcat(hostfile, file);

    int file_exists = 1;
    if(access(hostfile, F_OK) < 0) {
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", hostfile);
        logging(LOGFILE, logmsg);
        file_exists = 0;
    }

    char *tempfile = ".tempfile";

    if (file_exists == 1){
        if(copy_file(hostfile, tempfile) != 0)
            fprintf(stderr, "Error during copy!");
    }

    int server_exists = 0; //for sending hello if it's a new server
    int dup_number = 0; //record the duplication numbers
    int i;
    for(i=0;i<packet_recv->Data.procedure_number;i++){
        /*printable str*/
        memset(registerString, 0, sizeof(registerString));
        snprintf(registerString, sizeof(registerString), "%s|%s|%s|%s|%s\n", \
             packet_recv->sender_id, packet_recv->Data.port_number, \
             packet_recv->Data.program_name, packet_recv->Data.version_number, \
             packet_recv->Data.procedure_names[i]);

        /*check duplication before writing*/
        int is_dup = 0; 

        FILE *table_fp;
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        /*do not check dup if file do not exists*/
        if(file_exists == 1) {
            if ((table_fp = fopen(tempfile,"r")) < 0){
                char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", tempfile);
                logging(LOGFILE, logmsg);
                return -1;
            }
    
            while ((read = getline(&line, &len, table_fp)) != -1) {
                portMapperTable = parseRequestString(line);
                //to find the register info which match the request program_name, version_number, and procedure_name
                if(strcmp(portMapperTable->server_ip, packet_recv->sender_id) == 0 &&
                   strcmp(portMapperTable->port_number, packet_recv->Data.port_number) == 0){
                    server_exists = 1; 
                    if(strcmp(portMapperTable->program_name, packet_recv->Data.program_name) == 0 &&
                       strcmp(portMapperTable->version_number, packet_recv->Data.version_number) == 0 &&
                       strcmp(portMapperTable->procedure_name, packet_recv->Data.procedure_names[i]) == 0){
                        is_dup = 1;
                        break; //jump out the while loop
                    }
                }
            }
            if (line) free(line);
            fclose(table_fp);
        }

        if (is_dup == 1){
            printf("Duplicate registration: %s", registerString);
            dup_number ++;
            continue;
        }

        if(writeFile(registerString, strlen(registerString), hostfile, "a") < 0){
            perror("writeport");
            return -1;
        }

        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "write register into port mapper table. \n");
        logging(LOGFILE, logmsg);

    }

    unlinkFile(tempfile);
    free(portMapperTable);
    return dup_number * 10 + server_exists;
}
