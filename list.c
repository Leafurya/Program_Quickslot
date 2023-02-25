#include "list.h"
#include <stdlib.h>

#ifndef NULL
#define NULL 0
#endif

void InitList(List *list){
	list->head=malloc(sizeof(Node));
	list->head->next=NULL;
	list->head->data=NULL;
	
	list->cur=list->head;
	list->tail=list->head;
}
void AddData(List *list,void *data){
	Node *newNode=malloc(sizeof(Node));
	newNode->data=data;
	newNode->next=NULL;
	
	list->tail->next=newNode;
	list->tail=newNode;
}
char MoveNext(List *list){
	if(!list->cur->next){
		return 0;
	}
	list->cur=list->cur->next;
	return 1;
}
void *GetCurData(List list){
	return list.cur->data;
}
void ReturnToHead(List *list){
	list->cur=list->head;
}
void FreeList(List *list){
	Node *tnode;
	for(list->cur=list->head;list->cur->next;){
		tnode=list->cur->next;
		free(list->cur);
		list->cur=tnode;
	}
}
