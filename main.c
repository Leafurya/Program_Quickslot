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
#define WM_OPENPBDLG	WM_USER+20

LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK ListProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK EnumWindowsProc(HWND,LPARAM);
BOOL CALLBACK ModiDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK NameDlgProc(HWND,UINT,WPARAM,LPARAM);
//BOOL CALLBACK ProgressDlgProc(HWND,UINT,WPARAM,LPARAM);

void InitWindow(HWND);
//void TimerFunc(HWND);
void SaveCtrlsCommandFunc(WPARAM,LPARAM);
void ShowSaveButton(char);
void ShowAboutItemFunc(Item *,int,char *,int,char);

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
QuickSlotForFinding slotForFinding;

QuickSlot *nowSlot;

HICON programIcon;
HANDLE hKeyInputThread;
HWND hPbDlg;
int threadKiller=1;

int nowSlotIndex=0;
int itemIndex=0;
char *trayName="퀵슬롯";
char saving=0;
int mainWndW=500,mainWndH=300;

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance
		  ,LPSTR lpszCmdParam,int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst=hInstance;
	
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
		memset(quickslot,0,sizeof(quickslot));
	}
	
	hWnd=CreateWindow(mainWndClass,mainWndClass,WS_OVERLAPPEDWINDOW,
		  0,0,mainWndW,mainWndH,//CW_USEDEFAULT,CW_USEDEFAULT
		  NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,SW_HIDE);
	//ShowWindow(hWnd,SW_SHOW);

	while(GetMessage(&Message,0,0,0)) {
		if(!IsDialogMessage(hPbDlg,&Message)){
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
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
			mainWnd=hWnd;
			InitWindow(hWnd);
			//hPbDlg=CreateDialog(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
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
		case WM_TRAY_MSG:
			TrayCommandFunc(hWnd,lParam,quickslot,KEYCOUNT);
			return 0;
		case WM_OPENPBDLG:
			hPbDlg=CreateDialog(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
			return 0;
		case WM_COMMAND:
			switch((int)(LOWORD(wParam)/100)){
				case ID_SAVECTRLS:
					SaveCtrlsCommandFunc(wParam,lParam);
					break;
				default:
					index=HIWORD(wParam);
					nowSlot=&quickslot[index];
					switch(LOWORD(wParam)){
						case WM_EXIT_PROGRAM:
							DestroyWindow(hWnd);
							break;
						case WM_OPEN_PROGRAM:
							ShowWindow(hWnd,SW_SHOW);
							SetForegroundWindow(hWnd);
							break;
						case WM_CLOSE_SLOT:
							CloseSlot(nowSlot);
							break;
						case WM_OPEN_SLOT:
//							SetNowIndex(index);
							SetNowSlot(nowSlot);
							StartThread(SpreadThreadFunc,(int *)&index);
							DialogBox(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
							break;
						case WM_FOREGROUND_SLOT:
							ForegroundSlot(*nowSlot);
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
							printf("index: %d\n",index);
							if(nowSlotIndex==index){
								SendMessage(hWnd,WM_COMMAND,SAVECTRLS_BT_FIND,0);
							}
							break;
					}
					break;
			}
			return 0;
		case WM_CLOSE:
			if(saving){
				if(MessageBox(mainWnd,"저장을 취소하시겠습니까?\n(저장되지 않은 내용은 사라집니다.)","알림",MB_YESNO)==IDNO){
					return 0;
				}
				saving=0;
			}
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
	
//	ShowItemList(quickslot[nowSlotIndex].item,quickslot[nowSlotIndex].itemCount,sc.liItems);
//	ShowItemInfo(quickslot[nowSlotIndex].slotName,NULL,sc.stInfo);
	
	CreateTrayIcon(hWnd,programIcon,trayName);
	ShowSaveButton(0);
	
	oldListProc=(WNDPROC)SetWindowLongPtr(sc.liItems,GWLP_WNDPROC,(LONG_PTR)ListProc);
	//ShowSlotData(quickslot);
}
	char SlotNameCompare(void *data1,void *data2){
		return !strcmp((char *)data1,(char *)data2);
	}
	void ShowAboutItemFunc(Item *item,int itemSize,char *slotName,int itemIndex,char reList){
		//printf("itemSize: %d\n",itemSize);
		if(reList)
			ShowItemList(item,itemSize,sc.liItems);
		if(itemIndex==-1){
			ShowItemInfo(slotName,NULL,sc.stInfo);
		}
		else{
			ShowItemInfo(slotName,&item[itemIndex],sc.stInfo);
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
					if(!SetForegroundWindow(nowSlot->item[itemIndex].hWnd)){
						MessageBox(mainWnd,"창을 찾을 수 없습니다.","알림",MB_OK);
						break;
					}
					if(IsIconic(nowSlot->item[itemIndex].hWnd)){
						ShowWindow(nowSlot->item[itemIndex].hWnd,SW_NORMAL);
					}
					break;
				case LBN_SELCHANGE:
					itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
					ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,nowSlot->slotName,itemIndex,0);
					break;
			}
			break;
		case SAVECTRLS_BT_SAVE:
			if(!slotForFinding.itemCount){
				MessageBox(mainWnd,"지정된 아이템이 없어 저장을 취소합니다.","알림",MB_OK);
				ShowSaveButton(0);
				saving=0;
				ZeroMemory(&slotForFinding,sizeof(QuickSlotForFinding));
				break;
			}
			if(slotForFinding.itemCount>ITEM_MAXSIZE){
				MessageBox(mainWnd,"아이템이 너무 많습니다.\n(10개 이하만 가능)","알림",MB_OK);
				break;
			}
			if(DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG2),mainWnd,(DLGPROC)NameDlgProc)==DLG2_BT_CANCLE){
				MessageBox(mainWnd,"저장을 취소했습니다.","알림",MB_OK);
				break;
			}
			for(i=0;i<slotForFinding.itemCount;i++){
				quickslot[nowSlotIndex].item[i]=slotForFinding.item[i];
			}
			quickslot[nowSlotIndex].itemCount=slotForFinding.itemCount;
			sprintf(quickslot[nowSlotIndex].slotName,"%s",slotForFinding.slotName);
			SaveQuickslot(quickslot,sizeof(quickslot));
//			printf("quickslot[nowSlotIndex].slotName: %s\n",quickslot[nowSlotIndex].slotName);
			ShowItemInfo(quickslot[nowSlotIndex].slotName,NULL,sc.stInfo);
			ShowSaveButton(0);
			saving=0;
			//ShowSlotData(quickslot);
			break;
		case SAVECTRLS_BT_FIND:
			if(nowSlot->itemCount!=0){
				if(MessageBox(mainWnd,"슬롯을 교체하겠습니까?","알림",MB_YESNO)==IDNO){
					reList=0;
					break;
				}
			}
			ZeroMemory(nowSlot,sizeof(QuickSlot));
			nowSlot=(QuickSlot *)&slotForFinding;
			ZeroMemory(nowSlot,sizeof(QuickSlot));
			saving=1;
			itemIndex=-1;
			//EnumWindows(GetOpenedWindowProc,(LPARAM)&quickslot[nowSlotIndex]);
			EnumWindows(GetOpenedWindowProc,(LPARAM)nowSlot);
			ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,"",itemIndex,1);
			ShowSaveButton(1);
			
			MessageBox(mainWnd,"슬롯에 저장할 프로그램을 감지했습니다\n매개변수를 설정하여 정교한 작업을 시작하세요.\n모든 설정이 끝난 후 저장버튼을 클릭해주세요.","알림",MB_OK);
			break;
		case SAVECTRLS_BT_MODI:
			//itemIndex=SendMessage(sc.liItems,LB_GETCURSEL,0,0);
			if(itemIndex==-1){
				MessageBox(mainWnd,"아이템을 선택해주세요.","알림",MB_OK);
				break;
			}
			if(DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG1),mainWnd,(DLGPROC)ModiDlgProc)==ID_BT_CANCLE){
				break;
			}
			if(!saving){
				SaveQuickslot(quickslot,sizeof(quickslot));
				break;
			}
			ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,nowSlot->slotName,itemIndex,1);
			break;
		case SAVECTRLS_BT_REMOVE:
			if(MessageBox(mainWnd,"슬롯을 삭제하겠습니까?","알림",MB_YESNO)==IDYES){
				ZeroMemory(nowSlot,sizeof(QuickSlot));
				SaveQuickslot(quickslot,sizeof(quickslot));
				saving=0;
			}
			ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,nowSlot->slotName,itemIndex,1);
			ShowSaveButton(0);
			break;
		default:
			if(saving){
				if(MessageBox(mainWnd,"슬롯으로 이동하겠습니까?\n(저장되지 않은 내용은 사라집니다.)","알림",MB_YESNO)==IDNO){
					break;
				}
				ZeroMemory(nowSlot,sizeof(QuickSlotForFinding));
				saving=0;
			}
			itemIndex=-1;
			nowSlotIndex=wParam-SAVECTRLS_BT_ORIGIN;
			nowSlot=&quickslot[nowSlotIndex];
			
			ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,nowSlot->slotName,itemIndex,1);
			ShowSaveButton(0);
			break;
	}
}

