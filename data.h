#ifndef _data__h_
#define _data__h_

#include <windows.h>

typedef struct _data{
	char path[1024];
	char maximized;
	int xpos, ypos;
}Data, *LPData;

Data CreateData(char *,char,RECT);

#endif
