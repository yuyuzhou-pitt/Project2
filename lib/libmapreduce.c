#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "liblog.h"
#include "libfile.h"
#include "../packet/packet.h"

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