BOOL CALLBACK ModiDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	switch(iMessage){
		case WM_INITDIALOG:
			SetDlgItemText(hDlg,ID_ED_PATH,nowSlot->item[itemIndex].path);
			SetDlgItemText(hDlg,ID_ED_PARAM,nowSlot->item[itemIndex].parameter);
			break;
		case WM_COMMAND:
			switch(wParam){
				case ID_BT_CANCLE:
					EndDialog(hDlg,wParam);
					return TRUE;
				case ID_BT_OK:
					GetDlgItemText(hDlg,ID_ED_PATH,nowSlot->item[itemIndex].path,sizeof(nowSlot->item[itemIndex].path));
					GetDlgItemText(hDlg,ID_ED_PARAM,nowSlot->item[itemIndex].parameter,sizeof(nowSlot->item[itemIndex].parameter));
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
			if(strlen(nowSlot->slotName)==0){
				sprintf(name,"F%d슬롯",nowSlotIndex+1); 
			}
			SetDlgItemText(hDlg,ID_ED_NAME,name);
			break;
		case WM_COMMAND:
			switch(wParam){
				case DLG2_BT_CANCLE:
					EndDialog(hDlg,wParam);
					return TRUE;
				case DLG2_BT_OK:
					GetDlgItemText(hDlg,ID_ED_NAME,nowSlot->slotName,sizeof(nowSlot->slotName));
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
			MessageBox(mainWnd,"하나만 선택해 주시기 바랍니다.","알림",MB_OK);
			return;
		}
		index=SendMessage(hWnd,LB_GETCURSEL,0,0);
		if(((direction<0)?(index==0):((index+1)>=nowSlot->itemCount))){
			return;
		}
		tItem=nowSlot->item[index];
		nowSlot->item[index]=nowSlot->item[index+direction];
		nowSlot->item[index+direction]=tItem;
		ShowItemList(nowSlot->item,nowSlot->itemCount,sc.liItems);
		
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
					if(MessageBox(mainWnd,"아이템을 삭제하겠습니까?","알림",MB_YESNO)==IDNO){
						break;
					}
					SendMessage(hWnd,LB_GETSELITEMS,nowSlot->itemCount,(LPARAM)selItems);
					for(j=selCount-1;j>=0;j--){
						itemIndex=selItems[j];
						ZeroMemory(&nowSlot->item[itemIndex],sizeof(Item));
						for(i=itemIndex;i<nowSlot->itemCount-1;i++){
							nowSlot->item[i]=nowSlot->item[i+1];
						}
						nowSlot->itemCount--;
					}
					if(!saving){
						SaveQuickslot(quickslot,sizeof(quickslot));
					}
					ShowAboutItemFunc(nowSlot->item,nowSlot->itemCount,nowSlot->slotName,itemIndex,1);
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
	char trayMessage[2048]={0};
	int index=*((int *)args);
	HWND hWnd;
	char *status[ITEM_MAXSIZE];
	//SetWindowPos(mainWnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	
	//SetActiveWindow(mainWnd);
	ZeroMemory(status,sizeof(status));
	switch(SpreadQuickslot(quickslot,index,status)){
		case -1:
			CloseSlot(&quickslot[index]);
			SendMessage(hPbDlg,DM_CLOSE,0,0);
			for(i=0;i<quickslot[index].itemCount;i++){
				if(status[i]){
					free(status[i]);
				}
			}
			hPbDlg=0;
			return 1;
		case 1:
			SendMessage(hPbDlg,DM_CLOSE,0,0);
			for(i=0;i<quickslot[index].itemCount;i++){
				if(status[i]){
					free(status[i]);
				}
			}
			hPbDlg=0;
			return 1;
		default:
			break;
	}
	StartThread(ObserveSlotThreadFunc,&quickslot[index]);
	for(i=0;i<quickslot[index].itemCount;i++){
		if(status[i]){
			strcat(trayMessage,status[i]);
			free(status[i]);
		}
	}
	printf("trayMessage: %s\n",trayMessage);
	CreateNotification(mainWnd,"알림",trayMessage);
	ForegroundSlot(quickslot[index]);
	//printf(")DestroyWindow(hPbDlg): %d\n",DestroyWindow(hPbDlg));
	SendMessage(hPbDlg,DM_CLOSE,0,0);
//	if(!DestroyWindow(hPbDlg)){
//		printf("error: %d\n",GetLastError());
//	}
	hPbDlg=0;
	return 1;
}
unsigned __stdcall KeyInputThreadFunc(void *args){
	int index;
	RECT rt;
	char text[256]={0};
	
	
	while(threadKiller){
		if(IsWindow(hPbDlg)){
			continue;
		}
		if((index=GetSlotIndex())!=-1){
			//hPbDlg=CreateDialog(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
			//ShowWindow(hPbDlg,SW_SHOW);
//			SetNowIndex(index);
			SetNowSlot(&quickslot[index]);
			if(!quickslot[index].itemCount){
				continue;
			}
			if(IsSlotOpened(quickslot[index])){
				ForegroundSlot(quickslot[index]);
				continue;
			}
			SendMessage(mainWnd,WM_OPENPBDLG,0,0);
			StartThread(SpreadThreadFunc,(int *)&index);
			
//			DialogBox(g_hInst,MAKEINTRESOURCE(DLG_PROGRESS),mainWnd,(DLGPROC)ProgressDlgProc);
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
