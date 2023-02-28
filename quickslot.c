#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>
#include <strproc2.h>
#include <shellapi.h>
#include <psapi.h>

#include "list.h"
#include "progressbar.h"

QuickSlot *originSlotAdr;
List list;

void ShowSlotData(QuickSlot *slot){
	int i,j;
	printf("ShowSlotData==========\n");
	for(i=0;i<KEYCOUNT;i++){
		for(j=0;j<slot[i].itemCount;j++){
			printf("maxi:%d\thWnd:%d\tparam: %s|path:%s\n",slot[i].item[j].maximized,slot[i].item[j].hWnd,slot[i].item[j].parameter,slot[i].item[j].path);
			printf("(%d,%d)\n",slot[i].item[j].xpos,slot[i].item[j].ypos);
		}
		printf("-------------------------\n");
	}
}
int GetSlotIndex(int key){
	int i;
	static char toggle=1;
	if(GetKeyState(VK_CONTROL)&0x8000){
		for(i=0;i<KEYCOUNT;i++){
			if((GetKeyState(VK_F1+i)&0x8000)&&toggle){
				toggle=0;
				return i;
			}
		}
	}
	else{
		toggle=1;
	}
	return -1; //didn't find
}
char LoadQuickslot(QuickSlot (*pQuickslot)[KEYCOUNT],int size){
	FILE *file=fopen("data/slot","rb");
	
	if(file==NULL){
		return 0;
	}
	
	fread(pQuickslot,size,1,file);
	
	fclose(file);
	
	return 1;
}
char SaveQuickslot(QuickSlot *pQuickslot,int size){
	FILE *file=fopen("data/slot","wb");
	
	if(file==NULL){
		return 0;
	}
	
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
	BOOL IsNotHWNDInSlot(HWND hWnd){
		int i,j;
		for(i=0;i<KEYCOUNT;i++){
			if(originSlotAdr[i].itemCount!=0){
				for(j=0;j<originSlotAdr[i].itemCount;j++){
					if(originSlotAdr[i].item[j].hWnd==hWnd){
						return 0;
					}
				}
			}
		}
		return 1;
	}
	BOOL FilterWindow(HWND hWnd){
		BOOL isVisible=IsWindowVisible(hWnd);
		DWORD exStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
		BOOL isAppWindow=(exStyle&WS_EX_APPWINDOW);
		BOOL isToolWindow=(exStyle&WS_EX_TOOLWINDOW);
		BOOL isOwned=GetWindow(hWnd,GW_OWNER)?TRUE:FALSE;
		
		if(!isVisible){
			return TRUE;
		}
		if(!(isAppWindow||(!isToolWindow&&!isOwned))){
			return TRUE;
		}
		return FALSE;
	}
	BOOL ComparePreWindows(char *str,List *list){
		char *data; 
		//printf("GetCurStr : %s\n\n",str);
		while(MoveNext(list)){
			data=(char *)GetCurData(*list);
			//printf("GetCurData: %s\n",data);
			if(!strcmp(str,data)){
				free(str);
				ReturnToHead(list);
				return 0;
			}
		}
		AddData(list,str);
		
		ReturnToHead(list);
		return 1;
	}
BOOL CALLBACK GetHwndProc(HWND hWnd,LPARAM lParam){
	Item *target=(Item *)lParam;
	DWORD pID;
	HANDLE hProc;
	char path[1024]={0};
	char tpath[1024]={0};
	char *data;
	int i;
	
	if(FilterWindow(hWnd)){
		return TRUE;
	}
	
	GetWindowThreadProcessId(hWnd,&pID);
	hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
	if(hProc){
		GetModuleFileNameEx(hProc,NULL,tpath,1024);
		data=GetString("%d\"%s\"\0",hWnd,tpath);
		if(ComparePreWindows(data,&list)){
			printf("ComparePreWindows: %s\n",data);
			//free(data);
			sprintf(path,"\"%s\"",tpath);
			//printf("%s\n%s\n\n",path,data);
			if(IsNotHWNDInSlot(hWnd)){
				target->hWnd=hWnd;
				//printf("path: %s\t%d\n\n",target->path,target->hWnd);
				CloseHandle(hProc);
				if(IsZoomed(hWnd)){
					ShowWindow(hWnd,SW_SHOWNORMAL);
				}
				
				return FALSE;
			}
		}
		//printf("path: %s\t%d\n\n",target->path,target->hWnd);
		ZeroMemory(path,sizeof(path));
	}
	CloseHandle(hProc);
	return TRUE;
}
BOOL CALLBACK SavePreWindows(HWND hWnd,LPARAM lParam){
	List *list=(List *)lParam;
	DWORD pID;
	HANDLE hProc;
	char path[1024]={0};
	char tpath[1024]={0};
	int i;
	char *data;
	
	if(FilterWindow(hWnd)){
		return TRUE;
	}
	
	GetWindowThreadProcessId(hWnd,&pID);
	hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
	if(hProc){
		GetModuleFileNameEx(hProc,NULL,tpath,1024);
		sprintf(path,"%d\"%s\"\0",hWnd,tpath);
		AddData(list,(char *)GetString("%d\"%s\"\0",hWnd,tpath));
		ZeroMemory(path,sizeof(path));
	}
	CloseHandle(hProc);
	return TRUE;
}
	void StopSpread(char blockVar){
		
	}
char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex){
	int i,j;
	//SHELLEXECUTEINFOA info[ITEM_MAXSIZE]={0,};
	RECT rect;
	char cmd[2048]={0};
	QuickSlot slot=pOriginSlot[slotIndex];
	Item *items=pOriginSlot[slotIndex].item;
	
	originSlotAdr=pOriginSlot;
	int timeout=0;
	char blockVar=0;
	
	
	//ZeroMemory(info,sizeof(info));
	if(slot.itemCount!=0){
		SetBlockVar(&blockVar);
		InitList(&list);
		EnumWindows(SavePreWindows,(LPARAM)&list);
//		while(MoveNext(&list)){
//			printf("list: %s\n",(char *)GetCurData(list));
//		}
//		ReturnToHead(&list);
//		printf("\n");
		for(i=0;i<slot.itemCount;i++){
			while(blockVar){
				if(blockVar==-1){
					for(i=0;i<slot.itemCount;i++){
						DestroyWindow(items[i].hWnd);
					}
					FreeList(&list);
					return -1;
				}
			}
			StopSpread(*blockVar);
			if(items[i].hWnd){
				items[i].hWnd=0;
			}
			timeout=0;
			if(ShellExecute(NULL,"open",items[i].path,strlen(items[i].parameter)?items[i].parameter:NULL,NULL,SW_SHOW)<=(HINSTANCE)32){
				printf("cant open\n");
			}
			Sleep(200);
			do{
				EnumWindows(GetHwndProc,(LPARAM)&items[i]);
				//printf("%d ",timeout);
				if(timeout>=15000){
					//printf("cant find %s\n",items[i].path);
					items[i].hWnd=0;
					break;
				}
				timeout++;
			}while(!items[i].hWnd);
			StepBar();
		}
		for(i=0;i<slot.itemCount;i++){
			while(blockVar){
				if(blockVar==-1){
					return -1;
				}
			}
			//printf("%d%s\n",items[i].hWnd,items[i].path);
			if(items[i].hWnd){
				MoveWindow(items[i].hWnd,items[i].xpos,items[i].ypos<0?100:items[i].ypos,items[i].w,items[i].h,TRUE);
				if(items[i].maximized){
					ShowWindow(items[i].hWnd,SW_SHOWMAXIMIZED);
				}
				//SetForegroundWindow(items[i].hWnd);
				Sleep(100);
			}
			StepBar();
		}
		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
		FreeList(&list);
		return 0;
	}
	return 1;
}
void ShowItemList(QuickSlot slot,HWND list){
	int i;
	int listCount=SendMessage(list,LB_GETCOUNT,0,0);
	
	for(i=listCount;i>=0;i--){
		SendMessage(list,LB_DELETESTRING,i,0);
	}
	for(i=0;i<slot.itemCount;i++){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)slot.item[i].name);
	}
	
	if(!i){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)"EMPTY");
	}
}
void ShowItemInfo(char *name,Item item,HWND stText){
	char info[2048]={0,};
	
	sprintf(info,"%s\n\n경로:%s\n\n매개변수:%s",strlen(name)?name:"-",item.path,strlen(item.parameter)?item.parameter:"-");//strlen(name)?name:"-"
	SetWindowText(stText,info);
}
void CloseSlot(QuickSlot *slot){
	int i;
	for(i=0;i<slot->itemCount;i++){
		PostMessage(slot->item[i].hWnd,WM_CLOSE,0,0);
		slot->item[i].hWnd=0;
	}
}
