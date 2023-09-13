#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>
#include <strproc2.h>
#include <shellapi.h>
#include <psapi.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "list.h"
#include "progressbar.h"
#include "thread.h"
#include "log.h"

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
			GetWindowText(hWnd,path,sizeof(path));
//			printf("%s\t%s\n",path,tpath);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(tpath,progName,NULL,IsZoomed(hWnd),wInfo.rcWindow,hWnd,1,path);
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
//			printf("target->path: %s\n",target->path);
			target->hWnd=hWnd;
			AddData(&list,compareData);
			if(IsZoomed(hWnd)){
				ShowWindow(hWnd,SW_SHOWNORMAL);
			}
//			printf("find by path: %d\n",hWnd);
			//printf("new window %s\n",compareData);
			return FALSE;
		}
		else{
//			GetWindowText(hWnd,path,sizeof(path));
			target->hWnd=FindWindow(NULL,target->winTitle);
//			printf("%s\n",tpath);
//			printf("hWnd: %d\ntWnd: %d\n",hWnd,target->hWnd);
			if(target->hWnd==hWnd){
				AddData(&list,compareData);
				if(IsZoomed(hWnd)){
					ShowWindow(hWnd,SW_SHOWNORMAL);
				}
//				printf("find by title: %d\n",hWnd);
				return FALSE;
			}
			target->hWnd=0;
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
	FILE *file;
	char path[MAX_PATH];
	
	
	GetModuleFileName(NULL,path,sizeof(path));
	path[strlen(path)-strlen("program_quickslot.exe")]=0;
	sprintf(path,"%sdata\\slot",path);
	printf("path: %s\n",path);
	
	file=fopen(path,"rb");
	
	if(file==NULL){
		printf("%s\n",strerror(errno));
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
	FILE *file;
	char path[MAX_PATH];
	char version=NOW_DATA_VERSION;
	
	GetModuleFileName(NULL,path,sizeof(path));
	path[strlen(path)-strlen("program_quickslot.exe")]=0;
	sprintf(path,"%sdata\\slot",path);
	printf("path: %s\n",path);
	
	file=fopen("data/slot","wb");
	
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
			printf("(%d,%d) detecting: %d title:%s\n",slot[i].item[j].xpos,slot[i].item[j].ypos,slot[i].item[j].detecting,slot[i].item[j].winTitle);
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
QuickSlot *ChangeToV2(QuickSlotV1 *oldData){
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
			newData[i].item[j].detecting=oldData[i].item[j].detecting;
			
			sprintf(newData[i].item[j].path,"%s",oldData[i].item[j].path);
			sprintf(newData[i].item[j].name,"%s",oldData[i].item[j].name);
			sprintf(newData[i].item[j].parameter,"%s",oldData[i].item[j].parameter);
			sprintf(newData[i].item[j].winTitle,"");
		}
	}
	return newData;
}
void CheckVersion(){
	FILE *file=fopen("data/slot","rb");
	char version;
	char tVer;
	void *oldData;
	QuickSlot *newData;
	int i,j;
	int fileSize;
	
	if(!file){
		return;
	}
	fseek(file,0,SEEK_END);
//	printf("file size: %d\n",ftell(file));
	if(!ftell(file)){
		fwrite(0,sizeof(QuickSlot),1,file);
		fclose(file);
		return;
	}
	rewind(file);
	fread(&version,1,1,file);
	
	printf("data version: %d\n",version);
	if(NOW_DATA_VERSION==version){
		fclose(file);
		return;
	}
	switch(version){
		case 1:
			oldData=(QuickSlotV1 *)malloc(sizeof(QuickSlotV1)*KEYCOUNT);
			fread(oldData,1,sizeof(QuickSlotV1)*KEYCOUNT,file);
			break;
		default:
			oldData=(QuickSlotV0 *)malloc(sizeof(QuickSlotV0)*KEYCOUNT);
			rewind(file);
			fread(oldData,1,sizeof(QuickSlotV0)*KEYCOUNT,file);
			break;
	}
	fclose(file);
	
	for(tVer=version;NOW_DATA_VERSION!=tVer;){
		tVer++;
		switch(tVer){
			case 1:
				newData=ChangeToV1(oldData);
//				LogMessage(GetString("데이터 버전 업데이트(V%d -> V1)",version));
				break;
			case 2:
				newData=ChangeToV2(oldData);
//				LogMessage(GetString("데이터 버전 업데이트(V%d -> V2)",version));
				break;
		}
		free(oldData);
		oldData=newData;
	}
//	switch(version){
//		case 1:
//			oldData=(QuickSlotV1 *)malloc(sizeof(QuickSlotV1)*KEYCOUNT);
//			fread(oldData,1,sizeof(QuickSlotV1)*KEYCOUNT,file);
//			break;
//		default:
//			oldData=(QuickSlotV0 *)malloc(sizeof(QuickSlotV0)*KEYCOUNT);
//			rewind(file);
//			fread(oldData,1,sizeof(QuickSlotV0)*KEYCOUNT,file);
//			break;
//	}
//	fclose(file);
//	switch(NOW_DATA_VERSION){
//		case 1:
//			newData=ChangeToV1(oldData);
//			LogMessage(GetString("데이터 버전 업데이트(V%d -> V1)",version));
//			break;
//		case 2:
//			newData=ChangeToV2(oldData);
//			LogMessage(GetString("데이터 버전 업데이트(V%d -> V2)",version));
//			break;
//	}
	LogMessage(GetString("데이터 버전 업데이트(V%d -> V%d)",version,tVer));
	SaveLog();
//	ShowSlotData(newData);
//	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	SaveQuickslot(newData,sizeof(QuickSlot)*KEYCOUNT);
	free(newData);
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
		if(!blockVar){
			return 0;
		}
		while(*blockVar){
			if(*blockVar==-1){
				LogMessage(GetString("작업 중단"));
				FreeList(list);
//				printf("stop thread\n");
				return 1;
			}
		}
		return 0;
	}
	char ExecuteProcess(Item item){
//		LogMessage(GetString("프로그램 실행 중(%s)",item.winTitle));
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
//			printf("error: %d\n",GetLastError());
			LogMessage(GetString("프로그램 실행 실패(%s)|%s",item.winTitle,strerror(errno)));
			return EXECUTE_FAIL;
		}
		LogMessage(GetString("프로그램 실행 성공(%s)",item.winTitle));
		CloseHandle(seinfo.hProcess);
		return 0;
	}
	char GetItemWinHandle(Item *item,char *blockVar,List *list){
		int timeout=0;
		clock_t start,end;
//		LogMessage(GetString("프로그램 감지 중(%s)",item->winTitle));
		do{
//			printf("finding %d %s\n",item->hWnd,item->path);
			if(StopSpread(blockVar,list)){
//				printf("stop thread %s\n",item->path);
				return 1;
			}
			EnumWindows(GetHwndProc,(LPARAM)item);
//			printf("1\n");
			if(timeout>=700){
//				printf("not found %s\n",item->name);
				LogMessage(GetString("프로그램 감지 실패(%s)|timeout",item->winTitle));
				item->hWnd=0;
				return FINDING_FAIL;
			}
//			printf("2\n");
			timeout++;
//			printf("3\n");
			Sleep(1);
		}while(!item->hWnd);
		
		LogMessage(GetString("프로그램 감지 성공(%s)",item->winTitle));
//		printf("found %s\n",item->name);
		return 0;
	}
	char MoveItemWindow(Item item,char *blockVar,List *list){
		RECT pos;
		char posError;
		if(StopSpread(blockVar,list)){
			return 1;
		}
//		LogMessage(GetString("프로그램 배치 중(%s)",item.winTitle));
		//printf("%d%s\n",items[i].hWnd,items[i].path);
		if(item.hWnd){
//			printf("start x%d, y:%d\n",item.xpos,item.ypos,item.w,item.h);
			if(!MoveWindow(item.hWnd,item.xpos,item.ypos,item.w,item.h,FALSE)){
				LogMessage(GetString("프로그램 배치 실패(%s)",item.winTitle));
				return -1;
			}
			ShowWindow(item.hWnd,SW_NORMAL);
			if(item.maximized){
				ShowWindow(item.hWnd,SW_SHOWMAXIMIZED);
			}
			LogMessage(GetString("프로그램 배치 성공(%s)|X%d Y%d W%d H%d",item.winTitle,item.xpos,item.ypos,item.w,item.h));
		}
		//printf("move %d %s %s\n",item.hWnd,item.path,item.parameter);
		
//		Sleep(100);
		return 0;
	}
