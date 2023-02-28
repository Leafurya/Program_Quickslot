#ifndef _progressbar__h_
#define _progressbar__h_

#include <windows.h>
#include "quickslot.h"

BOOL CALLBACK ProgressDlgProc(HWND,UINT,WPARAM,LPARAM);
void StepBar();
void SetBlockVar(char *);

#endif
