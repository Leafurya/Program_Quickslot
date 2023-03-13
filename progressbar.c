#include "progressbar.h"
#include "resource.h"

#include <commctrl.h>
#include <stdio.h>

#define DLG_TIMER_TOTOP 0

HWND hProgressBar;
extern QuickSlot quickslot[KEYCOUNT];
int lastGage;
int _nowIndex;
char *blockVar;
HWND _hDlg=0;

BOOL CALLBACK ProgressDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	
	switch(iMessage){
		case WM_INITDIALOG:
			_hDlg=hDlg;
			//SetWindowLongPtr(hDlg,GWL_EXSTYLE,)
			SetWindowText(hDlg,quickslot[_nowIndex].slotName);
			SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			BringWindowToTop(hDlg);
			printf("hDlg: %d|foreground: %d\n",hDlg,GetForegroundWindow());
			//SetWindowPos(GetForegroundWindow(),HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
			
			hProgressBar=GetDlgItem(hDlg,DLG_PB_BAR);
			SendMessage(hProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,quickslot[_nowIndex].itemCount*2));
			break;
		case WM_COMMAND:
			switch(wParam){
				case DLG_PB_CANCEL:
					*blockVar=1;
					if(MessageBox(hDlg,"프로그램 전개를 중지하겠습니까?","알림",MB_YESNO)==IDNO){
						*blockVar=0;
						return TRUE;
					}
					*blockVar=-1;
					CloseHandle(hProgressBar);
					EndDialog(hDlg,wParam);
					return TRUE;
			}
			break;
	}
	return FALSE;
}
void StepBar(){
	SendMessage(hProgressBar,PBM_SETSTEP,1,0);
	SendMessage(hProgressBar,PBM_STEPIT,0,0);
	printf("step\n");
}
void SetBlockVar(char *pVar){
	blockVar=pVar;
}
HWND *GetDlgHandleAdr(){
	return &_hDlg;
}
void SetNowIndex(int nowIndex){
	_nowIndex=nowIndex;
}
void CALLBACK ExitDialog(){
	EndDialog(_hDlg,DLG_PB_CANCEL);
}
