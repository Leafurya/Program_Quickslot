#ifndef _data__h_
#define _data__h_

#include <windows.h>

typedef struct _item{
	char path[1024];
	char maximized;
	int xpos, ypos;
}Item, *LPItem;

Item CreateItem(char *,char,RECT);

#endif
