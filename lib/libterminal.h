#ifndef __LIB_TERMINAL__
#define __LIB_TERMINAL__

#include "../packet/packet.h"

/*for terminal options*/
typedef struct OptionsStruct{
    int count; //the total number of the options
    char command[128]; //. i.e.: register
    char option1[128]; //i.e.: program name
    char option2[128]; //i.e.: version number
    char option3[128]; //i.e.: procedure name
    char option4[128]; //i.e.: input file
    char option5[128]; //i.e.: output file
    char action[30]; // split, index, or search
    char remote_ipstr[100];
    char remote_port[6];
}OptionsStruct;


/*************
* 1. general *
*************/

OptionsStruct *command2struct(char *input);
struct PortMapperTable *parseRequestString(char *line);
int quit();

/*****************
* 2. port-mapper *
*****************/

int listPortMapper();
int helpPortMapper();

/************
* 3. server *
************/

int helpServer();

/************
* 4. client *
************/
int helpClient();

#endif
