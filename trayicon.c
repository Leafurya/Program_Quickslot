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
		opened=0;
//		printf("slot[i].slotName: %s\n",slot[i].slotName);
		if(strlen(slot[i].slotName)){
			opened=IsSlotOpened(slot[i]);
			AppendMenu(slotMenu[i],opened?MF_STRING:MF_STRING|MF_DISABLED,(UINT_PTR)(MAKEWPARAM(WM_CLOSE_SLOT,i)),"´Ý±â");
			AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_SLOT_PROPERTY,i)),"¼Ó¼º");
		}
		else{
			AppendMenu(slotMenu[i],MF_STRING,(UINT_PTR)(MAKEWPARAM(WM_FILL_SLOT,i)),"Ã¤¿ì±â");
		}
		
		sprintf(str,"F%d %s\0",i+1,strlen(slot[i].slotName)?slot[i].slotName:"ºó½½·Ô");
//		printf("tray slot name: %s\n",str);
		AppendMenu(main,MF_POPUP|(opened?MF_CHECKED:MF_UNCHECKED),(UINT_PTR)slotMenu[i],str);
	}
	
	AppendMenu(main,MF_SEPARATOR,0,0);
	//AppendMenu(main,MF_STRING,WM_OPEN_PROGRAM,"¿­±â");
	AppendMenu(main,MF_STRING,WM_EXIT_PROGRAM,"Á¾·á");
	
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
		case WM_LBUTTONUP:
			trayMenu=CreateTrayMenu(slot,slotMenu);
			SetForegroundWindow(hWnd);
			GetCursorPos(&mousePos);
			TrackPopupMenu(trayMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,mousePos.x,mousePos.y,0,hWnd,NULL);
			for(i=0;i<KEYCOUNT;i++){
				DestroyMenu(slotMenu[i]);
			}
			DestroyMenu(trayMenu);
			break;
	}
}
void ChangeTrayTitle(char *title){
	strcpy(trayicon.szTip,title);
	Shell_NotifyIcon(NIM_MODIFY,&trayicon);
}
