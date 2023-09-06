#include <stdio.h>
#include <strproc2.h>
#include <time.h>
#include "log.h"
#include "list.h"
#include "progressbar.h"
#include "thread.h"

#define _CRT_SECURE_NO_WARNINGS

List *msgList;
char block;

unsigned __stdcall LogThreadFunc(void *args){
	printf("save log\n");
	FILE *file=fopen("log.log","a");
	char *message;
	
	ShowAllData(msgList);
	
	ReturnToHead(msgList);
	while(MoveNext(msgList)){
		fseek(file,0,SEEK_SET);
		message=(char *)RemoveData(msgList,0);
		
		fwrite(message,strlen(message),1,file);
//		RemoveData(msgList,0);
		free(message);
	}
	
//	FreeList(msgList);
//	free(msgList);
	fclose(file);
}

void LogMessage(char *msg){
	time_t timer=time(NULL);
	struct tm *t=localtime(&timer);
	
	if(!msgList){
		msgList=(List *)malloc(sizeof(List));
		InitList(msgList);
	}
	
	char *message=GetString("(%d.%d.%d. %d:%d:%d)\t%s\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,msg);
	
	AddData(msgList,message);
	
	SetNowLog(msg);
//	free(msg);
}
void SaveLog(){
	printf("savelog\n");
	ShowAllData(msgList);
	StartThread(LogThreadFunc,NULL);
}
void EndLogging(){
	void *data;
	ReturnToHead(msgList);
	while(MoveNext(msgList)){
		data=(char *)RemoveData(msgList,0);
		free(data);
	}
	free(msgList);
}
