#include <stdio.h>
#include <stdlib.h>
#include "../lib/libscientific.h"

RemoteProgram *programLibrary(){
    RemoteProgram *remoteProgram;
    remoteProgram = (RemoteProgram *)malloc(sizeof(RemoteProgram));

    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "ScientificLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "1");
    remoteProgram->procedure_number = 4; 
    snprintf(remoteProgram->procedures[0], sizeof(remoteProgram->procedures[0]), "Multiply");
    snprintf(remoteProgram->procedures[1], sizeof(remoteProgram->procedures[1]), "Sort");
    snprintf(remoteProgram->procedures[2], sizeof(remoteProgram->procedures[2]), "Min");
    snprintf(remoteProgram->procedures[3], sizeof(remoteProgram->procedures[3]), "Max");

    return remoteProgram;
}


RemoteProgram *programLibraryv2(){
    RemoteProgram *remoteProgram;
    remoteProgram = (RemoteProgram *)malloc(sizeof(RemoteProgram));
    remoteProgram->procedure_number = 4;
    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "ScientificLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "2");
    snprintf(remoteProgram->procedures[0], sizeof(remoteProgram->procedures[0]), "Multiply");
    snprintf(remoteProgram->procedures[1], sizeof(remoteProgram->procedures[1]), "Sort");
    snprintf(remoteProgram->procedures[2], sizeof(remoteProgram->procedures[2]), "Min");
    snprintf(remoteProgram->procedures[3], sizeof(remoteProgram->procedures[3]), "Max");

    return remoteProgram;
}

/*for map reduce*/
RemoteProgram *MapReduceLibrary(){
    RemoteProgram *remoteProgram;
    remoteProgram = (RemoteProgram *)malloc(sizeof(RemoteProgram));
    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "MapReduceLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "1");
    remoteProgram->procedure_number = 2;
    snprintf(remoteProgram->procedures[0], sizeof(remoteProgram->procedures[0]), "Index");
    snprintf(remoteProgram->procedures[1], sizeof(remoteProgram->procedures[1]), "Reduce");

    return remoteProgram;
}

/*Configure me: configurable library function*/
RemoteProgram *getLibraryPtr(){
    //return &programLibrary; // for client
    return &MapReduceLibrary; // for minigoogle
}
