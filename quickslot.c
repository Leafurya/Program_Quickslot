#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>



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
		DWORD pid;
		HWND hWnd;
	}Target;
BOOL CALLBACK GetHwndProc(HWND hWnd,LPARAM lParam){
	//printf("%d\n",hWnd);
	Target *target=(Target *)lParam;
	DWORD pID;
	
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
	
	GetWindowThreadProcessId(hWnd,&pID);
	printf("pID: %d\ttarget->pid:%d\n",pID,target->pid);
	if(pID==target->pid){
		target->hWnd=hWnd;
		printf("target->hWnd:%d\n",target->hWnd);
		
		return FALSE;
	}
	
	return TRUE;
}
char SpreadQuickslot(QuickSlot slot){
	int i;
	STARTUPINFO si={0,};
	PROCESS_INFORMATION pi={0,};
	Target target;
	Item item;
	RECT rect;
	
	if(slot.itemCount!=0){
		for(i=0;i<slot.itemCount;i++){
			item=slot.item[i];
			memset(&si,0,sizeof(STARTUPINFO));
			si.cb=sizeof(STARTUPINFO);
			si.dwX=item.xpos;
			si.dwY=item.ypos;
			si.dwFlags=STARTF_USEPOSITION|STARTF_USESHOWWINDOW;
			si.wShowWindow=item.maximized?SW_SHOWMAXIMIZED:SW_SHOW;
			
			CreateProcess( NULL,   // No module name (use command line)
					        item.path,        // Command line
					        NULL,           // Process handle not inheritable
					        NULL,           // Thread handle not inheritable
					        FALSE,          // Set handle inheritance to FALSE
					        0,              // No creation flags
					        NULL,           // Use parent's environment block
					        NULL,           // Use parent's starting directory 
					        &si,            // Pointer to STARTUPINFO structure
					        &pi );           // Pointer to PROCESS_INFORMATION structure
			printf("%d,%d\n",si.dwX,si.dwY);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		return 0;
	}
	return 1;
}
