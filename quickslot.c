#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>
#include <psapi.h>

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
char LoadQuickslot(QuickSlot *pQuickslot[KEYCOUNT],int size){
	FILE *file=fopen("data/slot","rb");
	
	if(file==NULL){
		printf("%s\n",strerror(errno));
		return 0;
	}
	
	fread(pQuickslot,size,1,file);
	
	fclose(file);
	
	return 1;
}
char SaveQuickslot(QuickSlot *pQuickslot,int size){
	FILE *file=fopen("data/slot","wb");
	
	if(file==NULL){
		printf("%s\n",strerror(errno));
		return 0;
	}
	
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
BOOL CALLBACK GetHwndProc(HWND hWnd,LPARAM lParam){
	//printf("%d\n",hWnd);
	Item *target=(Item *)lParam;
	BOOL isVisible=IsWindowVisible(hWnd);
	DWORD exStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
	BOOL isAppWindow=(exStyle&WS_EX_APPWINDOW);
	BOOL isToolWindow=(exStyle&WS_EX_TOOLWINDOW);
	BOOL isOwned=GetWindow(hWnd,GW_OWNER)?TRUE:FALSE;
	
	DWORD pID;
	HANDLE hProc;
	char path[1024]={0};
	char tpath[1024]={0};
	
	if(!isVisible){
		return TRUE;
	}
	if(!(isAppWindow||(!isToolWindow&&!isOwned))){
		return TRUE;
	}
	GetWindowThreadProcessId(hWnd,&pID);
	hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
	if(hProc){
		GetModuleFileNameEx(hProc,NULL,tpath,1024);
		sprintf(path,"\"%s\"\0",tpath);
		//printf("%s\n%s\n\n",path,target->path);
		if(!strcmp(path,target->path)){
			printf("hWnd:%d\n",hWnd);
			target->hWnd=hWnd;
			CloseHandle(hProc);
			return FALSE;
		}
		ZeroMemory(path,sizeof(path));
	}
	CloseHandle(hProc);
	return TRUE;
}
char SpreadQuickslot(QuickSlot slot){
	int i;
	Item item;
	SHELLEXECUTEINFOA info[ITEM_MAXSIZE]={0,};
	RECT rect;
	char cmd[2048]={0};
	
	ZeroMemory(info,sizeof(info));
	if(slot.itemCount!=0){
		for(i=0;i<slot.itemCount;i++){
			item=slot.item[i];
			ZeroMemory(cmd,sizeof(cmd));
			sprintf(cmd,"%s %s",item.path,item.parameter);
			printf("cmd: %s\n",cmd);
			ShellExecute(NULL,"open",item.path,item.parameter,NULL,SW_SHOW);
		}
		Sleep(500);
		for(i=0;i<slot.itemCount;i++){
			item=slot.item[i];
			EnumWindows(GetHwndProc,(LPARAM)&item);
			GetWindowRect(item.hWnd,&rect);
			MoveWindow(item.hWnd,item.xpos,item.ypos,rect.right-rect.left,rect.bottom-rect.top,TRUE);
			printf("hWnd:%d\t%s\n",item.hWnd,item.path);
			if(item.maximized){
				ShowWindow(item.hWnd,SW_SHOWMAXIMIZED);
			}
			//SetWindowText(item.hWnd,"solt item");
			//CloseHandle(target.hWnd);
		}
		return 0;
	}
	return 1;
}
void ShowItemList(QuickSlot slot,HWND list){
	int i;
	int listCount=SendMessage(list,LB_GETCOUNT,0,0);
	
	for(i=listCount;i>=0;i--){
		if(SendMessage(list,LB_DELETESTRING,i,0)==-1){
			printf("%s\n",strerror(errno));
		}
	}
	for(i=0;i<slot.itemCount;i++){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)slot.item[i].name);
	}
	
	if(!i){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)"EMPTY");
	}
}
void ShowItemInfo(int index,Item item,HWND stText){
	char info[2048]={0,};
	sprintf(info,"F%d 슬롯\n\n경로:%s\n\n매개변수:%s",index,item.path,item.parameter?item.parameter:"non");
	SetWindowText(stText,info);
}
