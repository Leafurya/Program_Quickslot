#ifndef _progressbar__h_
#define _progressbar__h_

#include <windows.h>
#include "quickslot.h"

#define DM_CLOSE	WM_USER+21

BOOL CALLBACK ProgressDlgProc(HWND,UINT,WPARAM,LPARAM);
void StepBar();
void SetBlockVar(char *);
void SetNowIndex(int);
void SetNowLog(char *);
void SetNowSlot(QuickSlot *);

#endif
