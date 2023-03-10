#ifndef _trayicon__h_
#define _trayicon__h_

#include <windows.h>
#include <shellapi.h>

#include "quickslot.h"

#define WM_TRAY_MSG WM_USER+2
#define WM_EXIT_PROGRAM	WM_USER+3
#define WM_OPEN_PROGRAM	WM_USER+4
#define WM_CLOSE_SLOT	WM_USER+5
#define WM_OPEN_SLOT	WM_USER+6
#define WM_SLOT_PROPERTY	WM_USER+7
#define WM_FILL_SLOT	WM_USER+8
#define WM_FOREGROUND_SLOT	WM_USER+9

void CreateTrayIcon(HWND,HICON,char *);
void CreateNotification(HWND,char *,char *);
void DeleteTrayIcon();
void TrayCommandFunc(HWND,LPARAM,QuickSlot *,int);
void ChangeTrayTitle(char *);

#endif
