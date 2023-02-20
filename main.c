#include <windows.h>
#include <doublebuffer.h> 
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <strproc2.h>
#include <ctrlmanager.h>
#include <dirent.h>

#include "quickslot.h"
#include "ctrls.h"
#include "resource.h"
#include "trayicon.h"

#define TIMER_INPUT	0
#define WM_FINDWINDOW	WM_USER+4

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);
BOOL CALLBACK ModiDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK NameDlgProc(HWND,UINT,WPARAM,LPARAM);

void SaveCtrlsCommandFunc(WPARAM,LPARAM);
void ShowSaveButton(char);

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";

CtrlManager cm;
SaveCtrls sc;

QuickSlot quickslot[KEYCOUNT];
HICON programIcon;

int nowSlotIndex=0;
int itemIndex=0;
char *trayName="퀵슬롯";

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance
		  ,LPSTR lpszCmdParam,int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst=hInstance;
	int show=SW_HIDE;
	
	programIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_PROGRAMICON64));
	
	WndClass.cbClsExtra=0;
	WndClass.cbWndExtra=0;
	WndClass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon=programIcon;
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
	
	if(!opendir("./data")){
		mkdir("./data");
	}
	if(!LoadQuickslot(&quickslot,sizeof(quickslot))){
		show=SW_SHOW;
		memset(quickslot,0,sizeof(quickslot));
		//printf("zero memory\n");
	}            
	
	hWnd=CreateWindow(mainWndClass,mainWndClass,WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT,CW_USEDEFAULT,500,300,
		  NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,show);
	//ShowWindow(hWnd,SW_SHOW);

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
	char trayMessage[32]={0};
	switch(iMessage) {
		case WM_CREATE:
			CreateCtrlFont();
			InitCtrlManager(&cm,&mainRect);
			CreateSaveCtrls(&sc,hWnd,g_hInst);
			RegistCtrlGroup(&cm,&sc,ID_SAVECTRLS,sizeof(SaveCtrls),MoveSaveCtrls);
			SetNowCtrlGroup(&cm,ID_SAVECTRLS);
			SendMessage(hWnd,WM_SIZE,0,0);
			//ShowWindow(hWnd,SW_HIDE);
			
			for(index=0;index<KEYCOUNT;index++){
				//printf("index:%d===========\n",index);
				for(i=0;i<quickslot[index].itemCount;i++){
					quickslot[index].item[i].hWnd=0;
					//printf("max: %d| (%d,%d)%s\t |path: %s\n",quickslot[index].item[i].maximized,quickslot[index].item[i].xpos,quickslot[index].item[i].ypos,quickslot[index].item[i].name,quickslot[index].item[i].path);
				}
			}
			SetTimer(hWnd,TIMER_INPUT,1,NULL);
			//printf("%d,%d,%d\n",sizeof(HWND),sizeof(DWORD),sizeof(LPARAM));
			
			ShowItemList(quickslot[nowSlotIndex],sc.liItems);
			ShowItemInfo(quickslot[nowSlotIndex].slotName,quickslot[nowSlotIndex].item[0],sc.stInfo);
			
			CreateTrayIcon(hWnd,programIcon,trayName);
			ShowSlotData(quickslot);
			ShowSaveButton(0);
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
			printf("keydown:%d\n",wParam);
			switch(wParam){
				case VK_ESCAPE:
					//MoveWindow(wnd,0,0,1000,1000,TRUE);
					printf("================================\n");
					DestroyWindow(hWnd);
					break;
			}
			return 0;
		case WM_TIMER:
			switch(wParam){
				case TIMER_INPUT:
					if((index=GetSlotIndex())!=-1){
						printf("if((index=GetSlotIndex())!=-1):%p\n",quickslot[index].item);
						if(SpreadQuickslot(quickslot,index)){
							printf("empty slot\n");
							break;
						}
						sprintf(trayMessage,"\"%s\" 슬롯을 열었습니다.",quickslot[index].slotName);
						for(index=0;index<KEYCOUNT;index++){
							printf("index:%d===========\n",index);
							for(i=0;i<quickslot[index].itemCount;i++){
								//quickslot[index].item[i].hWnd=0;
								printf("hWnd:%d\tmax: %d| (%d,%d)%s\t |path: %s\n",quickslot[index].item[i].hWnd,quickslot[index].item[i].maximized,quickslot[index].item[i].xpos,quickslot[index].item[i].ypos,quickslot[index].item[i].name,quickslot[index].item[i].path);
							}
						}
						CreateNotification(hWnd,trayName,trayMessage);
						//sprintf(trayMessage,"F%d슬롯",nowSlotIndex+1);
						ChangeTrayTitle(quickslot[index].slotName);
						ShowSlotData(quickslot);
					}
					break;
			}
			return 0;
		case WM_TRAY_MSG:
			TrayCommandFunc(hWnd,lParam);
			return 0;
		case WM_COMMAND:
			switch((int)(LOWORD(wParam)/100)){
				case ID_SAVECTRLS:
					SaveCtrlsCommandFunc(wParam,lParam);
					break;
				default:
					switch(wParam){
						case WM_EXIT_PROGRAM:
							DestroyWindow(hWnd);
							break;
						case WM_OPEN_PROGRAM:
							ShowWindow(hWnd,SW_SHOW);
							break;
					}
					break;
			}
			return 0;
		case WM_CLOSE:
			ShowWindow(hWnd,SW_HIDE);
			return 0;
		case WM_DESTROY:
			DeleteTrayIcon();
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
			//printf("hWnd: %d\ttpath: %s\n",hWnd,tpath);
			//SendMessage(hWnd,WM_SIZE,SIZE_MINIMIZED,0);
//			for(i=0;i<lpQuickslot->itemCount;i++){
//				if(!strcmp(lpQuickslot->item[i].path,tpath)){
//					DeleteString(&str);
//					CloseHandle(hProc);
//					return TRUE;
//				}
//			}
			GetWindowRect(hWnd,&rect);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(path,NULL,IsZoomed(hWnd),wInfo.rcWindow,hWnd);
			//printf("pid:%d\n",pID);
		}
		DeleteString(&str);
	}
	CloseHandle(hProc);
	
	return TRUE;
}
	void ShowAboutItemFunc(int itemIndex,char reList){
		if(reList)
			ShowItemList(quickslot[nowSlotIndex],sc.liItems);
		ShowItemInfo(quickslot[nowSlotIndex].slotName,quickslot[nowSlotIndex].item[itemIndex],sc.stInfo);
	}
