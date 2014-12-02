#include <stdio.h>
#include <stdlib.h>
#include "../lib/libscientific.h"

RemoteProgram *ScientificLibrary(){
    RemoteProgram *remoteProgram;
    remoteProgram = (RemoteProgram *)malloc(sizeof(RemoteProgram));

    snprintf(remoteProgram->program_name, sizeof(remoteProgram->program_name), "ScientificLibrary");
    snprintf(remoteProgram->version_number, sizeof(remoteProgram->version_number), "1");
    remoteProgram->procedure_number = 4; 
    remoteProgram->data_is_file_or_dir = 0;
    remoteProgram->data_type = 0; // int
    snprintf(remoteProgram->procedures[0], sizeof(remoteProgram->procedures[0]), "Multiply");
    snprintf(remoteProgram->procedures[1], sizeof(remoteProgram->procedures[1]), "Sort");
    snprintf(remoteProgram->procedures[2], sizeof(remoteProgram->procedures[2]), "Min");
    snprintf(remoteProgram->procedures[3], sizeof(remoteProgram->procedures[3]), "Max");

    return remoteProgram;
}

RemoteProgram *ScientificLibraryV2(){
    RemoteProgram *remoteProgram;
    remoteProgram = (RemoteProgram *)malloc(sizeof(RemoteProgram));
    remoteProgram->procedure_number = 4;
    remoteProgram->data_is_file_or_dir = 0;
    remoteProgram->data_type = 0; // int
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
    remoteProgram->data_is_file_or_dir = 1;
    remoteProgram->data_type = 2; // str
    snprintf(remoteProgram->procedures[0], sizeof(remoteProgram->procedures[0]), "Index");
    snprintf(remoteProgram->procedures[1], sizeof(remoteProgram->procedures[1]), "Search");

    return remoteProgram;
}

/*Configure me: configurable library function*/
RemoteProgram *getLibraryPtr(char *program_name, char *version_number){
    RemoteProgram *wrongProgram = NULL;

    if (strcmp(program_name, "ScientificLibrary") == 0){ // for client
        if(strcmp(version_number, "1") == 0){
            return &ScientificLibrary; 
        }
        else if(strcmp(version_number, "2") == 0){
            return &ScientificLibraryV2;
        }
    }
    else if(strcmp(program_name, "MapReduceLibrary") == 0){ // for minigoogle
        if(strcmp(version_number, "1") == 0){
            return &MapReduceLibrary; 
        }
    }
    else{
        return wrongProgram;
    }
}
