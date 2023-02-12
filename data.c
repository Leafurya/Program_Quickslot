#include "data.h"
#include <stdio.h>

Item CreateItem(char *path,char maximized,RECT winPos){
	Item item;
	memset(&item,0,sizeof(Item));
	sprintf(item.path,"\"%s\"",path);
	item.maximized=maximized;
	item.xpos=winPos.left;
	item.ypos=winPos.top;
	
	return item;
}
