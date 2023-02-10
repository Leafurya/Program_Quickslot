#include <windows.h>
#include <doublebuffer.h> 
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <strproc2.h>

#include "quickslot.h"

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);

void GetProcessPath();
void GetOpenedWindow();

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";

QuickSlot quickSlot[VK_F24-VK_F1+1];

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
	HWND hWin;
	WINDOWINFO wInfo;
	int i;
	DWORD pID;
	switch(iMessage) {
		case WM_CREATE:
			SendMessage(hWnd,WM_SIZE,0,0);
			EnumWindows(EnumWindowsProc,(LPARAM)NULL);
			return 0;
		case WM_SIZE:
			if(wParam!=SIZE_MINIMIZED){
				GetClientRect(hWnd,&mainRect);
			}
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hWnd, &ps);
				DoubleBuffer(hdc,mainRect,DrawMainWnd);
			EndPaint(hWnd, &ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam){
				case VK_SPACE:
					GetProcessPath();
					break;
				case VK_RETURN:
					GetOpenedWindow();
					printf("================================\n");
					break;
			}
			return 0;
		case WM_DESTROY:
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
			printf("%d,(%d,%d)\tname:%s\npath: %s\n=================\n",IsZoomed(hWnd),wInfo.rcWindow.left,wInfo.rcWindow.top,progName,path);
		}
		CloseHandle(hProc);
	}
	
	return TRUE;
}
void GetProcessPath(){
	char bGet=FALSE;
	char buf[260]="";
	HANDLE hSnap;
	HANDLE hProc;
	PROCESSENTRY32 ppe;
	char path[1024]={0};
	
	hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPALL,0);
	ppe.dwSize=sizeof(PROCESSENTRY32);
	bGet=Process32First(hSnap,&ppe);
	printf("%d\n",bGet);
	while(bGet){
		hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,ppe.th32ProcessID);
		printf("hProc: %d  |",ppe.th32ProcessID);
		if(hProc){
			GetModuleFileNameEx(hProc,NULL,path,1024);
			printf("path: %s\n",path);
			CloseHandle(hProc);
		}
		bGet=Process32Next(hSnap,&ppe);
	}
	CloseHandle(hSnap); 
}
/*
열려있는 프로그램 감지하기-
전체 프로세스에서  
*/
void GetOpenedWindow(){
	WINDOWINFO wInfo;
	DWORD pID;
	HANDLE hProc;
	HWND tempWin=FindWindow(NULL,NULL);
	char path[1024]={0};
	int i;
	while(tempWin!=NULL){
		if(GetParent(tempWin)==NULL){
			GetWindowInfo(tempWin,&wInfo);
			if(IsWindowVisible(tempWin)){
				printf("%X\t|",wInfo.dwExStyle);
				GetWindowThreadProcessId(tempWin,&pID);
				hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pID);
				if(hProc){
					GetModuleFileNameEx(hProc,NULL,path,1024);
					printf("hProc: %d  |path: %s\n",pID,path);
					CloseHandle(hProc);
				}
			}
		}
		tempWin=GetWindow(tempWin,GW_HWNDNEXT);
	} 
}

