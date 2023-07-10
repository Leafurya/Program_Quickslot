#ifndef _data__h_
#define _data__h_

#include <windows.h>

typedef struct _itemv0{
	char path[1024];
	char name[256];
	char parameter[1024];
	char maximized;
	int xpos, ypos;
	int w,h;
	HWND hWnd;
}ItemV0;

typedef struct _itemv1{
	char path[1024];
	char name[256];
	char parameter[1024];
	char maximized;
	int xpos, ypos;
	int w,h;
	HWND hWnd;
	char detecting;
}ItemV1;

typedef struct _itemv2{
	char path[1024];
	char name[256];
	char parameter[1024];
	char maximized;
	int xpos, ypos;
	int w,h;
	HWND hWnd;
	char detecting;
	char winTitle[256];
}ItemV2;

typedef ItemV2 Item;

Item CreateItem(char *,char *,char *,char,RECT,HWND,char,char *);

#endif
