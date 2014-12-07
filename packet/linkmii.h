#ifndef __LINK_MII_H__
#define __LINK_MII_H__

typedef struct POSTING{
    char term[30];
    int count; // record the doc count
    char doc_id[128]; //the filename of the document
    int payload;
    struct POSTING *next;
}POSTING;

typedef struct MIITable{
    POSTING *posting;
    struct MIITable *next;
}MIITable;

POSTING *initPostList();
MIITable *initMIITable();
POSTING *insertPostList(POSTING *head, POSTING *posting);
MIITable *insertMIITable(MIITable *head, MIITable *mii);
int mii2file(char *file, MIITable *head);

#endif
