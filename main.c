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
#include "progressbar.h"
#include "thread.h"

//#define TIMER_INPUT	0
#define WM_FINDWINDOW	WM_USER+4

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK ListProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);
BOOL CALLBACK ModiDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK NameDlgProc(HWND,UINT,WPARAM,LPARAM);
//BOOL CALLBACK ProgressDlgProc(HWND,UINT,WPARAM,LPARAM);

void InitWindow(HWND);
void TimerFunc(HWND);
void SaveCtrlsCommandFunc(WPARAM,LPARAM);
void ShowSaveButton(char);
void ShowAboutItemFunc(int,char);

unsigned __stdcall SpreadThreadFunc(void *);
unsigned __stdcall KeyInputThreadFunc(void *);
unsigned __stdcall ObserveSlotThreadFunc(void *);

HWND mainWnd;
RECT mainRect;
HINSTANCE g_hInst;
LPSTR mainWndClass="ProgramQuickSlot";
WNDPROC oldListProc;

CtrlManager cm;
SaveCtrls sc;

QuickSlot quickslot[KEYCOUNT];

HICON programIcon;
HANDLE hKeyInputThread;
int threadKiller=1;

int nowSlotIndex=0;
int itemIndex=0;
char *trayName="������";
char saving=0;
int mainWndW=500,mainWndH=300;

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
	}
	
	hWnd=CreateWindow(mainWndClass,mainWndClass,WS_OVERLAPPEDWINDOW,
		  CW_USEDEFAULT,CW_USEDEFAULT,mainWndW,mainWndH,
		  NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,show);
	//ShowWindow(hWnd,SW_SHOW);

	while(GetMessage(&Message,0,0,0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

//	FreeConsole();

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
			mainWnd=hWnd;
			InitWindow(hWnd);
			return 0;
		case WM_SIZE:
			if(wParam!=SIZE_MINIMIZED){
				GetClientRect(hWnd,&mainRect);
				CallMoveFunc(cm,mainRect);
			}
			if(wParam==SIZE_MINIMIZED){
				printf("SIZE_MINIMIZED\n");
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
		case WM_TRAY_MSG:
			TrayCommandFunc(hWnd,lParam,quickslot,KEYCOUNT);
			return 0;
		case WM_COMMAND:
			switch((int)(LOWORD(wParam)/100)){
				case ID_SAVECTRLS:
					SaveCtrlsCommandFunc(wParam,lParam);
					break;
				default:
					index=HIWORD(wParam);
					switch(LOWORD(wParam)){
						case WM_EXIT_PROGRAM:
							DestroyWindow(hWnd);
							break;
						case WM_OPEN_PROGRAM:
							ShowWindow(hWnd,SW_SHOW);
							SetForegroundWindow(hWnd);
							break;
						case WM_CLOSE_SLOT:
							CloseSlot(&quickslot[index]);
							break;
						case WM_OPEN_SLOT:
							SetNowIndex(index);
							StartThread(SpreadThreadFunc,(int *)&index);
							DialogBox(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
							break;
						case WM_FOREGROUND_SLOT:
							ForegroundSlot(quickslot[index]);
							break;
						case WM_SLOT_PROPERTY:
							ShowWindow(hWnd,SW_SHOW);
							SendMessage(hWnd,WM_COMMAND,SAVECTRLS_BT_ORIGIN+index,0);
							break;
						case WM_FILL_SLOT:
							ShowWindow(hWnd,SW_SHOW);
							if(nowSlotIndex==index){
								SendMessage(hWnd,WM_COMMAND,SAVECTRLS_BT_FIND,0);
								break;
							}
							SendMessage(hWnd,WM_COMMAND,SAVECTRLS_BT_ORIGIN+index,0);
							//printf("index: %d\n",index); 
							if(nowSlotIndex==index){
								SendMessage(hWnd,WM_COMMAND,SAVECTRLS_BT_FIND,0);
							}
							break;
					}
					break;
			}
			return 0;
		case WM_CLOSE:
			ShowWindow(hWnd,SW_HIDE);
			return 0;
		case WM_DESTROY:
			threadKiller=0;
			WaitForSingleObject(hKeyInputThread,INFINITE);
			CloseHandle(hKeyInputThread);
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
	//SetTimer(hWnd,TIMER_INPUT,1,NULL);
	hKeyInputThread=StartThread(KeyInputThreadFunc,NULL);
	
	ShowItemList(quickslot[nowSlotIndex],sc.liItems);
	ShowItemInfo(quickslot[nowSlotIndex].slotName,NULL,sc.stInfo);
	
	CreateTrayIcon(hWnd,programIcon,trayName);
	ShowSaveButton(0);
	
	oldListProc=(WNDPROC)SetWindowLongPtr(sc.liItems,GWLP_WNDPROC,(LONG_PTR)ListProc);
	//ShowSlotData(quickslot);
}
	char SlotNameCompare(void *data1,void *data2){
		return !strcmp((char *)data1,(char *)data2);
	}
void TimerFunc(HWND hWnd){
	int index;
	int i;
	char trayMessage[32]={0};
	
	if((index=GetSlotIndex())!=-1){
		if(IsSlotOpened(quickslot[index])){
			ForegroundSlot(quickslot[index]);
			return;
		}
		
		SetNowIndex(index);
		StartThread(SpreadThreadFunc,(int *)&index);
		DialogBox(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
	}
}

	void ShowAboutItemFunc(int itemIndex,char reList){
		printf("nowSlotIndex: %d\n",nowSlotIndex);
		if(reList)
			ShowItemList(quickslot[nowSlotIndex],sc.liItems);
		if(itemIndex==-1){
			ShowItemInfo(quickslot[nowSlotIndex].slotName,NULL,sc.stInfo);
		}
		else{
			ShowItemInfo(quickslot[nowSlotIndex].slotName,&quickslot[nowSlotIndex].item[itemIndex],sc.stInfo);
		}
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
						MessageBox(mainWnd,"â�� ã�� �� �����ϴ�.","�˸�",MB_OK);
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
				MessageBox(mainWnd,"������ ����߽��ϴ�.","�˸�",MB_OK);
				break;
			}
			SaveQuickslot(quickslot,sizeof(quickslot));
			ShowItemInfo(quickslot[nowSlotIndex].slotName,NULL,sc.stInfo);
			ShowSaveButton(0);
			saving=0;
			break;
		case SAVECTRLS_BT_FIND:
			if(quickslot[nowSlotIndex].itemCount!=0){
				if(MessageBox(mainWnd,"������ ��ü�ϰڽ��ϱ�?","�˸�",MB_YESNO)==IDNO){
					reList=0;
					break;
				}
			}
			saving=1;
			itemIndex=-1;
			ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
			EnumWindows(GetOpenedWindowProc,(LPARAM)&quickslot[nowSlotIndex]);
			ShowAboutItemFunc(itemIndex,1);
			ShowSaveButton(1);
			
			MessageBox(mainWnd,"���Կ� ������ ���α׷��� �����߽��ϴ�\n�Ű������� �����Ͽ� ������ �۾��� �����ϼ���.\n��� ������ ���� �� �����ư�� Ŭ�����ּ���.","�˸�",MB_OK);
			break;
		case SAVECTRLS_BT_MODI:
			//itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
			if(itemIndex==-1){
				MessageBox(mainWnd,"�������� �������ּ���.","�˸�",MB_OK);
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
			if(MessageBox(mainWnd,"������ �����ϰڽ��ϱ�?","�˸�",MB_YESNO)==IDYES){
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
				SaveQuickslot(quickslot,sizeof(quickslot));
				saving=0;
			}
			ShowAboutItemFunc(itemIndex,1);
			ShowSaveButton(0);
			break;
		default:
			if(saving){
				if(MessageBox(mainWnd,"�������� �̵��ϰڽ��ϱ�?\n(������� ���� ������ ������ϴ�.)","�˸�",MB_YESNO)==IDNO){
					break;
				}
				ZeroMemory(&quickslot[nowSlotIndex],sizeof(QuickSlot));
				saving=0;
			}
			itemIndex=-1;
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
			break;
	}
	return FALSE;
}
BOOL CALLBACK NameDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	char name[256]={0};
	switch(iMessage){
		case WM_INITDIALOG:
			if(strlen(quickslot[nowSlotIndex].slotName)==0){
				sprintf(name,"F%d����",nowSlotIndex+1); 
			}
			SetDlgItemText(hDlg,ID_ED_NAME,name);
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
	void SwitchItemPosition(HWND hWnd,char direction){
		int selCount;
		int index;
		Item tItem;
		
		selCount=SendMessage(hWnd,LB_GETSELCOUNT,0,0);
		if(selCount!=1){
			MessageBox(mainWnd,"�ϳ��� ������ �ֽñ� �ٶ��ϴ�.","�˸�",MB_OK);
			return;
		}
		index=SendMessage(hWnd,LB_GETCURSEL,0,0);
		if(((direction<0)?(index==0):((index+1)>=quickslot[nowSlotIndex].itemCount))){
			return;
		}
		tItem=quickslot[nowSlotIndex].item[index];
		quickslot[nowSlotIndex].item[index]=quickslot[nowSlotIndex].item[index+direction];
		quickslot[nowSlotIndex].item[index+direction]=tItem;
		ShowItemList(quickslot[nowSlotIndex],sc.liItems);
		
		SendMessage(hWnd,LB_SETSEL,TRUE,index+direction);
		if(!saving){
			SaveQuickslot(quickslot,sizeof(quickslot));
		}
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
					if(MessageBox(mainWnd,"�������� �����ϰڽ��ϱ�?","�˸�",MB_YESNO)==IDNO){
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
					SwitchItemPosition(hWnd,-1);
					break;
				case VK_DOWN:
					SwitchItemPosition(hWnd,1);
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
unsigned __stdcall SpreadThreadFunc(void *args){
	int i;
	char trayMessage[32]={0};
	int index=*((int *)args);
	HWND hWnd;
	//SetWindowPos(mainWnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	
	//SetActiveWindow(mainWnd);
	switch(SpreadQuickslot(quickslot,index)){
		case -1:
			CloseSlot(&quickslot[index]);
			ExitDialog(); 
			return 1;
		case 1:
			ExitDialog();
			return 1;
		default:
			break;
	}
	StartThread(ObserveSlotThreadFunc,&quickslot[index]);
	sprintf(trayMessage,"\"%s\" ������ �������ϴ�.",quickslot[index].slotName);
	ForegroundSlot(quickslot[index]);
	ExitDialog();
	return 1;
}
unsigned __stdcall KeyInputThreadFunc(void *args){
	int index;
	RECT rt;
	char text[256]={0};
	
	while(threadKiller){
		if((index=GetSlotIndex())!=-1){
			if(!quickslot[index].itemCount){
				continue;
			}
			if(IsSlotOpened(quickslot[index])){
				ForegroundSlot(quickslot[index]);
				continue;
			}
			SetNowIndex(index);
			StartThread(SpreadThreadFunc,(int *)&index);
			
			DialogBox(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
		}
		Sleep(1);	
	}
}
unsigned __stdcall ObserveSlotThreadFunc(void *args){
	QuickSlot *slot=(QuickSlot *)args;
	int i;
	DWORD pID;
	HWND *hWnd;
	
	while(threadKiller){
		for(i=0;i<slot->itemCount;i++){
			hWnd=&slot->item[i].hWnd;
			if(!GetWindowThreadProcessId(*hWnd,&pID)){
				*hWnd=0;
				continue;
			}
			break;
		}
		if(i==slot->itemCount){
			return 0;
		}
		Sleep(1);
	}
}