//	unsigned __stdcall FindWindowThread(void *arg){
//		void **sharedData=arg;
//		char *blockVar=sharedData[0];
//		List *list=sharedData[1];
//		Item *item=sharedData[2];
//		char **status=sharedData[3];
//		
//		char findResult=GetItemWinHandle(item,blockVar,list);
//		printf("find %s %d\n",item->path,item->hWnd);
//		
////		printf("find thread for %s\n",item->path);
//		switch(findResult){
//			case 1:
////				printf("return 1\n");
//				return -1;
//			case FINDING_FAIL:
////				printf("return FINDING_FAIL\n");
//				SetNowLog(GetString("not found %s",item->name));
//				if(!(*status)){
//					*status=GetString("프로그램 감지 실패: %s\n",item->name);
//				}
//				return 1;
//			default:
////				printf("return default\n");
//				SetNowLog(GetString("move %s",item->name));
//				switch(MoveItemWindow(*item,blockVar,list)){
//					case 1:
//						return -1;
//					case -1:
//						printf("move %s\n",item->name);
//						if(!(*status)){
//							*status=GetString("프로그램 재배치 실패: %s\n",item->name);
//						}
//						break;
//				}
//				
//				break;
//		}
//		StepBar();
//		return 0;
//	}
//char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex,char *status[ITEM_MAXSIZE]){
//	int i;
//	QuickSlot slot=pOriginSlot[slotIndex];
//	Item *items=pOriginSlot[slotIndex].item;
//	
//	originSlotAdr=pOriginSlot;
//	static char blockVar=0; //controled by progressbar proc
//	void **sharedData;
//	HANDLE *threadHandles;
//	DWORD threadResult;
//	
//	if(slot.itemCount!=0){
//		threadHandles=(HANDLE *)malloc(sizeof(HANDLE)*slot.itemCount);
//		blockVar=0;
//		SetBlockVar(&blockVar);
//		InitList(&list);
//		EnumWindows(SavePreWindows,(LPARAM)&list);
//		ShowAllData(&list);
//		
//		for(i=0;i<slot.itemCount;i++){
//			if(StopSpread(&blockVar,&list)){
//				return -1;
//			}
//			items[i].hWnd=0;
//			
//			SetNowLog(GetString("execute %s",items[i].name));
//			status[i]=NULL;
//			if(ExecuteProcess(items[i])){
//				status[i]=GetString("프로그램 실행 실패: %s\n",items[i].name);
//			}
//			if(items[i].detecting&&!status[i]){
//				sharedData=(void **)malloc(sizeof(void *)*4);
//				sharedData[0]=&blockVar;
//				sharedData[1]=&list;
//				sharedData[2]=&items[i];
//				sharedData[3]=&status[i];
//				
//				threadHandles[i]=StartThread(FindWindowThread,sharedData);
////				Sleep(200);
////				SetNowLog(GetString("finding %s",items[i].name));
////				switch(GetItemWinHandle(&items[i],&blockVar,&list)){
////					case 1:
////						return -1;
////					case FINDING_FAIL:
////						SetNowLog(GetString("not found %s",items[i].name));
////						if(!status[i]){
////							status[i]=GetString("프로그램 감지 실패: %s\n",items[i].name);
////						}
////						break;
////					default:
////						SetNowLog(GetString("found %s",items[i].name));
////						break;
////				}
//			}
//			StepBar();
//		}
//		for(i=0;i<slot.itemCount;i++){
//			WaitForSingleObject(threadHandles[i],INFINITE);
//			printf("%d thread done %d\n",i,threadResult);
//		}
//		for(i=0;i<slot.itemCount;i++){
//			free(sharedData);
//		}
////		for(i=0;i<slot.itemCount;i++){
////			SetNowLog(GetString("move %s",items[i].name));
////			switch(MoveItemWindow(items[i],&blockVar,&list)){
////				case 1:
////					return -1;
////				case -1:
////					if(!status[i]){
////						status[i]=GetString("프로그램 재배치 실패: %s\n",items[i].name);
////					}
////					break;
////			}
////			StepBar();
////		}
//		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
//		FreeList(&list);
//		return 0;
//	}
//	return 1;
//}
Item OpenItem(char *path,char *param,char *winTitle){
	InitList(&list);
	EnumWindows(SavePreWindows,(LPARAM)&list);
	Item item;
	char *name;
	STRING str;
	RECT rect;
	
//	printf("path: %s\nparam: %s\n",path,param);
	str=Split(path,'\\');
	name=str.strings[str.size-1];
	if(name[strlen(name)-1]=='\"'){
		name[strlen(name)-1]=0;
	}
//	printf("%s\n",str.strings[str.size-1]);
	item=CreateItem(path,name,param,0,rect,0,1,winTitle);
	if(ExecuteProcess(item)){
//		printf("프로그램 실행 실패\n");
	}
	Sleep(200);
	GetItemWinHandle(&item,NULL,&list);
//	SetNowLog(GetString("finding %s",items[i].name));
//	switch(GetItemWinHandle(&item,NULL,&list)){
//		case 1:
//			return -1;
//		case FINDING_FAIL:
////			SetNowLog(GetString("not found %s",items[i].name));
////			if(!status[i]){
////				status[i]=GetString("프로그램 감지 실패(timeout): %s\n",items[i].name);
////			}
//			break;
//		default:
////			SetNowLog(GetString("found %s",items[i].name));
//			break;
//	}
//	printf("%d\n",item.hWnd);
	FreeList(&list);
	DeleteString(&str);
	return item;
}
char SpreadQuickslot(QuickSlot *pOriginSlot,int slotIndex,char *status[ITEM_MAXSIZE]){
	int i;
	QuickSlot slot=pOriginSlot[slotIndex];
	Item *items=pOriginSlot[slotIndex].item;
	
	originSlotAdr=pOriginSlot;
	static char blockVar=0; //controled by progressbar proc
	void **sharedData;
	
	if(slot.itemCount!=0){
		blockVar=0;
		SetBlockVar(&blockVar);
		InitList(&list);
		EnumWindows(SavePreWindows,(LPARAM)&list);
		
		for(i=0;i<slot.itemCount;i++){
			if(StopSpread(&blockVar,&list)){
				return -1;
			}
			items[i].hWnd=0;
			
//			SetNowLog(GetString("execute %s",items[i].name));
			if(ExecuteProcess(items[i])){
				status[i]=GetString("프로그램 실행 실패: %s\n",items[i].name);
			}
			if(items[i].detecting){
				Sleep(200);
//				SetNowLog(GetString("finding %s",items[i].name));
//				LogMessage(GetString("프로그램 감지 중(%s)",items[i].winTitle));
				switch(GetItemWinHandle(&items[i],&blockVar,&list)){
					case 1:
						return -1;
					case FINDING_FAIL:
//						LogMessage(GetString("프로그램 감지 실패(%s)|timeout",items[i].winTitle));
//						SetNowLog(GetString("not found %s",items[i].name));
						if(!status[i]){
							status[i]=GetString("프로그램 감지 실패(timeout): %s\n",items[i].name);
						}
						break;
				}
				
			}
			else{
				LogMessage(GetString("프로그램 감지 안함(%s)",items[i].winTitle));
			}
			StepBar();
		}
		for(i=0;i<slot.itemCount;i++){
//			SetNowLog(GetString("move %s",items[i].name));
			switch(MoveItemWindow(items[i],&blockVar,&list)){
				case 1:
					return -1;
				case -1:
//					LogMessage(GetString("프로그램 배치 실패(%s)",items[i].winTitle);
					if(!status[i]){
						status[i]=GetString("프로그램 배치 실패: %s\n",items[i].name);
					}
					break;
			}
			Sleep(10);
			StepBar();
		}
//		printf("title: %s|%d\n",items[i].winTitle,strlen(items[i].winTitle));
//		
//		printf("after title: %s|%d\n",items[i].winTitle,strlen(items[i].winTitle));
		memcpy(pOriginSlot[slotIndex].item,items,sizeof(pOriginSlot[slotIndex].item));
		FreeList(&list);
		return 0;
	}
	return 1;
}

void ShowItemList(Item *item,int itemCount,HWND list){
	int i;
	int listCount=SendMessage(list,LB_GETCOUNT,0,0);
	char str[1024]={0};
	
	for(i=listCount;i>=0;i--){
		SendMessage(list,LB_DELETESTRING,i,0);
	}
	for(i=0;i<itemCount;i++){
		sprintf(str,"(%s)%s\0",item[i].name,item[i].winTitle);
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)str);
		//printf("item[i].name: %s\n",item[i].name);
	}
	
	if(!i){
		SendMessage(list,LB_ADDSTRING,0,(LPARAM)"EMPTY");
	}
}
void ShowItemInfo(char *name,Item *item,HWND stText,HWND cbDetecting){
	char info[2048]={0,};
	
	if(item){
		sprintf(info,"%s\n\n경로:%s\n\n매개변수:%s",strlen(name)?name:"-",item->path,strlen(item->parameter)?item->parameter:"-");//strlen(name)?name:"-"
	}
	else{
		sprintf(info,"%s\n\n경로:-\n\n매개변수:-",strlen(name)?name:"-");//strlen(name)?name:"-"
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
//	printf("foregrounded\n");
}
