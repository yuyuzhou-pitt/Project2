#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>

#include "libmapreduce.h"

static FILE* wrdcount_file;
static CharType chtype;
static CharType prv_chtype;
static char ch;
static char token[_MAX_WORDLEN_];
static int top;
static errbit;

int initWordcount(char *file, char *target_dir){
	errbit=0;
	ch = 0;
	chtype=begin;
	prv_chtype=begin;

	CLEARTOEK;
	memset((char*)token,0,_MAX_WORDLEN_);

	if(access(file, F_OK) < 0) {
		errbit=1;
        return -1;
    }

    if ((wrdcount_file = fopen(file,"r")) < 0){
    	errbit=2;
        return -1;
    }
    return 0;
}

int getToken(){
	bool breakflag=false;
	do{
		GETCHAR;
		switch(chtype){
			case (character):
				switch(prv_chtype){
						case (begin):
							APPENDTOEKN;
							prv_chtype=character;
							break;
						case (space):
							APPENDTOEKN;
							prv_chtype=character;
							break;
						case (puncmark):
							prv_chtype=puncmark;
							break;
						case (character):
							APPENDTOEKN;
							prv_chtype=character;
							break;
				}break;
			case (space):
				switch(prv_chtype){
						case (begin):
							prv_chtype=space;
							break;
						case (space):
							prv_chtype=space;
							break;
						case (puncmark):
							prv_chtype=space;
							break;
						case (character):
							FULLFILLTOKEN;
							breakflag=true;
							prv_chtype=space;
							break;
				}break;
			case (puncmark):
				switch(prv_chtype){
						case (begin):
							prv_chtype=puncmark;
							break;
						case (space):
							prv_chtype=space;
							break;
						case (puncmark):
							prv_chtype=puncmark;
							break;
						case (character):
							FULLFILLTOKEN;
							breakflag=true;
							prv_chtype=puncmark;
							break;
				}break;
			case (endoffile):
				switch(prv_chtype){
						case (begin):
							breakflag=true;
							break;
						case (space):
							breakflag=true;
							break;
						case (puncmark):
							breakflag=true;
							break;
						case (character):
							FULLFILLTOKEN;
							breakflag=true;
							break;
				}break;
			default:
				breakflag=true;
		}
		if(breakflag){
			break;
		}
	}while(true);
	return 0;
}

int fcmp(const void *p1,const void *p2)
{
	return (strcmp((char *)p1, (char*)p2));
}

char* getfilename(char *file){
	int i=strlen(file)-1;
	while( (i>0) && file[i]!='/' ) --i;
	if(i!=0) ++i;
	return ((char*)(&(file[i])));
}

int Wordcount(char *file, char *target_dir){
	char wordset[_MAX_WORDNUMBER_][_MAX_WORDLEN_];
	int wstop=-1,count=0,ret=0;

	initWordcount(file,target_dir);
	struct timeval t_stamp = getUTimeStamp();
	char full_file_path[_MAX_FILEBUFFERLEN_];
	//open file to write into
	char* ctmpptr = strstr(file,".txt"); memset(ctmpptr,0,strlen(ctmpptr)*sizeof(char)); *ctmpptr='\0';
	ctmpptr=getfilename(file);
	snprintf(full_file_path, sizeof(full_file_path), "%s/%s.txt___%d.%d.txt", target_dir, 
                 ctmpptr, (int)(t_stamp.tv_sec),(int)(t_stamp.tv_usec));
	FILE* fout=fopen(full_file_path,"w");
	if(fout){
		for(getToken();chtype!=endoffile;getToken()){
			strcpy(wordset[++wstop],token);
			CLEARTOEK;
		}
		if(prv_chtype==character){
			strcpy(wordset[++wstop],token);	
		}
		//sort
		qsort(wordset, wstop, _MAX_WORDLEN_*(sizeof(char)), fcmp);
		//write into file
		int i=0,j=0,sum=0;
		for(i=0;i<wstop;i=j){
			for(sum=0,j=i; (strcmp(wordset[j],wordset[i])==0) && j<wstop; ++j,++sum)	;
			fprintf(fout,"%s %d\n",wordset[i],sum);
		}
		fclose(fout);
	}else{
		return -1;
	}
	return 0;
}

static char _Wswordbuff[_MAX_WORDLEN_];
static int _Wscount;

int Wordsort1letter(FILE* fin, FILE* foutvector, char chpattern, char* _filename){
	if(_Wswordbuff[0]>chpattern) return 0;
	if(_Wswordbuff[0]<chpattern)
		while( (fscanf(fin,"%s %d",(char*)_Wswordbuff,&_Wscount) >1) && _Wswordbuff[0]<chpattern ) ;
	do{
		fprintf(foutvector,"%s\t%s\t%d\n",_Wswordbuff,_filename,_Wscount);
	}while( (fscanf(fin,"%s %d",(char*)_Wswordbuff,&_Wscount) >1) && _Wswordbuff[0]==chpattern);
	return 0;
}

int WordSort1file(char *srcdir, char *destdir, int pos_start, int pos_end){
	DIR *in_dp;
    struct dirent *in_ep;
    FILE* foutvector[total_letter];
    FILE* ftmp=NULL;

    char path_foutbuffer[_MAX_FILEBUFFERLEN_];
    int i=0,j=0,vtop=-1;
    for(i=pos_start;i<=pos_end;++i){
    	sprintf((char*)path_foutbuffer,"%s/%c.txt",destdir,'a'+i);
//printf("WordSort1file, %s\n",path_foutbuffer);
    	if( (ftmp=fopen(path_foutbuffer,"a")) != NULL){
 		   	foutvector[++vtop]=ftmp;
		}
    }
//printf("top = %d\n",vtop);
    in_dp = opendir((char*)srcdir);
    FILE* fin=NULL;
    char file_path[_MAX_FILEBUFFERLEN_];
    if (in_dp != NULL){
        while (in_ep = readdir (in_dp)){
            if(strcmp(getStrAfterDelimiter(in_ep->d_name, '.'), "txt") != 0){
                continue;
            }
            snprintf(file_path, sizeof(file_path), "%s/%s", (char*)srcdir, in_ep->d_name);
            if((fin=fopen(file_path,"r"))==NULL){
            	continue;
            }
            memset(_Wswordbuff,0,_MAX_WORDLEN_);
            strcpy(file_path,in_ep->d_name);
            char* ctmpptr = strstr(file_path,"___"); memset(ctmpptr,0,strlen(ctmpptr)*sizeof(char)); *ctmpptr='\0';
            for(j=0;j<=vtop;++j){
           		Wordsort1letter((FILE*)fin,(FILE*)(foutvector[j]),'a'+pos_start+j,(char*)file_path);
           	}
        }
        (void) closedir (in_dp);
    }
	return 0;
}

int WordSort(char *srcdir, char *destdir, int totalworker, int currentno)
{
	int work_splits = total_letter/totalworker;
	int work_left = total_letter%totalworker;
	int start_poswork=work_splits*currentno ,end_poswork=work_splits*(currentno+1)-1;
	if(currentno==totalworker-1){
		end_poswork+=work_left;
	}
//printf("WordSort: %s, %s, %d, %d\n",srcdir,destdir,start_poswork,end_poswork);
	WordSort1file(srcdir,destdir,start_poswork,end_poswork);
	return 0;
}

/*
int main(int argc, char* argv[]){
//	Wordcount(argv[1],argv[2]);
	printf("wordcount Done\n");
	int totalworker=5;
	int currentno=1;
	WordSort(argv[1],argv[2],totalworker,currentno);
	return 0;
}
*/
