#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "liblog.h"
#include "libfile.h"
#include "libterminal.h"
#include "../packet/packet.h"
#include "../packet/linkmii.h"

char logmsg[128];

/* Use below file fucntion to split file
 * int writeFile(char *str, int size, char *file, char *writeMode);
 * int readFile(char *str, int size, char *file);
*/

int Split(char *file, char *target_dir){
    char split_str[SPLIT_BLOCK];

    FILE *split_fp;

    if(access(file, F_OK) < 0) {
        snprintf(logmsg, sizeof(logmsg), "readfile: File not found: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    if ((split_fp = fopen(file,"r")) < 0){
        snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    struct timeval t_stamp;
    char target_file[1024];
    int count;
    while((count = fread(split_str, 1, SPLIT_BLOCK, split_fp)) > 0){
        int f_index = count;
        while(count == SPLIT_BLOCK && (split_str[f_index] != ' ' && split_str[f_index] != '\n')){
            f_index--;
        }
        fseek(split_fp, f_index-count, SEEK_CUR);
        t_stamp = getUTimeStamp();
        /*please use .txt to be the file extension (checking in client/socket_execute.c:jobTracker())*/
        snprintf(target_file, sizeof(target_file), "%s/%s___%d.%d.txt", target_dir, 
                 getStrAfterDelimiter(file, '/'), t_stamp.tv_sec, t_stamp.tv_usec);
        snprintf(logmsg, sizeof(logmsg), "Split: write into part file: %s\n", target_file);logging(LOGFILE, logmsg);
        writeFile(split_str, f_index, target_file, "w");
    }

    fclose(split_fp);

    return 0;
}

/*all the reduced file will be store in the temp directory .MII*/
int Reduce(char *file, char *target_dir){
    FILE *s_fp; 
    char *line = NULL;
    size_t len = 0;
    ssize_t n_read;
    
    if ((s_fp = fopen(file,"r")) < 0){
        snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }
    
    char w_line[1024];
    int aN;
    char target_file[1024];
    MIITable *miitable;
    miitable = initMIITable();
    int str_len = 0;
    while ((n_read = getline(&line, &len, s_fp)) != -1) {
        /*split line*/
        SplitStr *str_array;
        str_array = (SplitStr *)malloc(sizeof(SplitStr));
        str2array(str_array, line, '\t');
        if(str_array->count < 3){
            continue;
        }

        /*generate the posting list*/
        POSTING *postlist;
        postlist = initPostList();
        for(aN=1;aN < str_array->count;aN=aN+2){
            POSTING *posting;
            posting = (POSTING *)malloc(sizeof(POSTING));
            snprintf(posting->term, sizeof(posting->term), str_array->terms[0]);
            posting->count = (str_array->count -1)/2;
            snprintf(posting->doc_id, sizeof(posting->doc_id), str_array->terms[aN]);
            posting->payload = atoi(str_array->terms[aN+1]);

            /*insert posting into list according to doc_id (alphabeta)*/
            insertPostList(postlist, posting);
        }

        /*generate the mii node*/
        MIITable *mii;
        mii = (MIITable *)malloc(sizeof(MII));
        mii->posting = postlist; 

        /*insert posting list into mii table according to term (alphabeta)*/
        insertMIITable(miitable, mii);
    }
    fclose(s_fp);
    
    /*create new file name (e.g.: a_Sort.txt -> a_MII.txt)*/
    char old_file[30];
    snprintf(old_file, sizeof(old_file), getStrAfterDelimiter(file, '/'));
    snprintf(target_file, sizeof(target_file), "%s/%c_%s.txt", target_dir, old_file[0], MII);
    char mii_str[WRITE_BLOCK];
    memset(mii_str, 0, sizeof(mii_str));
    mii2str(mii_str, miitable);
    writeFile(mii_str, strlen(mii_str)+1, target_file, "w");
    return 0;
}

/*all the split file will be store in the directory .Single*/
int Search(char *file, char *term, char *target_dir){
    FILE *s_fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t n_read;

    if ((s_fp = fopen(file,"r")) < 0){
        snprintf(logmsg, sizeof(logmsg), "readfile: Failed to open file: %s\n", file);
        logging(LOGFILE, logmsg);
        return -1;
    }

    SplitStr *str_array;
    str_array = (SplitStr *)malloc(sizeof(SplitStr));
    char w_line[1024];
    int aN;
    int bingo = 0;
    char target_file[1024];
    snprintf(target_file, sizeof(target_file), "%s/%s.txt", target_dir, term);
    while ((n_read = getline(&line, &len, s_fp)) != -1) {
        str2array(str_array, line, '\t');
        if(strcmp(term, str_array->terms[0]) == 0){
            bingo = 1;
            for(aN=1;aN < str_array->count;aN=aN+2){
                snprintf(w_line, sizeof(w_line), "%s\t%s\n", str_array->terms[aN], str_array->terms[aN+1]);
                writeFile(w_line, sizeof(w_line), target_file, "a");
            }
        }
    }

    if(bingo == 0){
        snprintf(w_line, sizeof(w_line), "Sorry, term \"%s\" found nowhere.\n", term);
        writeFile(w_line, strlen(w_line), target_file, "a");
    }

    fclose(s_fp);

    return 0;
}
