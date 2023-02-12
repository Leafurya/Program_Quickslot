#ifndef _ctrls__h_
#define _ctrls__h_

#include <windows.h>

#define FKEYCOUNT		12

#define ID_SAVECTRLS			1
#define SAVECTRLS_BT_ORIGIN		100

typedef struct _savectrls{
	HWND btSlot[FKEYCOUNT];
}SaveCtrls;

void CreateSaveCtrls(void *,HWND,HINSTANCE);
void MoveSaveCtrls(void *,RECT);

#endif