void ShowSaveButton(char val){
	ShowWindow(sc.btFind,val?SW_HIDE:SW_SHOW);
	ShowWindow(sc.btSave,val?SW_SHOW:SW_HIDE);
}
void SaveCtrlsCommandFunc(WPARAM wParam,LPARAM lParam){
	int i;
	//int itemIndex=0;
	char msgString[256];
	char reList=1;
	switch(LOWORD(wParam)){
		case SAVECTRLS_LI_ITEMS:
			//printf("list click\n");
			switch(HIWORD(wParam)){
				case LBN_DBLCLK:
					//printf("dblcick\n");
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					printf("%d,%d,quickslot[nowSlotIndex].item[itemIndex].hWnd: %d\n",nowSlotIndex,itemIndex,quickslot[nowSlotIndex].item[itemIndex].hWnd);
					if(!SetForegroundWindow(quickslot[nowSlotIndex].item[itemIndex].hWnd)){
						printf("%d %s\n",GetLastError(),strerror(errno));
						MessageBox(mainWnd,"창을 찾을 수 없습니다.","알림",MB_OK);
						break;
					}
					if(IsIconic(quickslot[nowSlotIndex].item[itemIndex].hWnd)){
						//MessageBox(mainWnd,"창이 최소화 되어있습니다.","알림",MB_OK);
						ShowWindow(quickslot[nowSlotIndex].item[itemIndex].hWnd,SW_NORMAL);
					}
					break;
				case LBN_SELCHANGE:
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					ShowAboutItemFunc(itemIndex,0);
					//ShowItemInfo(nowSlotIndex+1,quickslot[nowSlotIndex].item[itemIndex],sc.stInfo);
					break;
			}
			break;
		case SAVECTRLS_BT_SAVE:
			//printf("done: %d\n",SaveQuickslot(quickslot,sizeof(quickslot)));
			//printf("%d",DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG2),mainWnd,(DLGPROC)NameDlgProc));
			if(DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG2),mainWnd,(DLGPROC)NameDlgProc)==DLG2_BT_CANCLE){
				MessageBox(mainWnd,"저장을 취소했습니다.","알림",MB_OK);
				break;
			}
			SaveQuickslot(quickslot,sizeof(quickslot));
			printf("quickslot[nowSlotIndex].slotName: %s|\n",quickslot[nowSlotIndex].slotName); 
			ShowItemInfo(quickslot[nowSlotIndex].slotName,quickslot[nowSlotIndex].item[0],sc.stInfo);
			//ShowItemInfo(quickslot)
			ShowSaveButton(0);
