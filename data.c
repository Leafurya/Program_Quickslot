#include "data.h"
#include "quickslot.h"
#include <stdio.h>

Item CreateItem(char *path,char *name,char *parameter,char maximized,RECT winPos,HWND hWnd,char detecting,char *winTitle){
	Item item;
	
	memset(&item,0,sizeof(Item));
	if(path[0]!='\"'&&path[lstrlen(path)-1]!='\"'){
		sprintf(item.path,"\"%s\"",path);
	}
	else if(path[0]!='\"'){
		sprintf(item.path,"\"%s",path);
	}
	else if(path[lstrlen(path)-1]!='\"'){
		sprintf(item.path,"%s\"",path);
	}
	else{
		sprintf(item.path,"%s",path);
	}
	
	
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
	sprintf(item.winTitle,"%s",winTitle);
	//printf("(%d,%d) path: %s | %s\n",item.xpos,item.ypos,item.path,item.parameter);
	
	return item;
}

