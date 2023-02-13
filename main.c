#include <windows.h>
#include <doublebuffer.h> 
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <strproc2.h>
#include <ctrlmanager.h>

#include "quickslot.h"
#include "ctrls.h"

#define TIMER_INPUT	0

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);

void SaveCtrlsCommandFunc(WPARAM,LPARAM);

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";

CtrlManager cm;
SaveCtrls sc;

QuickSlot quickslot[KEYCOUNT];

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance
		  ,LPSTR lpszCmdParam,int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst=hInstance;
	
	WndClass.cbClsExtra=0;
	WndClass.cbWndExtra=0;
	WndClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	WndClass.hInstance=hInstance;
	WndClass.lpfnWndProc=(WNDPROC)MainWndProc;
	WndClass.lpszClassName=mainWndClass;
	WndClass.lpszMenuName=NULL;
	WndClass.style=CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	AllocConsole();
	freopen("COIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	hWnd=CreateWindow(mainWndClass,mainWndClass,WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT,CW_USEDEFAULT,500,500,
		  NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);

	while(GetMessage(&Message,0,0,0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	FreeConsole();

	return Message.wParam;
}
	void DrawMainWnd(HDC dbDC,RECT rt){
		TextOut(dbDC,10,10,"hello",5);
	}
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int i,index;
	POINT pos;
	switch(iMessage) {
		case WM_CREATE:
			SendMessage(hWnd,WM_SIZE,0,0);
			
			InitCtrlManager(&cm,&mainRect);
			CreateSaveCtrls(&sc,hWnd,g_hInst);
			RegistCtrlGroup(&cm,&sc,ID_SAVECTRLS,sizeof(SaveCtrls),MoveSaveCtrls);
			SetNowCtrlGroup(&cm,ID_SAVECTRLS);
			
			if(!LoadQuickslot(&quickslot,sizeof(quickslot))){
				memset(quickslot,0,sizeof(quickslot));
			}
			for(index=0;index<KEYCOUNT;index++){
				printf("index:%d===========\n",index);
				for(i=0;i<quickslot[index].itemCount;i++){
					printf("max: %d| (%d,%d) |path: %s\n",quickslot[index].item[i].maximized,quickslot[index].item[i].xpos,quickslot[index].item[i].ypos,quickslot[index].item[i].path);
				}
			}
			SetTimer(hWnd,TIMER_INPUT,1,NULL);
			printf("%d,%d,%d\n",sizeof(HWND),sizeof(DWORD),sizeof(LPARAM));
			return 0;
		case WM_SIZE:
			if(wParam!=SIZE_MINIMIZED){
				GetClientRect(hWnd,&mainRect);
				CallMoveFunc(cm,mainRect);
			}
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hWnd, &ps);
				DoubleBuffer(hdc,mainRect,DrawMainWnd);
			EndPaint(hWnd, &ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam){
				case VK_RETURN:
					SendMessage(hWnd,WM_SIZE,SIZE_MAXIMIZED,0);
					printf("================================\n");
					break;
			}
			return 0;
		case WM_TIMER:
			switch(wParam){
				case TIMER_INPUT:
					if((index=GetSlotIndex())!=-1){
						if(SpreadQuickslot(quickslot[index])){
							printf("empty slot\n");
						}
					}
//					if(GetKeyState(VK_LBUTTON)&0x8000){
//						GetCursorPos(&pos);
//						printf("%d,%d\n",pos.x,pos.y);
//					}
					break;
			}
			return 0;
		case WM_COMMAND:
			switch((int)(LOWORD(wParam)/100)){
				case ID_SAVECTRLS:
					SaveCtrlsCommandFunc(wParam,lParam);
					break;
			}
			return 0;
		case WM_DESTROY:
			DestroyCtrlManager(&cm);
			PostQuitMessage(0);
			return 0;
	}
	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}
	BOOL IsFilteredWindow(char *name){
		const char *windowFilter[]={"SystemSettings.exe","ApplicationFrameHost.exe","TextInputHost.exe","Program_Quickslot.exe"};
		int i;
		for(i=0;i<(sizeof(windowFilter)/sizeof(char *));i++){
			if(!strcmp(name,windowFilter[i])){
				return TRUE;
			}
		}
		return FALSE;
	}
BOOL CALLBACK EnumWindowsProc(HWND hWnd,LPARAM lParam){
	//printf("%d\n",hWnd);
	QuickSlot *lpQuickslot=(QuickSlot *)lParam;
	
	BOOL isVisible=IsWindowVisible(hWnd);
	DWORD exStyle=GetWindowLong(hWnd,GWL_EXSTYLE);
	BOOL isAppWindow=(exStyle&WS_EX_APPWINDOW);
	BOOL isToolWindow=(exStyle&WS_EX_TOOLWINDOW);
	BOOL isOwned=GetWindow(hWnd,GW_OWNER)?TRUE:FALSE;
	
	WINDOWINFO wInfo;
	DWORD pID;
	HANDLE hProc;
	char path[1024]={0};
	STRING str;
	int i;
	char *progName;
	RECT rect;
	
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
		str=Split(path,'\\');
		progName=str.strings[str.size-1];
		if(!IsFilteredWindow(progName)){
			GetWindowInfo(hWnd,&wInfo);
			//SendMessage(hWnd,WM_SIZE,SIZE_MINIMIZED,0);
			for(i=0;i<lpQuickslot->itemCount;i++){
				if(!strcmp(lpQuickslot->item[i].path,path)){
					DeleteString(&str);
					CloseHandle(hProc);
					return TRUE;
				}
			}
			GetWindowRect(hWnd,&rect);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(path,IsZoomed(hWnd),wInfo.rcWindow);
			printf("(%d,%d)\n",rect.left,rect.top);
		}
		DeleteString(&str);
		CloseHandle(hProc);
	}
	
	return TRUE;
}
void SaveCtrlsCommandFunc(WPARAM wParam,LPARAM lParam){
	int index=wParam-SAVECTRLS_BT_ORIGIN;
	int i; 
	
	printf("index:%d\n",index);
	if(quickslot[index].itemCount==0){
		EnumWindows(EnumWindowsProc,(LPARAM)&quickslot[index]);
		for(i=0;i<quickslot[index].itemCount;i++){
			printf("max: %d| (%d,%d) |path: %s\n",quickslot[index].item[i].maximized,quickslot[index].item[i].xpos,quickslot[index].item[i].ypos,quickslot[index].item[i].path);
		}
		printf("done: %d\n",SaveQuickslot(quickslot,sizeof(quickslot)));
	}
	else{
		printf("no place\n");
	}
}
