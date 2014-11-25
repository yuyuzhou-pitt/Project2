#ifndef __LIB_TERMINAL__
#define __LIB_TERMINAL__

#include "../packet/packet.h"

/*for terminal options*/
typedef struct OptionsStruct{
    char command[128]; //. i.e.: register
    char option1[128]; //i.e.: program name
    char option2[128]; //i.e.: version number
    char option3[128]; //i.e.: procedure name
    char option4[128]; //i.e.: input file
    char option5[128]; //i.e.: output file
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
