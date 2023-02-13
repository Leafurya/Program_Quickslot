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
	typedef struct _target{
		char *path;
		HWND hWnd;
	}Target;
BOOL CALLBACK GetHwndProc(HWND hWnd,LPARAM lParam){
	//printf("%d\n",hWnd);
	Target *target=(Target *)lParam;
	BOOL isVisible=IsWindowVisible(hWnd);
	DWORD exStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
	BOOL isAppWindow=(exStyle&WS_EX_APPWINDOW);
	BOOL isToolWindow=(exStyle&WS_EX_TOOLWINDOW);
	BOOL isOwned=GetWindow(hWnd,GW_OWNER)?TRUE:FALSE;
	
	DWORD pID;
	HANDLE hProc;
	char path[1024]={0};
	
	if(!isVisible){
		return TRUE;
	}
	if(!(isAppWindow||(!isToolWindow&&!isOwned))){
		return TRUE;
	}
	
	GetWindowThreadProcessId(hWnd,&pID);
	hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
	if(hProc){
		GetModuleFileNameEx(hProc,NULL,path,1024);
		if(!strcmp(path,target->path)){
			target->hWnd=hWnd;
			CloseHandle(hProc);
			return FALSE;
		}
	}
	CloseHandle(hProc);
	return TRUE;
}
char SpreadQuickslot(QuickSlot slot){
	int i;
	Target target;
	Item item;
	SHELLEXECUTEINFOA info[ITEM_MAXSIZE]={0,};
	RECT rect;
	
	ZeroMemory(info,sizeof(info));
	if(slot.itemCount!=0){
		for(i=0;i<slot.itemCount;i++){
			item=slot.item[i];
			
			info[i].cbSize = sizeof(SHELLEXECUTEINFO);
	        info[i].fMask = NULL;
	        info[i].hwnd = NULL;
	        info[i].lpVerb = NULL;
	        info[i].lpFile = item.path;
	        info[i].lpParameters = NULL;
	        info[i].lpDirectory = NULL;
	        info[i].nShow = SW_MAXIMIZE;
	        info[i].hInstApp = NULL;
			
			ShellExecuteEx(&info[i]);
			EnumWindows(GetHwndProc,(LPARAM)&target);
			GetWindowRect(target.hWnd,&rect);
			SetWindowPos(target.hWnd,HWND_NOTOPMOST,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,SWP_SHOWWINDOW);
			if(item.maximized){
				ShowWindow(target.hWnd,SW_SHOWMAXIMIZED);
			}
		}
		return 0;
	}
	return 1;
}
