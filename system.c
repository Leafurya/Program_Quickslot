#include "system.h"
#include <unistd.h>
#include <stdio.h>
#include <windows.h>

void SetStartProgram(){
	char path[MAX_PATH];
	char value[MAX_PATH];
	char *name="Program_Quickslot";
	HKEY regKey;
	
	getcwd(path,MAX_PATH);
	GetModuleFileName(NULL,path,sizeof(path));
	printf("%s\n",path);
	sprintf(value,"\"%s\"",path);
	
	if(RegCreateKey(HKEY_CURRENT_USER,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",&regKey)==ERROR_SUCCESS){
		RegSetValueEx(regKey,name,0,REG_SZ,(LPBYTE)value,strlen(value)+1);
	}
	RegCloseKey(regKey);
}

void RemoveStartProgram(){
	
}
