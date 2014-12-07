#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
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

int Wordcount(char *file, char *target_dir){
	char wordset[_MAX_WORDNUMBER_][_MAX_WORDLEN_];
	int wstop=-1,count=0,ret=0;

	initWordcount(file,target_dir);
	struct timeval t_stamp = getUTimeStamp();
	char full_file_path[_MAX_FILEBUFFERLEN_];
	//open file to write into
	char* ctmpptr = strstr(file,".txt"); memset(ctmpptr,0,strlen(ctmpptr)*sizeof(char)); *ctmpptr='\0';
	snprintf(full_file_path, sizeof(full_file_path), "%s/%s.txt___%d.%d.txt", target_dir, 
                 file, (int)(t_stamp.tv_sec),(int)(t_stamp.tv_usec));
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

int WordSort(char *srcdir, char *destdir, int totalworker, int currentno)
{
	
	return 0;
}

int main(int argc, char* argv[]){
	Wordcount(argv[1],argv[2]);
	printf("wordcount Done\n");
	//Wordsort(argv[1],argv[2]);
	return 0;
}
