#ifndef _trayicon__h_
#define _trayicon__h_

#include <windows.h>
#include <shellapi.h>

#define WM_TRAY_MSG WM_USER+2
#define WM_EXIT_PROGRAM	WM_USER+3
#define WM_OPEN_PROGRAM	WM_USER+4

void CreateTrayIcon(HWND,HICON,char *);
void CreateNotification(HWND,char *,char *);
void DeleteTrayIcon();
void TrayCommandFunc(HWND,LPARAM);
void ChangeTrayTitle(char *);

#endif
