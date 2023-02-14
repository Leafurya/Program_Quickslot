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
#include "resource.h"

#define TIMER_INPUT	0

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);
BOOL CALLBACK ModiDlgProc(HWND,UINT,WPARAM,LPARAM);

void SaveCtrlsCommandFunc(WPARAM,LPARAM);

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";

CtrlManager cm;
SaveCtrls sc;

QuickSlot quickslot[KEYCOUNT];

int nowSlotIndex=0;
int itemIndex=0;

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
		  CW_USEDEFAULT,CW_USEDEFAULT,500,300,
		  NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);

	while(GetMessage(&Message,0,0,0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	FreeConsole();

	return Message.wParam;
}
	void DrawMainWnd(HDC hdc,RECT rt){
		FillRect(hdc,&rt,(HBRUSH)(COLOR_WINDOW));
		SetBkMode(hdc,TRANSPARENT);
	}
LRESULT CALLBACK MainWndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int i,index;
	POINT pos;
	HWND wnd;
	switch(iMessage) {
		case WM_CREATE:
			
			
			CreateCtrlFont();
			InitCtrlManager(&cm,&mainRect);
			CreateSaveCtrls(&sc,hWnd,g_hInst);
			RegistCtrlGroup(&cm,&sc,ID_SAVECTRLS,sizeof(SaveCtrls),MoveSaveCtrls);
			SetNowCtrlGroup(&cm,ID_SAVECTRLS);
			
			if(!LoadQuickslot(&quickslot,sizeof(quickslot))){
				memset(quickslot,0,sizeof(quickslot));
				printf("zero memory\n");
			}
			for(index=0;index<KEYCOUNT;index++){
				printf("index:%d===========\n",index);
				for(i=0;i<quickslot[index].itemCount;i++){
					printf("max: %d| (%d,%d)%s\t |path: %s\n",quickslot[index].item[i].maximized,quickslot[index].item[i].xpos,quickslot[index].item[i].ypos,quickslot[index].item[i].name,quickslot[index].item[i].path);
				}
			}
			SetTimer(hWnd,TIMER_INPUT,1,NULL);
			printf("%d,%d,%d\n",sizeof(HWND),sizeof(DWORD),sizeof(LPARAM));
			
			ShowItemList(quickslot[nowSlotIndex],sc.liItems);
			ShowItemInfo(1,quickslot[nowSlotIndex].item[0],sc.stInfo);
			
			SendMessage(hWnd,WM_SIZE,0,0);
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
					wnd=FindWindow(NULL,"solt item");
					printf("wnd: %d\n",wnd);
					//MoveWindow(wnd,0,0,1000,1000,TRUE);
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
	char tpath[1024]={0};
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
			sprintf(tpath,"\"%s\"",path);
			printf("tpath: %s\n",tpath);
			//SendMessage(hWnd,WM_SIZE,SIZE_MINIMIZED,0);
//			for(i=0;i<lpQuickslot->itemCount;i++){
//				if(!strcmp(lpQuickslot->item[i].path,tpath)){
//					DeleteString(&str);
//					CloseHandle(hProc);
//					return TRUE;
//				}
//			}
			GetWindowRect(hWnd,&rect);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(path,NULL,IsZoomed(hWnd),wInfo.rcWindow);
			printf("(%d,%d)\n",rect.left,rect.top);
		}
		DeleteString(&str);
	}
	CloseHandle(hProc);
	
	return TRUE;
}
	void ShowAboutItemFunc(int itemIndex,char reList){
		if(reList)
			ShowItemList(quickslot[nowSlotIndex],sc.liItems);
		ShowItemInfo(nowSlotIndex+1,quickslot[nowSlotIndex].item[itemIndex],sc.stInfo);
	}
