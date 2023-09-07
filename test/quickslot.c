#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>

char LoadQuickslot(QuickSlot (*pQuickslot)[KEYCOUNT],int size){
	FILE *file=fopen("data/slot","rb");
	
	if(file==NULL){
		return 0;
	}
	
	fseek(file,1,SEEK_SET);
	fread(pQuickslot,size,1,file);
	
	fclose(file);
	
	return 1;
}
char SaveQuickslot(QuickSlot *pQuickslot,int size){
	FILE *file=fopen("data/slot","ab");
	
	if(file==NULL){
		return 0;
	}
	fseek(file,1,SEEK_SET);
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
char UpdateSlotFile(QuickSlot *pQuickslot,int size){
	FILE *file=fopen("data/slot","wb");
	char version=NOW_DATA_VERSION;
	
	if(file==NULL){
		return 0;
	}
	
	fwrite(&version,1,1,file);
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
void ShowSlotData(QuickSlot *slot){
	int i,j;
	printf("ShowSlotData:==========\n");
	for(i=0;i<KEYCOUNT;i++){
		printf("%d-%s-----------------------\n",i,slot[i].slotName);
		for(j=0;j<slot[i].itemCount;j++){
			printf("maxi:%d\thWnd:%d\tparam: %s|path:%s\n",slot[i].item[j].maximized,slot[i].item[j].hWnd,slot[i].item[j].parameter,slot[i].item[j].path);
//			printf("(%d,%d)\n",slot[i].item[j].xpos,slot[i].item[j].ypos);
			printf("(%d,%d) detecting: %d\n",slot[i].item[j].xpos,slot[i].item[j].ypos,slot[i].item[j].detecting);
		}
	}
}
QuickSlot *ChangeToV1(QuickSlotV0 *oldData){
	QuickSlot *newData=(QuickSlot *)malloc(sizeof(QuickSlot)*KEYCOUNT);
	int i,j;
	for(i=0;i<KEYCOUNT;i++){
		newData[i].itemCount=oldData[i].itemCount;
		sprintf(newData[i].slotName,"%s",oldData[i].slotName);
		for(j=0;j<ITEM_MAXSIZE;j++){
			newData[i].item[j].maximized=oldData[i].item[j].maximized;
			newData[i].item[j].xpos=oldData[i].item[j].xpos;
			newData[i].item[j].ypos=oldData[i].item[j].ypos;
			newData[i].item[j].w=oldData[i].item[j].w;
			newData[i].item[j].h=oldData[i].item[j].h;
			newData[i].item[j].hWnd=oldData[i].item[j].hWnd;
			newData[i].item[j].detecting=0;
			
			sprintf(newData[i].item[j].path,"%s",oldData[i].item[j].path);
			sprintf(newData[i].item[j].name,"%s",oldData[i].item[j].name);
			sprintf(newData[i].item[j].parameter,"%s",oldData[i].item[j].parameter);
		}
	}
	return newData;
}
void CheckVersion(){
	FILE *file=fopen("data/slot","rb");
	char version;
	void *oldData;
	QuickSlot *newData;
	int i,j;
	
	if(!file){
		return;
	}
	fread(&version,1,1,file);
	printf("data version: %d\n",version);
	
	if(NOW_DATA_VERSION==version){
		printf("new version\n");
		fclose(file);
		return;
	}
	printf("old version\n");
	switch(version){
		default:
			oldData=(QuickSlotV0 *)malloc(sizeof(QuickSlotV0)*KEYCOUNT);
			rewind(file);
			fread(oldData,1,sizeof(QuickSlotV0)*KEYCOUNT,file);
			break;
	}
	switch(NOW_DATA_VERSION){
		case 1:
			newData=ChangeToV1(oldData);
			UpdateSlotFile(newData,sizeof(QuickSlot)*KEYCOUNT);
			break;
	}
	fclose(file);
}
