#include "data.h"
#include "quickslot.h"
#include <stdio.h>

Item CreateItem(char *path,char *name,char *parameter,char maximized,RECT winPos,HWND hWnd,char detecting){
	Item item;
	
	memset(&item,0,sizeof(Item));
	sprintf(item.path,"%s",path);
	sprintf(item.name,"%s",name);
	if(parameter){
		sprintf(item.parameter,"%s",parameter);
	}
	
	item.maximized=maximized;
	item.xpos=winPos.left;
	item.ypos=winPos.top;
	item.w=winPos.right-winPos.left;
	item.h=winPos.bottom-winPos.top;
	item.hWnd=hWnd;
	item.detecting=detecting;
	//printf("(%d,%d) path: %s | %s\n",item.xpos,item.ypos,item.path,item.parameter);
	
	return item;
}

