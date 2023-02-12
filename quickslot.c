#include "quickslot.h"
#include <stdlib.h>
#include <stdio.h>



int GetSlotIndex(int key){
	int i;
	static char toggle=1;
	if(GetKeyState(VK_CONTROL)&0x8000){
		for(i=0;i<KEYCOUNT;i++){
			if((GetKeyState(VK_F1+i)&0x8000)&&toggle){
				toggle=0;
				return i;
			}
		}
	}
	else{
		toggle=1;
	}
	return -1; //didn't find
}
char LoadQuickslot(QuickSlot **pQuickslot,int size){
	FILE *file=fopen("data/slot","rb");
	
	if(file==NULL){
		printf("%s\n",strerror(errno));
		return 0;
	}
	
	fread(pQuickslot,size,1,file);
	
	fclose(file);
	
	return 1;
}
char SaveQuickslot(QuickSlot *pQuickslot,int size){
	FILE *file=fopen("data/slot","wb");
	
	if(file==NULL){
		printf("%s\n",strerror(errno));
		return 0;
	}
	
	fwrite(pQuickslot,size,1,file);
	
	fclose(file);
	return 1;
}
char SpreadQuickslot(QuickSlot slot){
	int i;
	STARTUPINFO si={0,};
	PROCESS_INFORMATION pi={0,};
	if(slot.itemCount!=0){
		for(i=0;i<slot.itemCount;i++){
			CreateProcess( NULL,   // No module name (use command line)
					        slot.item[i].path,        // Command line
					        NULL,           // Process handle not inheritable
					        NULL,           // Thread handle not inheritable
					        FALSE,          // Set handle inheritance to FALSE
					        0,              // No creation flags
					        NULL,           // Use parent's environment block
					        NULL,           // Use parent's starting directory 
					        &si,            // Pointer to STARTUPINFO structure
					        &pi );           // Pointer to PROCESS_INFORMATION structure
			//CreateProcess(NULL,slot.item[i].path,NULL,NULL,TRUE,,NULL,NULL);
//			system(slot.item[i].path);
//			SetWindowPos()
		}
		return 0;
	}
	return 1;
}
