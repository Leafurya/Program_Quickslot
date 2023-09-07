#include "list.h"
#include <stdlib.h>
#include <stdio.h>

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
//	printf("add data: %s\n",(char *)data);
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
void *RemoveData(List *list,int index){
	int i;
	void *data;
	Node *target;
	ReturnToHead(list);
	for(i=0;i<index;i++){
		MoveNext(list);
	}
	
	target=list->cur->next;
	data=list->cur->next->data;
	list->cur->next=list->cur->next->next;
	free(target);
	
	if(!list->cur->next){
		list->tail=list->cur;
	}
	return data;
}
char FindData(List *list,void *data,char (*Compare)(void *data1,void *data2)){
	ReturnToHead(list);
	int index=0;
	while(MoveNext(list)){
		if(Compare(GetCurData(*list),data)){
			return index;
		}
		index++;
	}
	return -1;
}
void ShowAllData(List *list){
	ReturnToHead(list);
	while(MoveNext(list)){
		printf("list: %s\n",(char *)GetCurData(*list));
	}
}
