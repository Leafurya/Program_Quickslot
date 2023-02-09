#include <windows.h>
#include <doublebuffer.h> 
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <psapi.h>

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
void GetProcessPath();

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="mainWndClass";

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
	switch(iMessage) {
		case WM_CREATE:
			SendMessage(hWnd,WM_SIZE,0,0);
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
					//CreateProcess(NULL,name,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
					hWin=FindWindow(NULL,"Leafurya/C-Library: c lib - Chrome");
					GetWindowInfo(hWin,&wInfo);
					printf("%d %d %d %d\n",wInfo.rcWindow.left,wInfo.rcWindow.top,wInfo.rcWindow.right,wInfo.rcWindow.bottom);
					for(i=0;i<1920;i++){
						SetWindowPos(hWin,HWND_NOTOPMOST,i,wInfo.rcWindow.top,wInfo.rcClient.right,wInfo.rcClient.bottom,SWP_NOMOVE|SWP_NOSIZE);
						//Sleep(100);
					}
					break;
			}
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
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
		printf("name: %s  |",ppe.szExeFile);
		hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,ppe.th32ProcessID);
		if(hProc){
			GetModuleFileNameEx(hProc,NULL,path,1024);
			printf("path: %s\n",path);
			CloseHandle(hProc);
		}
		bGet=Process32Next(hSnap,&ppe);
	}
	CloseHandle(hSnap); 
}

