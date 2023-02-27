#include "data.h"
#include <stdio.h>
#include <strproc2.h>

Item CreateItem(char *path,char *parameter,char maximized,RECT winPos,HWND hWnd){
	Item item;
	STRING string=Split(path,'\\');
	
	memset(&item,0,sizeof(Item));
	sprintf(item.path,"\"%s\"",path);
	sprintf(item.name,"%s",string.strings[string.size-1]);
	if(parameter){
		sprintf(item.parameter,"%s",parameter);
	}
	
	item.maximized=maximized;
	item.xpos=winPos.left;
	item.ypos=winPos.top;
	item.w=winPos.right-winPos.left;
	item.h=winPos.bottom-winPos.top;
	item.hWnd=hWnd;
	
	DeleteString(&string);
	
	printf("(%d,%d) path: %s | %s\n",item.xpos,item.ypos,item.path,item.parameter);
	
	return item;
}
