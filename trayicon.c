#include "trayicon.h"
#include <stdio.h> 

NOTIFYICONDATA trayicon;

void CreateTrayIcon(HWND hWnd,HICON icon,char *title){
	memset(&trayicon,0,sizeof(trayicon));
	trayicon.cbSize=sizeof(NOTIFYICONDATA);
	trayicon.hWnd=hWnd;
	trayicon.uID=100;
	trayicon.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	trayicon.uCallbackMessage=WM_TRAY_MSG;
	trayicon.hIcon=icon;
	strcpy(trayicon.szTip,title);
	Shell_NotifyIcon(NIM_ADD,&trayicon);
}
void CreateNotification(HWND hWnd,char *title,char *content){
	NOTIFYICONDATA ballData;
	memset(&ballData,0,sizeof(ballData));
	ballData.cbSize=sizeof(NOTIFYICONDATA);
	ballData.hWnd=hWnd;
	ballData.uID=trayicon.uID;
	ballData.uFlags=NIF_INFO;
	ballData.uCallbackMessage=WM_TRAY_MSG;
	ballData.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	ballData.uTimeout=700;
	ballData.dwInfoFlags=NIIF_INFO;
	strcpy(ballData.szInfoTitle,title);
	strcpy(ballData.szInfo,content);
	Shell_NotifyIcon(NIM_MODIFY,&ballData);
}
void DeleteTrayIcon(){
	Shell_NotifyIcon(NIM_DELETE,&trayicon);
}
HMENU CreateTrayMenu(QuickSlot *slot,HMENU *slotMenu){
	HMENU main;
	char str[256]={0};
	char opened=0;
	
	int i;
	
	main=CreatePopupMenu();
	
	for(i=0;i<KEYCOUNT;i++){
		slotMenu[i]=CreatePopupMenu();
		
		if(strlen(slot[i].slotName)){
			opened=IsSlotOpened(slot[i]);
//			if(opened=IsSlotOpened(slot[i])){
//				sprintf(str,"����(shift+ctrl+F%d)\0",i+1);
//				AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_FOREGROUND_SLOT,i)),str);
//				//AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_CLOSE_SLOT,i)),"�ݱ�");
//			}
//			else{
//				sprintf(str,"����(shift+ctrl+F%d)\0",i+1);
//				AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_OPEN_SLOT,i)),str);
//			}
			AppendMenu(slotMenu[i],opened?MF_STRING:MF_STRING|MF_DISABLED,(UINT_PTR)(MAKEWPARAM(WM_CLOSE_SLOT,i)),"�ݱ�");
			AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_SLOT_PROPERTY,i)),"�Ӽ�");
		}
		else{
			AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_FILL_SLOT,i)),"ä���");
		}
		
		sprintf(str,"F%d %s\0",i+1,strlen(slot[i].slotName)?slot[i].slotName:"�󽽷�");
		AppendMenu(main,MF_POPUP|(opened?MF_CHECKED:MF_UNCHECKED),(UINT_PTR)slotMenu[i],str);
	}
	
	AppendMenu(main,MF_SEPARATOR,0,0);
	//AppendMenu(main,MF_STRING,WM_OPEN_PROGRAM,"����");
	AppendMenu(main,MF_STRING,WM_EXIT_PROGRAM,"����");
	
	return main;
}
void TrayCommandFunc(HWND hWnd,LPARAM lParam,QuickSlot *slot,int maxSlotSize){
	HMENU trayMenu;
	HMENU slotMenu[KEYCOUNT];
	POINT mousePos;
	int i,j;
	char menuStr[126]={0};
	
	switch(LOWORD(lParam)){
		case WM_RBUTTONUP:
			trayMenu=CreateTrayMenu(slot,slotMenu);
//			openedSlotList=CreatePopupMenu();
//			
//			AppendMenu(openedSlotList,MF_STRING,-1,"�׽�Ʈ ����");
//			
//			//TrackPopupMenu(openedSlotList,TPM_LEFTALIGN|TPM_RIGHTBUTTON,mousePos.x,mousePos.y,0,hWnd,NULL);
//			
//			trayMenu=CreatePopupMenu();
//			for(i=0;i<maxSlotSize;i++){
//				if(IsSlotOpened(slot[i])){
//					sprintf(menuStr,"%s �ݱ�",slot[i].slotName);
//					AppendMenu(trayMenu,MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_CLOSE_SLOT,i)),menuStr);
//				}
//			}
//			AppendMenu(trayMenu,MF_POPUP,(UINT_PTR)openedSlotList,"sub");
//			AppendMenu(trayMenu,MF_SEPARATOR,0,0);
//			AppendMenu(trayMenu,MF_STRING,WM_OPEN_PROGRAM,"����");
//			AppendMenu(trayMenu,MF_STRING,WM_EXIT_PROGRAM,"����");
			SetForegroundWindow(hWnd);
			GetCursorPos(&mousePos);
			TrackPopupMenu(trayMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,mousePos.x,mousePos.y,0,hWnd,NULL);
			for(i=0;i<KEYCOUNT;i++){
				DestroyMenu(slotMenu[i]);
			}
			DestroyMenu(trayMenu);
			printf("rclick\n");
			break;
		case WM_LBUTTONUP:
			ShowWindow(hWnd,SW_SHOW);
			//SetForegroundWindow(hWnd);
			break;
	}
}
void ChangeTrayTitle(char *title){
	strcpy(trayicon.szTip,title);
	Shell_NotifyIcon(NIM_MODIFY,&trayicon);
}
