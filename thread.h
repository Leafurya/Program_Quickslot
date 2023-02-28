#ifndef _thread__h_
#define _thread__h_

#include <windows.h>

HANDLE StartThread(unsigned (__stdcall *ThreadFunc)(void *arg),void *pArg);

#endif
