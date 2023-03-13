#ifndef _data__h_
#define _data__h_

#include <windows.h>

typedef struct _item{
	char path[1024];
	char name[256];
	char parameter[1024];
	char maximized;
	int xpos, ypos;
	int w,h;
	HWND hWnd;
}Item;

Item CreateItem(char *,char *,char *,char,RECT,HWND);

#endif
