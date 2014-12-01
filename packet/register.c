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
    sciLibrary = (*libraryPtr)();
    //int getPort(char *portstr, int size, char *ipfile)
    Register_Serv register_serv; // MUST NOT be pointer as to be send remote
    snprintf(register_serv.program_name, sizeof(register_serv.program_name), "%s", options->option1);
    snprintf(register_serv.version_number, sizeof(register_serv.version_number), "%s", options->option2);
    snprintf(register_serv.port_number, sizeof(register_serv.port_number), "%d", getPortNumber(server_ip));
    register_serv.procedure_number = sciLibrary->procedure_number;
    int pN;
    for(pN=0; pN < sciLibrary->procedure_number; pN++){
        snprintf(register_serv.procedure_names[pN], sizeof(register_serv.procedure_names[pN]), "%s", sciLibrary->procedures[pN]);
    }

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