//			sprintf(msgString,"F%d슬롯을 저장했습니다.",nowSlotIndex+1);
//			MessageBox(mainWnd,msgString,"알림",MB_OK);
			break;
		case SAVECTRLS_BT_FIND:
			itemIndex=0;
			//printf("index:%d\n",nowSlotIndex);
			if(quickslot[nowSlotIndex].itemCount!=0){
				if(MessageBox(mainWnd,"슬롯을 교체하겠습니까?","알림",MB_YESNO)==IDNO){
					reList=0;
					break;
				}
			}
			ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			EnumWindows(EnumWindowsProc,(LPARAM)&quickslot[nowSlotIndex]);
			//ShowAboutItemFunc(itemIndex,1);
//			for(i=0;i<quickslot[nowSlotIndex].itemCount;i++){
//				printf("max: %d| (%d,%d) |path: %s\n",quickslot[nowSlotIndex].item[i].maximized,quickslot[nowSlotIndex].item[i].xpos,quickslot[nowSlotIndex].item[i].ypos,quickslot[nowSlotIndex].item[i].path);
//			}
			MessageBox(mainWnd,"슬롯에 저장할 프로그램을 감지했습니다\n매개변수를 설정하여 정교한 작업을 시작하세요.\n모든 설정이 끝난 후 저장버튼을 클릭해주세요.","알림",MB_OK);
			ShowAboutItemFunc(itemIndex,1);
			ShowSaveButton(1);
			break;
		case SAVECTRLS_BT_MODI:
			itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
			if(itemIndex==-1){
				MessageBox(mainWnd,"아이템을 선택해주세요.","알림",MB_OK);
				break;
			}
			if(DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG1),mainWnd,(DLGPROC)ModiDlgProc)==ID_BT_CANCLE){
				break;
			}
			if(IsWindowVisible(sc.btFind)){
				SaveQuickslot(quickslot,sizeof(quickslot));
			}
			ShowAboutItemFunc(itemIndex,1);
			break;
		case SAVECTRLS_BT_REMOVE:
			if(MessageBox(mainWnd,"슬롯을 삭제하겠습니까?","알림",MB_YESNO)==IDYES){
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
				SaveQuickslot(quickslot,sizeof(quickslot));
			}
			ShowAboutItemFunc(itemIndex,1);
			ShowSaveButton(0);
			break;
		default:
			if(IsWindowVisible(sc.btSave)){
				if(MessageBox(mainWnd,"슬롯으로 이동하겠습니까?\n(저장되지 않은 내용은 사라집니다.)","알림",MB_YESNO)==IDNO){
					break;
				}
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			}
			itemIndex=0;
			nowSlotIndex=wParam-SAVECTRLS_BT_ORIGIN;
			//printf("index: %d\n",nowSlotIndex);
			ShowAboutItemFunc(itemIndex,1);
			ShowSaveButton(0);
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
BOOL CALLBACK NameDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	switch(iMessage){
		case WM_INITDIALOG:
			if(strlen(quickslot[nowSlotIndex].slotName)==0){
				sprintf(quickslot[nowSlotIndex].slotName,"F%d슬롯",nowSlotIndex+1); 
			}
			SetDlgItemText(hDlg,ID_ED_NAME,quickslot[nowSlotIndex].slotName);
			break;
		case WM_COMMAND:
			switch(wParam){
				case DLG2_BT_CANCLE:
					EndDialog(hDlg,wParam);
					return TRUE;
				case DLG2_BT_OK:
					GetDlgItemText(hDlg,ID_ED_NAME,quickslot[nowSlotIndex].slotName,sizeof(quickslot[nowSlotIndex].slotName));
					EndDialog(hDlg,wParam);
					return TRUE;
			}
			break;
	}
	return FALSE;
}
