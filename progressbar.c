#include "progressbar.h"
#include "resource.h"

#include <commctrl.h>
#include <stdio.h>

HWND hProgressBar;
extern QuickSlot quickslot[KEYCOUNT];
extern int nowSlotIndex;
int lastGage;
char *blockVar;
HWND _hDlg=0;

BOOL CALLBACK ProgressDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	switch(iMessage){
		case WM_INITDIALOG:
			_hDlg=hDlg;
			hProgressBar=GetDlgItem(hDlg,DLG_PB_BAR);
			SendMessage(hProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,quickslot[nowSlotIndex].itemCount*2));
			break;
		case WM_COMMAND:
			switch(wParam){
				case DLG_PB_CANCEL:
					*blockVar=1;
					CloseHandle(hProgressBar);
					EndDialog(hDlg,wParam);
					return TRUE;
			}
			break;
	}
	return FALSE;
}
void StepBar(){
	int lastGage;
	lastGage=SendMessage(hProgressBar,PBM_STEPIT,0,0);
}
void SetBlockVar(char *pVar){
	blockVar=pVar;
}
HWND *GetDlgHandleAdr(){
	return &_hDlg;
}

