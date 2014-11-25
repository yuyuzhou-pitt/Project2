/* This file provides commands for the SRPC terminal.
 * It divides into four parts:
 * 1. general
 * 2. port-mapper
 * 3. server
 * 4. client
 */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "liblog.h"
#include "../packet/packet.h"
#include "../lib/libterminal.h"

/*************
* 1. general *
*************/

// parse command into options, 3 the most
OptionsStruct *command2struct(char *input){
    char *command, *argument1, *argument2, *argument3, *argument4, *argument5;
    char *temp1, *temp2, *temp3, *temp4; 
    OptionsStruct *parse_options;
    parse_options = (OptionsStruct *)calloc(1, sizeof(OptionsStruct));
    command = strtok_r(input, " ", &temp1); //option2 is for temp use
    argument1 = strtok_r(temp1, " ", &temp2);
    argument2 = strtok_r(temp2, " ", &temp3);
    argument3 = strtok_r(temp3, " ", &temp4);
    argument4 = strtok_r(temp4, " ", &argument5);
    snprintf(parse_options->command, sizeof(parse_options->command), "%s", command);
    snprintf(parse_options->option1, sizeof(parse_options->option1), "%s", argument1);
    snprintf(parse_options->option2, sizeof(parse_options->option2), "%s", argument2);
    snprintf(parse_options->option3, sizeof(parse_options->option3), "%s", argument3);
    snprintf(parse_options->option4, sizeof(parse_options->option4), "%s", argument4);
    snprintf(parse_options->option5, sizeof(parse_options->option5), "%s", argument5);
    return parse_options;
}

// parse command into options, 3 the most
int command2array(char *command, char options[3][32]){
    char *option1, *option2, *option3, *option4;
    option1 = strtok_r(command, " ", &option2); //option2 is for temp use
    option3 = strtok_r(option2, " ", &option4);
    strncpy(options[0], option1, 32);
    strncpy(options[1], option3, 32);
    strncpy(options[2], option4, 32);
    //options[0] = option1;
    //options[1] = option3;
    //options[2] = option4;
    return 0;
}

struct PortMapperTable *parseRequestString(char *line){
    char *option1, *option2, *option3, *option4, *option5, *option6, *option7, *option8;
 
    struct PortMapperTable *portMapperTable;
    portMapperTable = (struct PortMapperTable *)malloc(sizeof(struct PortMapperTable));
 
    option1 = strtok_r(line, "|", &option2); //option2 is for temp use
    option3 = strtok_r(option2, "|", &option4); //so does option4
    option5 = strtok_r(option4, "|", &option6); //so does option6
    option7 = strtok_r(option6, "|", &option8);
 
    snprintf(portMapperTable->server_ip, sizeof(portMapperTable->server_ip), "%s", option1);
    snprintf(portMapperTable->port_number, sizeof(portMapperTable->port_number), "%s", option3);
    snprintf(portMapperTable->program_name, sizeof(portMapperTable->program_name), "%s", option5);
    snprintf(portMapperTable->version_number, sizeof(portMapperTable->version_number), "%s", option7);
    snprintf(portMapperTable->procedure_name, strlen(option8), "%s", option8); //trim the last '\n'
 
    return portMapperTable;
}

// exit the services
int quit(){
    char confirm[4];
    fprintf(stdout, "WARNING: Do you want to quit? Please confirm: [y/N]");
    scanf("%c", confirm);
    if(confirm[0] == 'y'){
    //if(strcmp(confirm, "y") == 0 || strcmp(confirm, "y\273AJ4") == 0 ) { //confirm[0] == 'y'){
        return 0;
    }
    return 1;
}

/*****************
* 2. port-mapper *
*****************/

int helpPortMapper(){
    printf("\nUsage: <subcommand>\n");
    printf("A user interface for the SRPC system.\n\n");
    printf("Subcommands:\n");
    printf("  help      show this message\n");
    printf("  list      list the current services in port mapper table\n");
    printf("  quit      stop SRPC socket and quit\n");
    return 0;
}

/*for port-mapper terminal, to print the requst*/
int listPortMapper(){
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char file[32];
    memset(file, 0, sizeof(file));

    strcpy(file, "../");
    strcat(file, PORT_MAPPER_TABLE_FILE);

    if(access(file, F_OK) < 0) {
        fprintf(stdout, "Sorry, no service found. Please register first.\n");
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if ((fp = fopen(file,"r")) < 0){
        fprintf(stdout, "Sorry, can not get service information.\n");
        char logmsg[128]; snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    fprintf(stdout, "Server_IP     |Port |Program_name|Version|Procedure\n");
    while ((read = getline(&line, &len, fp)) != -1) {
        fprintf(stdout, "%s", line);
    }

    fclose(fp);

    return 0;
}

/************
* 3. server *
************/

int helpServer(){
    printf("\nUsage: <subcommand> [options]\n");
    printf("A user interface for the SRPC system.\n\n");
    printf("Subcommands:\n");
    printf("  help      show this message\n");
    printf("  register  <program-name> <version-number>\n");
    printf("            register services into prot-mapper table\n");
    printf("                <program-name>   the program name provided by this server\n");
    printf("                <version-number> version number for the program\n");
    printf("  quit      stop SRPC socket and quit\n");
    return 0;
}

/************
* 4. client *
************/

int helpClient(){
    printf("\nUsage: <subcommand> [options]\n");
    printf("A user interface for the SRPC system.\n\n");
    printf("Subcommands:\n");
    printf("  help      show this message\n");
    printf("  request   <program-name> <version-number> <procedure>\n");
    printf("		request server info for certain procedure provided by certain program of certain version\n");
    printf("  execute   <program-name> <version-number> <procedure> <input_file> <out_putfile>\n");
    printf("            request and call the remote procedure call\n");
    printf("  quit      stop SRPC socket and quit\n");

    return 0;
}