void SaveCtrlsCommandFunc(WPARAM wParam,LPARAM lParam){
	int i;
	//int itemIndex=0;
	char msgString[256];
	char reList=1;
	switch(LOWORD(wParam)){
		case SAVECTRLS_LI_ITEMS:
			printf("list click\n");
			switch(HIWORD(wParam)){
				case LBN_SELCHANGE:
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					ShowAboutItemFunc(itemIndex,0);
					//ShowItemInfo(nowSlotIndex+1,quickslot[nowSlotIndex].item[itemIndex],sc.stInfo);
					break;
			}
			break;
		case SAVECTRLS_BT_SAVE:
			itemIndex=0;
			printf("index:%d\n",nowSlotIndex);
			if(quickslot[nowSlotIndex].itemCount!=0){
				sprintf(msgString,"F%d슬롯을 교체하겠습니까?",nowSlotIndex+1);
				if(MessageBox(mainWnd,msgString,"알림",MB_YESNO)==IDNO){
					reList=0;
					break;
				}
			}
			ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			EnumWindows(EnumWindowsProc,(LPARAM)&quickslot[nowSlotIndex]);
			for(i=0;i<quickslot[nowSlotIndex].itemCount;i++){
				printf("max: %d| (%d,%d) |path: %s\n",quickslot[nowSlotIndex].item[i].maximized,quickslot[nowSlotIndex].item[i].xpos,quickslot[nowSlotIndex].item[i].ypos,quickslot[nowSlotIndex].item[i].path);
			}
			printf("done: %d\n",SaveQuickslot(quickslot,sizeof(quickslot)));
			ShowAboutItemFunc(itemIndex,1);
			break;
		case SAVECTRLS_BT_MODI:
			itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
			DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG1),mainWnd,(DLGPROC)ModiDlgProc);
			ShowAboutItemFunc(itemIndex,1);
			break;
		case SAVECTRLS_BT_REMOVE:
			sprintf(msgString,"F%d슬롯을 삭제하겠습니까?",nowSlotIndex+1);
			if(MessageBox(mainWnd,msgString,"알림",MB_YESNO)==IDYES){
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
				SaveQuickslot(quickslot,sizeof(quickslot));
			}
			ShowAboutItemFunc(itemIndex,1);
			break;
		default:
			itemIndex=0;
			nowSlotIndex=wParam-SAVECTRLS_BT_ORIGIN;
			printf("index: %d\n",nowSlotIndex);
			ShowAboutItemFunc(itemIndex,1);
			break;
	}
}

BOOL CALLBACK ModiDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	switch(iMessage){
		case WM_INITDIALOG:
			SetDlgItemText(hDlg,ID_ED_PATH,quickslot[nowSlotIndex].item[itemIndex].path);
			SetDlgItemText(hDlg,ID_ED_PARAM,quickslot[nowSlotIndex].item[itemIndex].parameter);
			break;
		case WM_COMMAND:
			switch(wParam){
				case ID_BT_CANCLE:
					EndDialog(hDlg,wParam);
					return TRUE;
				case ID_BT_OK:
					GetDlgItemText(hDlg,ID_ED_PATH,quickslot[nowSlotIndex].item[itemIndex].path,sizeof(quickslot[nowSlotIndex].item[itemIndex].path));
					GetDlgItemText(hDlg,ID_ED_PARAM,quickslot[nowSlotIndex].item[itemIndex].parameter,sizeof(quickslot[nowSlotIndex].item[itemIndex].parameter));
					SaveQuickslot(quickslot,sizeof(quickslot));
					EndDialog(hDlg,wParam);
					return TRUE;
				case ID_BT_REMOVE:
					if(MessageBox(hDlg,"아이템을 삭제하겠습니까?","알림",MB_YESNO)==IDYES){
						ZeroMemory(&quickslot[nowSlotIndex].item[itemIndex],sizeof(Item));
						for(i=itemIndex;i<quickslot[nowSlotIndex].itemCount-1;i++){
							quickslot[nowSlotIndex].item[i]=quickslot[nowSlotIndex].item[i+1];
						}
						quickslot[nowSlotIndex].itemCount--;
						SaveQuickslot(quickslot,sizeof(quickslot));
						EndDialog(hDlg,wParam);
						return TRUE;
					}
					break;
			}
	}
	return FALSE;
}
