#include <windows.h>
#include <doublebuffer.h> 
#include <stdio.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <strproc2.h>
#include <ctrlmanager.h>
#include <dirent.h>
#include <psapi.h>

#include "quickslot.h"
#include "ctrls.h"
#include "resource.h"
#include "trayicon.h"

#define TIMER_INPUT	0
#define WM_FINDWINDOW	WM_USER+4

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK ListProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);
BOOL CALLBACK ModiDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK NameDlgProc(HWND,UINT,WPARAM,LPARAM);

void InitWindow(HWND);
void TimerFunc(HWND);
void SaveCtrlsCommandFunc(WPARAM,LPARAM);
void ShowSaveButton(char);

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";
WNDPROC oldListProc;

CtrlManager cm;
SaveCtrls sc;

QuickSlot quickslot[KEYCOUNT];
HICON programIcon;

int nowSlotIndex=0;
int itemIndex=0;
char *trayName="퀵슬롯";
char saving=0;

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

//	AllocConsole(); 
//	freopen("COIN$", "r", stdin);
//	freopen("CONOUT$", "w", stdout);
//	freopen("CONOUT$", "w", stderr); 
	
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

	//FreeConsole();

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
			InitWindow(hWnd);
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
				case VK_ESCAPE:
					DestroyWindow(hWnd);
					break;
			}
			return 0;
		case WM_TIMER:
			switch(wParam){
				case TIMER_INPUT:
					TimerFunc(hWnd);
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
							SetForegroundWindow(hWnd);
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
void InitWindow(HWND hWnd){
	int i;
	int index;
	int nowSlotIndex;
	
	CreateCtrlFont();
	InitCtrlManager(&cm,&mainRect);
	CreateSaveCtrls(&sc,hWnd,g_hInst);
	RegistCtrlGroup(&cm,&sc,ID_SAVECTRLS,sizeof(SaveCtrls),MoveSaveCtrls);
	SetNowCtrlGroup(&cm,ID_SAVECTRLS);
	SendMessage(hWnd,WM_SIZE,0,0);
	
	for(index=0;index<KEYCOUNT;index++){
		for(i=0;i<quickslot[index].itemCount;i++){
			quickslot[index].item[i].hWnd=0;
		}
	}
	SetTimer(hWnd,TIMER_INPUT,1,NULL);
	
	ShowItemList(quickslot[nowSlotIndex],sc.liItems);
	ShowItemInfo(quickslot[nowSlotIndex].slotName,quickslot[nowSlotIndex].item[0],sc.stInfo);
	
	CreateTrayIcon(hWnd,programIcon,trayName);
	ShowSaveButton(0);
	
	oldListProc=(WNDPROC)SetWindowLongPtr(sc.liItems,GWLP_WNDPROC,(LONG_PTR)ListProc);
	ShowSlotData(quickslot);
}
void TimerFunc(HWND hWnd){
	int index;
	int i;
	char trayMessage[32]={0};
	
	if((index=GetSlotIndex())!=-1){
		for(i=0;i<quickslot[index].itemCount;i++){
			if(quickslot[index].item[i].hWnd){
				CloseSlot(&quickslot[index]);
				sprintf(trayMessage,"\"%s\" 슬롯을 닫았습니다.",quickslot[index].slotName);
				CreateNotification(hWnd,trayName,trayMessage);
				return;
			}
		}
		if(SpreadQuickslot(quickslot,index)){
			return;
		}
		sprintf(trayMessage,"\"%s\" 슬롯을 열었습니다.",quickslot[index].slotName);
		ShowSlotData(quickslot);
		CreateNotification(hWnd,trayName,trayMessage);
		ChangeTrayTitle(quickslot[index].slotName);
	}
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
			GetWindowRect(hWnd,&rect);
			lpQuickslot->item[lpQuickslot->itemCount++]=CreateItem(path,NULL,IsZoomed(hWnd),wInfo.rcWindow,hWnd);
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
	char msgString[256];
	char reList=1;
	switch(LOWORD(wParam)){
		case SAVECTRLS_LI_ITEMS:
			switch(HIWORD(wParam)){
				case LBN_DBLCLK:
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					if(!SetForegroundWindow(quickslot[nowSlotIndex].item[itemIndex].hWnd)){
						MessageBox(mainWnd,"창을 찾을 수 없습니다.","알림",MB_OK);
						break;
					}
					if(IsIconic(quickslot[nowSlotIndex].item[itemIndex].hWnd)){
						ShowWindow(quickslot[nowSlotIndex].item[itemIndex].hWnd,SW_NORMAL);
					}
					break;
				case LBN_SELCHANGE:
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					ShowAboutItemFunc(itemIndex,0);
					break;
			}
			break;
		case SAVECTRLS_BT_SAVE:
			if(DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG2),mainWnd,(DLGPROC)NameDlgProc)==DLG2_BT_CANCLE){
				MessageBox(mainWnd,"저장을 취소했습니다.","알림",MB_OK);
				break;
			}
			SaveQuickslot(quickslot,sizeof(quickslot));
			ShowItemInfo(quickslot[nowSlotIndex].slotName,quickslot[nowSlotIndex].item[0],sc.stInfo);
			ShowSaveButton(0);
			saving=0;
			break;
		case SAVECTRLS_BT_FIND:
			itemIndex=0;
			if(quickslot[nowSlotIndex].itemCount!=0){
				if(MessageBox(mainWnd,"슬롯을 교체하겠습니까?","알림",MB_YESNO)==IDNO){
					reList=0;
					break;
				}
			}
			saving=1;
			ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			EnumWindows(EnumWindowsProc,(LPARAM)&quickslot[nowSlotIndex]);
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
			if(!saving){
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
			if(saving){
				if(MessageBox(mainWnd,"슬롯으로 이동하겠습니까?\n(저장되지 않은 내용은 사라집니다.)","알림",MB_YESNO)==IDNO){
					break;
				}
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			}
			itemIndex=0;
			nowSlotIndex=wParam-SAVECTRLS_BT_ORIGIN;
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
LRESULT CALLBACK ListProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int selCount;
	static char ctrlUp=1;
	int index,i,j,itemIndex;
	int selItems[16];
	Item tItem;
	
	switch(iMessage){
		case WM_KEYDOWN:
			switch(wParam){
				case VK_DELETE:
					selCount=SendMessage(hWnd,LB_GETSELCOUNT,0,0);
					if(!selCount){
						break;
					}
					if(MessageBox(mainWnd,"아이템을 삭제하겠습니까?","알림",MB_YESNO)==IDNO){
						break;
					}
					SendMessage(hWnd,LB_GETSELITEMS,quickslot[nowSlotIndex].itemCount,(LPARAM)selItems);
					for(j=selCount-1;j>=0;j--){
						itemIndex=selItems[j];
						ZeroMemory(&quickslot[nowSlotIndex].item[itemIndex],sizeof(Item));
						for(i=itemIndex;i<quickslot[nowSlotIndex].itemCount-1;i++){
							quickslot[nowSlotIndex].item[i]=quickslot[nowSlotIndex].item[i+1];
						}
						quickslot[nowSlotIndex].itemCount--;
					}
					if(!saving){
						SaveQuickslot(quickslot,sizeof(quickslot));
					}
					ShowAboutItemFunc(itemIndex,1);
					break;
				case VK_CONTROL:
					ctrlUp=0;
					break;
				case VK_UP:
					selCount=SendMessage(hWnd,LB_GETSELCOUNT,0,0);
					if(selCount!=1){
						break;
					}
					index=SendMessage(hWnd,LB_GETCURSEL,0,0);
					if(index==0){
						break;
					}
					tItem=quickslot[nowSlotIndex].item[index];
					quickslot[nowSlotIndex].item[index]=quickslot[nowSlotIndex].item[index-1];
					quickslot[nowSlotIndex].item[index-1]=tItem;
					ShowItemList(quickslot[nowSlotIndex],sc.liItems);
					SendMessage(hWnd,LB_SETSEL,TRUE,index-1);
					break;
				case VK_DOWN:
					selCount=SendMessage(hWnd,LB_GETSELCOUNT,0,0);
					if(selCount!=1){
						break;
					}
					index=SendMessage(hWnd,LB_GETCURSEL,0,0);
					if(index>=quickslot[nowSlotIndex].itemCount){
						break;
					}
					tItem=quickslot[nowSlotIndex].item[index];
					quickslot[nowSlotIndex].item[index]=quickslot[nowSlotIndex].item[index+1];
					quickslot[nowSlotIndex].item[index+1]=tItem;
					ShowItemList(quickslot[nowSlotIndex],sc.liItems);
					SendMessage(hWnd,LB_SETSEL,TRUE,index+1);
					break;
			}
			return 0;
		case WM_KEYUP:
			switch(wParam){
				case VK_CONTROL:
					ctrlUp=1;
					break;
			}
			return 0;
		case WM_LBUTTONUP:
			if(ctrlUp){
				index=SendMessage(hWnd,LB_GETCURSEL,0,0);
				SendMessage(hWnd,LB_SETSEL,FALSE,-1);
				SendMessage(hWnd,LB_SETSEL,TRUE,index);
			}
			break;
		case WM_LBUTTONDOWN:
			if(ctrlUp){
				index=SendMessage(hWnd,LB_GETCURSEL,0,0);
				SendMessage(hWnd,LB_SETSEL,FALSE,index);
			}
			break;
	}
	return CallWindowProc(oldListProc,hWnd,iMessage,wParam,lParam);
}
