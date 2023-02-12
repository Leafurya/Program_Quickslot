#ifndef _ctrlmanager__h_
#define _ctrlmanager__h_

#ifdef _cplusplus
extern "C"{
#endif

#include <windows.h>

typedef struct _ctrlgroup{
	int groupId;
	void (*MoveFunc)(void *,RECT);
	int ctrlCount;
	HWND *ctrls;
}CtrlGroup;

typedef struct _ctrlmanager{
	CtrlGroup *ctrlGroups;
	CtrlGroup *nowGroup;
	RECT *rect;
	int ctrlGroupCount;
}CtrlManager;

void InitCtrlManager(CtrlManager *cm,RECT *rt);
void RegistCtrlGroup(CtrlManager *cm,void *ctrlGroup,int groupId,int size,void (*MoveFunc)(void *,RECT));
char ChangeCtrlGroup(CtrlManager *cm,int groupId);
void ShowCtrlGroup(CtrlGroup cm);
void HideCtrlGroup(CtrlGroup cm);
CtrlGroup *GetCtrlGroupById(CtrlManager cm,int groupId);
char CallMoveFunc(CtrlManager cm,RECT rt);
void DestroyCtrlManager(CtrlManager *cm);
void SetNowCtrlGroup(CtrlManager *cm,int id);

#ifdef _cplusplus
}
#endif
#endif
