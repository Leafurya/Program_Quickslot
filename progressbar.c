#include "progressbar.h"
#include "resource.h"

#include <commctrl.h>
#include <stdio.h>

#define DLG_TIMER_TOTOP 0


HWND hProgressBar;
HWND pbDlg;
extern QuickSlot quickslot[KEYCOUNT];
int lastGage;
int _nowIndex;
char *blockVar;
QuickSlot *pNowSlot;

BOOL CALLBACK ProgressDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam){
	int i;
	
	switch(iMessage){
		case WM_INITDIALOG:
			pbDlg=hDlg;
			//SetWindowLongPtr(hDlg,GWL_EXSTYLE,)
			SetWindowText(hDlg,pNowSlot->slotName);
			SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
			BringWindowToTop(hDlg);
			//SetWindowPos(GetForegroundWindow(),HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
			
//			hProgressBar=GetDlgItem(hDlg,DLG_PB_BAR);
//			SendMessage(hProgressBar,PBM_SETRANGE,0,MAKELPARAM(0,quickslot[_nowIndex].itemCount*2));
			SendDlgItemMessage(pbDlg,DLG_PB_BAR,PBM_SETRANGE,0,MAKELPARAM(0,pNowSlot->itemCount*2));
//			printf("quickslot[_nowIndex].itemCount*2: %d\n",pNowSlot->itemCount*2);
			return TRUE;
		case WM_COMMAND:
//			printf("dlg command: %d\n",wParam);
			switch(LOWORD(wParam)){
				case DLG_PB_CANCEL:
					*blockVar=1;
					if(MessageBox(hDlg,"프로그램 전개를 중지하겠습니까?","알림",MB_YESNO)==IDNO){
						*blockVar=0;
						return TRUE;
					}
					*blockVar=-1;
					DestroyWindow(hDlg);
					return TRUE;
			}
			break;
		case DM_CLOSE:
			DestroyWindow(hDlg);
			return TRUE;
	}
	return FALSE;
}
void StepBar(){
//	printf("start ");
	//printf("hProgressBar: %d\n",hProgressBar);
//	SendMessage(hProgressBar,PBM_SETSTEP,1,0);
//	printf("next ");
//	SendMessage(hProgressBar,PBM_STEPIT,0,0);
//	SendMessage(pbDlg,DM_STEP,0,0);
	SendDlgItemMessage(pbDlg,DLG_PB_BAR,PBM_SETSTEP,1,0);
	SendDlgItemMessage(pbDlg,DLG_PB_BAR,PBM_STEPIT,0,0);
//	printf("step\n");
}
void SetBlockVar(char *pVar){
	blockVar=pVar;
}
void SetNowSlot(QuickSlot *slot){
	pNowSlot=slot;
}
void SetNowIndex(int nowIndex){
	_nowIndex=nowIndex;
}
void SetNowLog(char *log){
	SetDlgItemText(pbDlg,DLG_ST_LOG,log);
	free(log);
}
