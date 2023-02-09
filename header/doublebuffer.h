#ifndef _doublebuffer__h_
#define _doublebuffer__h_

#ifdef __cplusplus
extern "C"{
#endif

#include <windows.h>

void DoubleBuffer(HDC hdc,RECT rt,void (*DrawFunc)(HDC,RECT));

#ifdef __cplusplus
}
#endif
#endif
