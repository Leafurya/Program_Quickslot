#ifndef _ctrls__h_
#define _ctrls__h_

#include <windows.h>

#define FKEYCOUNT		12

void CreateCtrlFont();

#define ID_SAVECTRLS			1
#define SAVECTRLS_LI_ITEMS		100
#define SAVECTRLS_BT_SAVE		101
#define SAVECTRLS_BT_MODI		102
#define SAVECTRLS_BT_REMOVE		103
#define SAVECTRLS_BT_ORIGIN		170

typedef struct _savectrls{
	HWND btSlot[FKEYCOUNT];
	HWND liItems;
	HWND btSave;
	HWND btModi;
	HWND btRemove;
	HWND stInfo;
}SaveCtrls;

void CreateSaveCtrls(void *,HWND,HINSTANCE);
void MoveSaveCtrls(void *,RECT);

#endif
