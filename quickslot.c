#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>
#include <psapi.h>

QuickSlot *originSlotAdr;

void ShowSlotData(QuickSlot *slot){
	int i,j;
	printf("ShowSlotData==========\n");
	for(i=0;i<KEYCOUNT;i++){
		for(j=0;j<slot[i].itemCount;j++){
			printf("maxi:%d\thWnd:%d\tparam: %s|path:%s\n",slot[i].item[j].maximized,slot[i].item[j].hWnd,slot[i].item[j].parameter,slot[i].item[j].path);
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
		printf("errno:%d\n",errno);
		printf("%s\n",strerror(errno));
		return 0;
	}
	
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
	char IsNotHWNDInSlot(HWND hWnd){
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
	int i;
	
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
			if(IsNotHWNDInSlot(hWnd)){
				//printf("hWnd:%d\n",hWnd);
				printf("lParam: %p\n",lParam);
				target->hWnd=hWnd;
				CloseHandle(hProc);
				if(IsZoomed(hWnd)){
					ShowWindow(hWnd,SW_SHOWNORMAL);
				}
				return FALSE;
			}
		}
		ZeroMemory(path,sizeof(path));
	}
	CloseHandle(hProc);
	return TRUE;
}
char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex){
	int i;
	SHELLEXECUTEINFOA info[ITEM_MAXSIZE]={0,};
	RECT rect;
	char cmd[2048]={0};
	QuickSlot slot=pOriginSlot[slotIndex];
	Item *items=pOriginSlot[slotIndex].item;
	
	originSlotAdr=pOriginSlot;
	
	printf("SpreadQuickslot:%p\n",pOriginSlot[slotIndex].item);
	printf("originSlotAdr:%p\n",originSlotAdr[slotIndex].item);
	
	ZeroMemory(info,sizeof(info));
	if(slot.itemCount!=0){
		for(i=0;i<slot.itemCount;i++){
			//item=items[i];
			//printf("cmd: %s\n",cmd);
			if(items[i].hWnd){
				CloseHandle(items[i].hWnd);
				items[i].hWnd=0;
			}
			ShellExecute(NULL,"open",items[i].path,strlen(items[i].parameter)?items[i].parameter:NULL,NULL,SW_SHOW);
			Sleep(200);
			printf("&items[i]:%p\n",&items[i]);
			do{
				EnumWindows(GetHwndProc,(LPARAM)&items[i]);
			}while(!items[i].hWnd);
			
			GetWindowRect(items[i].hWnd,&rect);
			//MoveWindow(items[i].hWnd,items[i].xpos,items[i].ypos<0?100:items[i].ypos,rect.right-rect.left,rect.bottom-rect.top,TRUE);
			printf("hWnd:%d\n",items[i].hWnd);
			if(items[i].maximized){
				MoveWindow(items[i].hWnd,items[i].xpos,items[i].ypos<0?100:items[i].ypos,500,500,TRUE);
			}
			else{
				MoveWindow(items[i].hWnd,items[i].xpos,items[i].ypos<0?100:items[i].ypos,rect.right-rect.left,rect.bottom-rect.top,TRUE);
			} 
			Sleep(100);
		}
		for(i=0;i<slot.itemCount;i++){
			if(items[i].maximized){
				ShowWindow(items[i].hWnd,SW_SHOWMAXIMIZED);
			}
		}
		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
//		Sleep(250);
//		for(i=0;i<slot.itemCount;i++){
//			//item=items[i];
//			EnumWindows(GetHwndProc,(LPARAM)&items[i]);
//			GetWindowRect(items[i].hWnd,&rect);
//			MoveWindow(items[i].hWnd,items[i].xpos,items[i].ypos,rect.right-rect.left,rect.bottom-rect.top,TRUE);
//			//printf("hWnd:%d\t%s\n",items[i].hWnd,items[i].path);
//			if(items[i].maximized){
//				ShowWindow(items[i].hWnd,SW_SHOWMAXIMIZED);
//			}
//			Sleep(250);
//			//SetWindowText(item.hWnd,"solt item");
//			//CloseHandle(target.hWnd);
//		}
		return 0;
	}
	return 1;
}
void ShowItemList(QuickSlot slot,HWND list){
	int i;
	int listCount=SendMessage(list,LB_GETCOUNT,0,0);
	
	for(i=listCount;i>=0;i--){
		SendMessage(list,LB_DELETESTRING,i,0);
//		if(SendMessage(list,LB_DELETESTRING,i,0)==-1){
//			//printf("%s\n",strerror(errno));
//		}
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
