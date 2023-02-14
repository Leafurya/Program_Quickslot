#include "ctrls.h"

int fontSize=12;
char *fontName=(char *)"굴림";
HFONT font;
char *keys[FKEYCOUNT]={"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"};

void CreateCtrlFont(){
	font=CreateFont(fontSize,0,0,0,0,0,0,0,HANGEUL_CHARSET,0,0,0,VARIABLE_PITCH|FF_ROMAN,fontName);
}
	void SetFontEach(HWND c,HFONT font){
		SendMessage(c,WM_SETFONT,(WPARAM)font,MAKELPARAM(TRUE,0));
	}
	void SetFontAll(HWND *c,int size,HFONT font){
		int i;
		int count=size/sizeof(HWND);
		for(i=0;i<count;i++){
			SendMessage(c[i],WM_SETFONT,(WPARAM)font,MAKELPARAM(TRUE,0));
		}
	}
void CreateSaveCtrls(void *ctrls,HWND hWnd,HINSTANCE hInst){
	int i;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	for(i=0;i<FKEYCOUNT;i++){
		c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)(SAVECTRLS_BT_ORIGIN+i),hInst,NULL);
	}
	c->liItems=CreateWindow("listbox",NULL,LBS_NOTIFY|WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_LI_ITEMS,hInst,NULL);
	c->btSave=CreateWindow("button","저장",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_SAVE,hInst,NULL);
	c->btModi=CreateWindow("button","수정",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_MODI,hInst,NULL);
	c->btRemove=CreateWindow("button","삭제",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_REMOVE,hInst,NULL);
	
	c->stInfo=CreateWindow("static","test text",WS_CHILD|SS_LEFT|WS_VISIBLE,0,0,0,0,hWnd,(HMENU)-1,hInst,NULL);
//	for(i=0;i<3;i++){
//		c->stInfo[i]=CreateWindow("static","test text",WS_CHILD|SS_LEFT,0,0,0,0,hWnd,(HMENU)0,hInst,NULL);
//	}
	
	SetFontAll((HWND *)c,sizeof(SaveCtrls),font);
}
void MoveSaveCtrls(void *ctrls,RECT rect){
	int i;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	int btW=(int)(rect.right/FKEYCOUNT);
	int btH=50;
	int x=0;
	int underBtW=(int)(rect.right/3),underBtH=40,underBtY=rect.bottom-underBtH;
	int liW=200,liH=rect.bottom-btH-underBtH;
	
//	int margin=20;
//	int stW=rect.right-liW, stH=20;
//	int stX=liW+margin, stY=btH+(liH/2)-(stH/2)-stH;
	
	int margin=20;
	int stW=rect.right-liW-(margin*2), stH=80;
	int stX=liW+margin, stY=btH+(liH/2)-(stH/2);
	
	for(i=0;i<FKEYCOUNT;i++,x+=btW){
		MoveWindow(c->btSlot[i],x,0,btW,btH,TRUE);
	}
	MoveWindow(c->liItems,0,btH,liW,liH,TRUE);
	MoveWindow(c->stInfo,stX,stY,stW,stH,FALSE);
//	for(i=0;i<3;i++,stY+=stH){
//		MoveWindow(c->stInfo[i],stX,stY,stW,stH,FALSE);
//	}
	
	x=0;
	MoveWindow(c->btSave,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btModi,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btRemove,x,underBtY,underBtW,underBtH,TRUE);
}
