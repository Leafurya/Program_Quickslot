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
#define SAVECTRLS_BT_CHANGE		104
#define SAVECTRLS_BT_REFIND		105
#define SAVECTRLS_CB_DETECTING	106
#define SAVECTRLS_BT_ADDITEM	107

#define SAVECTRLS_BT_ORIGIN		170

#define SAVECTRLS_BT_F1			170
#define SAVECTRLS_BT_F2			171
#define SAVECTRLS_BT_F3			172
#define SAVECTRLS_BT_F4			173
#define SAVECTRLS_BT_F5			174
#define SAVECTRLS_BT_F6			175
#define SAVECTRLS_BT_F7			176
#define SAVECTRLS_BT_F8			177
#define SAVECTRLS_BT_F9			178
#define SAVECTRLS_BT_F10		179
#define SAVECTRLS_BT_F11		180
#define SAVECTRLS_BT_F12		181

typedef struct _savectrls{
//	HWND btSlot[FKEYCOUNT];
	HWND liItems;
	HWND stInfo;
	HWND btChange;
	HWND btRemove;
	
	HWND btSave;
	HWND btRefind;
	
	HWND btModi;
	HWND cbDetecting;
	
	HWND btAddItem;
}SaveCtrls;

void CreateSaveCtrls(void *,HWND,HINSTANCE);
void MoveSaveCtrls(void *,RECT);

#endif
