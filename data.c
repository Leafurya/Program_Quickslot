#include "data.h"
#include <stdio.h>

Data CreateData(char *path,char maximized,RECT winPos){
	Data data;
	memset(&data,0,sizeof(Data));
	sprintf(data.path,"%s",path);
	data.maximized=maximized;
	data.xpos=winPos.left;
	data.ypos=winPos.top;
	
	return data;
}
