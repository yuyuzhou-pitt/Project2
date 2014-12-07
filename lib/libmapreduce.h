#include "libfile.h"

#define _MAX_WORDLEN_ 32 
#define _MAX_FILEBUFFERLEN_ 512
#define _MAX_WORDNUMBER_ 2<<16

typedef enum _CharType{
	endoffile,
	character,
	puncmark,
	space,
	begin,
}CharType;

typedef enum _bool{
	false,
	true
}bool;

typedef struct _interMediate{
	char* word;
	int count;
}interMediate;

#define CLEARTOEK do{	\
	top=-1; 	\
	memset((char*)token,0,_MAX_WORDLEN_); \
}while(0)

#define FULLFILLTOKEN token[++top]='\0'
#define APPENDTOEKN token[++top]=tolower(ch)

#define GETCHAR do{	\
	ch = fgetc(wrdcount_file); \
	if( ( (ch<='z') && (ch>='a')) || ( (ch>='A') && (ch<='Z') ) ){ \
		chtype = character; \
	}else if( (ch==' ') || (ch=='\t') || (ch=='\n') || (ch=='\r')){	\
		chtype = space; \
	}else if( ch == EOF ){ \
		chtype = endoffile; \
	}else{	\
		chtype = puncmark;	\
	}	\
}while(0)

int initWordcount(char *file, char *target_dir);
int getToken();
int Wordcount(char *file, char *target_dir);
int Wordsort(char *file, char *target_dir);

int letter_freq[] = {1182,1271,1513,774,687,798,711,745,445,262,264,694,1026,384,337,1203,87,745,2052,936,153,317,541,8,88,49};


