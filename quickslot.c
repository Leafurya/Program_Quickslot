#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>
#include <strproc2.h>
#include <shellapi.h>
#include <psapi.h>
#include <time.h>

#include "list.h"
#include "progressbar.h"
#include "thread.h"

QuickSlot *originSlotAdr;
List list;

BOOL ComparePreWindows(char *str,List *list){
	char *data; 
	//printf("GetCurStr : %s\n\n",str);
	ReturnToHead(list);
	while(MoveNext(list)){
		data=(char *)GetCurData(*list);
		//printf("GetCurData: %s\n",data);
		if(!strcmp(str,data)){
			free(str);
			return 1;
		}
	}
	AddData(list,str);
	return 0;
}
BOOL IsUselessWindow(char *name){
	const char *windowFilter[]={"SystemSettings.exe","ApplicationFrameHost.exe","TextInputHost.exe","Program_Quickslot.exe"};
	int i;
	for(i=0;i<(sizeof(windowFilter)/sizeof(char *));i++){
		if(!strcmp(name,windowFilter[i])){
			return TRUE;
		}
	}
	return FALSE;
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
BOOL GetProcessPath(char *path,HWND hWnd){
	DWORD pID;
	HANDLE hProc;
	
	if(FilterWindow(hWnd)){
		return FALSE;
	}
	
	GetWindowThreadProcessId(hWnd,&pID);
	hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
	if(hProc){
		GetModuleFileNameEx(hProc,NULL,path,1024);
		CloseHandle(hProc);
		return TRUE;
	}
	CloseHandle(hProc);
	return FALSE;
}

BOOL CALLBACK SavePreWindows(HWND hWnd,LPARAM lParam){
	char path[1024]={0};
	List *list=(List *)lParam;
	if(GetProcessPath(path,hWnd)){
		//tpath=GetString("%d\"%s\"",hWnd,path);
		AddData(list,(char *)GetString("%d\"%s\"\0",hWnd,path));
	}
	return TRUE;
}
BOOL CALLBACK GetOpenedWindowProc(HWND hWnd,LPARAM lParam){
	QuickSlot *lpQuickslot=(QuickSlot *)lParam;
	char path[1024]={0};
	char tpath[1024]={0};
	STRING str;
	char *progName;
	WINDOWINFO wInfo;
	
	if(GetProcessPath(path,hWnd)){
		str=Split(path,'\\');
		progName=str.strings[str.size-1];
		if(!IsUselessWindow(progName)){
			GetWindowInfo(hWnd,&wInfo);
			sprintf(tpath,"\"%s\"",path);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(tpath,progName,NULL,IsZoomed(hWnd),wInfo.rcWindow,hWnd,1);
			//printf("lpQuickslot: %p\n",lpQuickslot);
			//printf("maxi:%d\thWnd:%d\tparam: %s|path:%s\n",slot[i].item[j].maximized,slot[i].item[j].hWnd,slot[i].item[j].parameter,slot[i].item[j].path);
		}
		DeleteString(&str);
	}
	return TRUE;
}
BOOL CALLBACK GetHwndProc(HWND hWnd,LPARAM lParam){
	Item *target=(Item *)lParam;
	char *compareData;
	char path[1024]={0};
	char tpath[1024]={0};
	
	if(GetProcessPath(path,hWnd)){
		compareData=GetString("%d\"%s\"\0",hWnd,path);
		if(ComparePreWindows(compareData,&list)){
			return TRUE;
		}
		
		//printf("\n");
		//ShowAllData(&list);
		//printf("\n");

		sprintf(tpath,"\"%s\"",path);
		if(!strcmp(tpath,target->path)){
			target->hWnd=hWnd;
			if(IsZoomed(hWnd)){
				ShowWindow(hWnd,SW_SHOWNORMAL);
			}
			//printf("new window %s\n",compareData);
			return FALSE;
		}
	}
	return TRUE;
}


//void ShowSlotData(QuickSlot *slot){
//	int i,j;
//	printf("ShowSlotData:==========\n");
//	for(i=0;i<KEYCOUNT;i++){
//		for(j=0;j<slot[i].itemCount;j++){
//			printf("maxi:%d\thWnd:%d\tparam: %s|path:%s\n",slot[i].item[j].maximized,slot[i].item[j].hWnd,slot[i].item[j].parameter,slot[i].item[j].path);
//			printf("(%d,%d)\n",slot[i].item[j].xpos,slot[i].item[j].ypos);
//		}
//		printf("%d-%s-----------------------\n",i,slot[i].slotName);
//	}
//}
int GetSlotIndex(int key){
	int i;
	static char toggle=1;
	char isDesktop=1;
	char className[128]={0};
	
	//printf("GetAsyncKeyState(VK_SHIFT): %x|GetAsyncKeyState(VK_CONTROL): %x\n",GetAsyncKeyState(VK_SHIFT),GetAsyncKeyState(VK_CONTROL));
	
	if((GetAsyncKeyState(VK_SHIFT)&0x8000)&&(GetAsyncKeyState(VK_CONTROL)&0x8000)){
		for(i=0;i<KEYCOUNT;i++){
			if((GetAsyncKeyState(VK_F1+i)&0x8000)&&toggle){
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
	
	fseek(file,1,SEEK_SET);
	fread(pQuickslot,size,1,file);
	
	fclose(file);
	
	return 1;
}
//char SaveQuickslot(QuickSlot *pQuickslot,int size){
//	FILE *file=fopen("data/slot","ab");
//	
//	if(file==NULL){
//		return 0;
//	}
//	fseek(file,1,SEEK_SET);
//	fwrite(pQuickslot,size,1,file);
//	
//	fclose(file);
//	return 1;
//}
char SaveQuickslot(QuickSlot *pQuickslot,int size){
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
			newData[i].item[j].detecting=1;
			
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
	
	if(NOW_DATA_VERSION==version){
		fclose(file);
		return;
	}
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
			SaveQuickslot(newData,sizeof(QuickSlot)*KEYCOUNT);
			break;
	}
	fclose(file);
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
	char StopSpread(char *blockVar,List *list){
		int i;
		while(*blockVar){
			if(*blockVar==-1){
				FreeList(list);
				printf("stop thread\n");
				return 1;
			}
		}
		return 0;
	}
	char ExecuteProcess(Item item){
		HANDLE target;
		int timeout=0;
		
		SHELLEXECUTEINFO seinfo;
		ZeroMemory(&seinfo,sizeof(SHELLEXECUTEINFO));
		
		seinfo.cbSize=sizeof(SHELLEXECUTEINFO);
		seinfo.lpFile=item.path;
		seinfo.lpParameters=item.parameter;
		seinfo.nShow=SW_SHOW;
		seinfo.fMask=SEE_MASK_NOCLOSEPROCESS;
		
		if(!ShellExecuteEx(&seinfo)){
			printf("error: %d\n",GetLastError());
			return EXECUTE_FAIL;
		}
		
		CloseHandle(seinfo.hProcess);
		return 0;
	}
	char GetItemWinHandle(Item *item,char *blockVar,List *list){
		int timeout=0;
		clock_t start,end;
		
		do{
			if(StopSpread(blockVar,list)){
				return 1;
			}
			EnumWindows(GetHwndProc,(LPARAM)item);
			if(timeout>=1000){
				item->hWnd=0;
				return FINDING_FAIL;
			}
			timeout++;
			Sleep(1);
		}while(!item->hWnd);
		return 0;
	}
	char MoveItemWindow(Item item,char *blockVar,List *list){
		if(StopSpread(blockVar,list)){
			return 1;
		}
		//printf("%d%s\n",items[i].hWnd,items[i].path);
		if(item.hWnd){
			if(!MoveWindow(item.hWnd,item.xpos,item.ypos<0?100:item.ypos,item.w,item.h,TRUE)){
				return -1;
			}
			ShowWindow(item.hWnd,SW_NORMAL);
			if(item.maximized){
				ShowWindow(item.hWnd,SW_SHOWMAXIMIZED);
			}
		}
		//printf("move %d %s %s\n",item.hWnd,item.path,item.parameter);
		
		Sleep(100);
		return 0;
	}
	unsigned __stdcall FindWindowThread(void *arg){
		void **sharedData=arg;
		char *blockVar=sharedData[0];
		List *list=sharedData[1];
		Item *item=sharedData[2];
		char **status=sharedData[3];
		
		switch(GetItemWinHandle(item,blockVar,list)){
			case 1:
				return -1;
			case FINDING_FAIL:
				SetNowLog(GetString("not found %s",item->name));
				if(!(*status)){
					*status=GetString("���α׷� ���� ����: %s\n",item->name);
				}
				return 1;
			default:
				SetNowLog(GetString("move %s",item->name));
				switch(MoveItemWindow(*item,blockVar,list)){
					case 1:
						return -1;
					case -1:
						if(!(*status)){
							*status=GetString("���α׷� ���ġ ����: %s\n",item->name);
						}
						break;
				}
				StepBar();
				break;
		}
	}
char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex,char *status[ITEM_MAXSIZE]){
	int i;
	QuickSlot slot=pOriginSlot[slotIndex];
	Item *items=pOriginSlot[slotIndex].item;
	
	originSlotAdr=pOriginSlot;
	static char blockVar=0; //controled by progressbar proc
	void **sharedData;
	HANDLE *threadHandles;
	DWORD threadResult;
	
	if(slot.itemCount!=0){
		threadHandles=(HANDLE *)malloc(sizeof(HANDLE)*slot.itemCount);
		blockVar=0;
		SetBlockVar(&blockVar);
		InitList(&list);
		EnumWindows(SavePreWindows,(LPARAM)&list);
		
		for(i=0;i<slot.itemCount;i++){
			if(StopSpread(&blockVar,&list)){
				return -1;
			}
			items[i].hWnd=0;
			
			SetNowLog(GetString("execute %s",items[i].name));
			if(ExecuteProcess(items[i])){
				status[i]=GetString("���α׷� ���� ����: %s\n",items[i].name);
			}
			if(items[i].detecting){
				sharedData=(void **)malloc(sizeof(void *)*4);
				sharedData[0]=&blockVar;
				sharedData[1]=&list;
				sharedData[2]=&items[i];
				sharedData[3]=&status[i];
				
				threadHandles[i]=StartThread(FindWindowThread,sharedData);
//				Sleep(200);
//				SetNowLog(GetString("finding %s",items[i].name));
//				switch(GetItemWinHandle(&items[i],&blockVar,&list)){
//					case 1:
//						return -1;
//					case FINDING_FAIL:
//						SetNowLog(GetString("not found %s",items[i].name));
//						if(!status[i]){
//							status[i]=GetString("���α׷� ���� ����: %s\n",items[i].name);
//						}
//						break;
//					default:
//						SetNowLog(GetString("found %s",items[i].name));
//						break;
//				}
			}
			StepBar();
		}
		for(i=0;i<slot.itemCount;i++){
			WaitForSingleObject(threadHandles[i],threadResult);
			
		}
//		for(i=0;i<slot.itemCount;i++){
//			SetNowLog(GetString("move %s",items[i].name));
//			switch(MoveItemWindow(items[i],&blockVar,&list)){
//				case 1:
//					return -1;
//				case -1:
//					if(!status[i]){
//						status[i]=GetString("���α׷� ���ġ ����: %s\n",items[i].name);
//					}
//					break;
//			}
//			StepBar();
//		}
		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
		FreeList(&list);
		return 0;
	}
	return 1;
}
//char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex,char *status[ITEM_MAXSIZE]){
//	int i;
//	QuickSlot slot=pOriginSlot[slotIndex];
//	Item *items=pOriginSlot[slotIndex].item;
//	
//	originSlotAdr=pOriginSlot;
//	static char blockVar=0; //controled by progressbar proc
//	void **sharedData;
//	
//	if(slot.itemCount!=0){
//		blockVar=0;
//		SetBlockVar(&blockVar);
//		InitList(&list);
//		EnumWindows(SavePreWindows,(LPARAM)&list);
//		
//		for(i=0;i<slot.itemCount;i++){
//			if(StopSpread(&blockVar,&list)){
//				return -1;
//			}
//			items[i].hWnd=0;
//			
//			SetNowLog(GetString("execute %s",items[i].name));
//			if(ExecuteProcess(items[i])){
//				status[i]=GetString("���α׷� ���� ����: %s\n",items[i].name);
//			}
//			if(items[i].detecting){
//				Sleep(200);
//				SetNowLog(GetString("finding %s",items[i].name));
//				switch(GetItemWinHandle(&items[i],&blockVar,&list)){
//					case 1:
//						return -1;
//					case FINDING_FAIL:
//						SetNowLog(GetString("not found %s",items[i].name));
//						if(!status[i]){
//							status[i]=GetString("���α׷� ���� ����: %s\n",items[i].name);
//						}
//						break;
//					default:
//						SetNowLog(GetString("found %s",items[i].name));
//						break;
//				}
//			}
//			StepBar();
//		}
//		for(i=0;i<slot.itemCount;i++){
//			SetNowLog(GetString("move %s",items[i].name));
//			switch(MoveItemWindow(items[i],&blockVar,&list)){
//				case 1:
//					return -1;
//				case -1:
//					if(!status[i]){
//						status[i]=GetString("���α׷� ���ġ ����: %s\n",items[i].name);
//					}
//					break;
//			}
//			StepBar();
//		}
//		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
//		FreeList(&list);
//		return 0;
//	}
//	return 1;
//}

void ShowItemList(Item *item,int itemCount,HWND list){
	int i;
	int listCount=SendMessage(list,LB_GETCOUNT,0,0);
	
	for(i=listCount;i>=0;i--){
		SendMessage(list,LB_DELETESTRING,i,0);
	}
	for(i=0;i<itemCount;i++){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)item[i].name);
		//printf("item[i].name: %s\n",item[i].name);
	}
	
	if(!i){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)"EMPTY");
	}
}
void ShowItemInfo(char *name,Item *item,HWND stText,HWND cbDetecting){
	char info[2048]={0,};
	
	if(item){
		sprintf(info,"%s\n\n���:%s\n\n�Ű�����:%s",strlen(name)?name:"-",item->path,strlen(item->parameter)?item->parameter:"-");//strlen(name)?name:"-"
	}
	else{
		sprintf(info,"%s\n\n���:-\n\n�Ű�����:-",strlen(name)?name:"-");//strlen(name)?name:"-"
	}
	SetWindowText(stText,info);
	if(!item){
		SendMessage(cbDetecting,BM_SETCHECK,BST_UNCHECKED,0);
		EnableWindow(cbDetecting,FALSE);
		return;
	}
	EnableWindow(cbDetecting,TRUE);
	if(item->detecting) {
		SendMessage(cbDetecting,BM_SETCHECK,BST_CHECKED,0);
	}
	else{
		SendMessage(cbDetecting,BM_SETCHECK,BST_UNCHECKED,0);
	}
}
void CloseSlot(QuickSlot *slot){
	int i;
	for(i=0;i<slot->itemCount;i++){
		PostMessage(slot->item[i].hWnd,WM_CLOSE,0,0);
		slot->item[i].hWnd=0;
	}
}
char IsSlotOpened(QuickSlot slot){
	int j;
	for(j=0;j<slot.itemCount;j++){
		if(slot.item[j].hWnd){
			return 1;
		}
	}
	return 0;
}
void ForegroundSlot(QuickSlot slot){
	int i;
	Item *items;
	DWORD myThread;
	DWORD targetThread;
	HWND hWnd;
	
	items=slot.item;
	myThread=GetCurrentThreadId();
	for(i=0;i<slot.itemCount;i++){
		ShowWindow(items[i].hWnd,SW_MINIMIZE);
	}
	for(i=0;i<slot.itemCount;i++){
		hWnd=items[i].hWnd;
		if(IsIconic(hWnd)){
			ShowWindow(hWnd,SW_NORMAL);
		}
		targetThread=GetWindowThreadProcessId(hWnd,NULL);
		if(AttachThreadInput(myThread,targetThread,TRUE)){
			SetForegroundWindow(hWnd);
			BringWindowToTop(hWnd);
			if(items[i].maximized){
				ShowWindow(hWnd,SW_SHOWMAXIMIZED);
			}
			SetActiveWindow(hWnd);
			AttachThreadInput(myThread,targetThread,FALSE);
		}
		//tpathtpathStopFlash(hWnd);
		Sleep(50);
	}
	printf("foregrounded\n");
}
