#include <process.h>
#include "thread.h"
#include <stdio.h>

#include <stdlib.h>


HANDLE StartThread(unsigned (__stdcall *ThreadFunc)(void *arg),void *pArg){
	HANDLE handle;
	
	handle=(HANDLE)_beginthreadex(NULL,0,ThreadFunc,pArg,0,NULL);
	return handle;
}
