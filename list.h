#ifndef _list__h_
#define _list__h_

typedef struct _node{
	void *data;
	struct _node *next;
}Node;
typedef struct _list{
	Node *head;
	Node *cur;
	Node *tail;
}List;

void InitList(List *);
void AddData(List *,void *);
char MoveNext(List *);
void *GetCurData(List);
void FreeList(List *);
void ReturnToHead(List *);
void *RemoveData(List *,int);
char FindData(List *,void *,char (*)(void *,void *));
void ShowAllData(List *);

#endif
