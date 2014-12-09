#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "linkmii.h"

/*list initialization*/
POSTING *initPostList(){
    POSTING *head, *z;
    head = (POSTING *)malloc(sizeof *head); //head->next->key is the first term
    z = (POSTING *)malloc(sizeof *head); //dummy rear, to protect dequeue from an empty list
    //head = (POSTING *)calloc(1, sizeof *head); //head->next->key is the first term
    //z = (POSTING *)calloc(1, sizeof *head); //dummy rear, to protect dequeue from an empty list
    head->next = z;
    z->next = z; 
    return head;
}

/*list initialization*/
MIITable *initMIITable(){
    MIITable *head, *z;
    head = (MIITable *)malloc(sizeof *head); //head->next->key is the first term
    z = (MIITable *)malloc(sizeof *head); //dummy rear, to protect dequeue from an empty list
    //head = (POSTING *)calloc(1, sizeof *head); //head->next->key is the first term
    //z = (POSTING *)calloc(1, sizeof *head); //dummy rear, to protect dequeue from an empty list
    head->next = z;
    z->next = z; 
    return head;
}

/*insert a node POSTING according to doc_id (alphabeta)*/
POSTING *insertPostList(POSTING *head, POSTING *posting){
    POSTING *t, *r, *z;
    r = head;
    z = head->next; // to find the rear MII

    int Max_flag = 1;
    while(z->next!=z){

        /*if doc_id existst, update the payload*/
        if (strcmp(z->term, posting->term) == 0 &&
            strcmp(z->doc_id, posting->doc_id) == 0){
           z->payload += posting->payload; 
           Max_flag = 0;
           break;
        }
        /*find the fist node with bigger (alphabeta) doc_id, insert posting before the node*/
        else if(strcmp(z->term, posting->term) == 0 &&
            strcmp(z->doc_id, posting->doc_id) > 0){

            posting->next = z;
            r->next = posting;

            Max_flag = 0;
            break;
        }

        r = r->next; // to find the POSTING before rear MII
        z = z->next;
    }

    /*if posting has the biggest doc_id, append it to the end*/
    if (Max_flag == 1){
        posting->next = z;
        r->next = posting;
    }

    return head;
}

/*merge two post list*/
POSTING *mergePostList(POSTING *list1, POSTING *list2){
    POSTING *r, *t, *z;
    r = list2;
    z = list2->next; // to find the rear MII

    while(z->next!=z){
        /*remove the node from list2 when inserting*/
        t = z;
        r->next = z->next;
        t->next = NULL;

        /*insert the single node*/
        insertPostList(list1, t);

        /*move only the rear pointer*/
        z = r->next;
    }

    return list1;
}

/*get the count sum of the posting*/
int postPayloadSum(POSTING *list){
    int sum = 0;

    POSTING *r, *z;
    r = list;
    z = list->next; // to find the rear MII

    while(z->next!=z){
        sum += z->payload;

        r = r->next; // to find the POSTING before rear MII
        z = z->next;
    }

    return sum;
}

/*insert a node MIITable according to term (alphabeta)*/
MIITable *insertMIITable(MIITable *head, MIITable *mii){
    MIITable *r, *z;
    r = head;
    z = head->next; // to find the rear MII
    
    int Max_flag = 1;
    while(z->next!=z){
    
        /*if iterm exists, insert the posting*/
        if (strcmp(z->posting->next->term, mii->posting->next->term) == 0){
           //insertPostList(z->posting, mii->posting->next);
           mergePostList(z->posting, mii->posting);
           Max_flag = 0;
           break;
        }
        /*find the fist node with bigger iterm (alphabeta), insert mii before the node*/
        else if(strcmp(z->posting->next->term, mii->posting->next->term) < 0){
            mii->next = z;
            r->next = mii;

            Max_flag = 0;
            break;
        }

        r = r->next; // to find the POSTING before rear MII
        z = z->next;
    }

    /*if posting has the biggest doc_id, append it to the end*/
    if (Max_flag == 1){
        mii->next = z;
        r->next = mii;
    }

    return head;
}

/*insert a node MIITable according to term (alphabeta)*/
MIITable *insertMIIByPayload(MIITable *head, MIITable *mii){

    MIITable *r, *z;
    r = head;
    z = head->next; // to find the rear MII

    while(z->next!=z){
        /* if iterm exists:
         * 1. merge the two posting into the one to be inserted
         * 2. remove the node from miitable*/
        if (strcmp(z->posting->next->term, mii->posting->next->term) == 0){
           //insertPostList(z->posting, mii->posting->next);
           mergePostList(z->posting, mii->posting);
           mii = z;
           r->next = z->next;
           //mii->next = NULL;
           break;
        }
        r = r->next;
        z = z->next;
    }

    int miisum = postPayloadSum(mii->posting);

    r = head;
    z = head->next;
    int Max_flag = 1;
    while(z->next!=z){
        /*find the fist node with small payload sum, insert mii before the node*/
        if(miisum > postPayloadSum(z->posting)){
            mii->next = z;
            r->next = mii;

            Max_flag = 0;
            break;
        }

        r = r->next;
        z = z->next;
    }

    /*if posting has the smallest payload sum, append it to the end*/
    if (Max_flag == 1){
        mii->next = z;
        r->next = mii;
    }

    return head;
}

int post2str(char *post_str, POSTING *list){
    POSTING *r, *z;
    r = list;
    z = list->next; // to find the rear MII

    snprintf(post_str, strlen(z->term)+1, "%s", z->term);
    while(z->next!=z){
        char doc_str[1024];
        snprintf(doc_str, sizeof(doc_str), "\t%s\t%d", z->doc_id, z->payload); 
        strcat(post_str, doc_str);
        r = r->next;
        z = z->next;
    }

    return 0;
}

int mii2str(char *mii_str, MIITable *miitalbe){
    MIITable *r, *z;
    r = miitalbe;
    z = miitalbe->next; // to find the rear MII
 
    while(z->next!=z){
        char post_str[10240];
        memset(post_str, 0, sizeof(post_str));
        post2str(post_str, z->posting);
        strcat(mii_str, post_str);
        strcat(mii_str, "\n");
        r = r->next; // to find the POSTING before rear MII
        z = z->next;
    }

    return 0;
}

int post2strWithSum(char *post_str, POSTING *list){
    POSTING *r, *z;
    r = list;
    z = list->next; // to find the rear MII

    int payloadsum = postPayloadSum(list);
    char post_head[128];
    snprintf(post_head, sizeof(post_head), "%s\t(Sum: %d)", z->term, postPayloadSum(list));
    snprintf(post_str, strlen(post_head)+1, post_head);
    while(z->next!=z){
        char doc_str[1024];
        snprintf(doc_str, sizeof(doc_str), "\t%s\t%d", z->doc_id, z->payload);
        strcat(post_str, doc_str);
        r = r->next;
        z = z->next;
    }

    return 0;
}

int mii2strWithSum(char *mii_str, MIITable *miitalbe){
    MIITable *r, *z;
    r = miitalbe;
    z = miitalbe->next; // to find the rear MII

    while(z->next!=z){
        char post_str[10240];
        memset(post_str, 0, sizeof(post_str));
        post2strWithSum(post_str, z->posting);
        strcat(mii_str, post_str);
        strcat(mii_str, "\n");
        r = r->next; // to find the POSTING before rear MII
        z = z->next;
    }

    return 0;
}

