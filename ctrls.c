#include "ctrls.h"

int fontSize=12;
char *fontName=(char *)"±¼¸²";
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
}
void MoveSaveCtrls(void *ctrls,RECT rect){
	int i;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	int btW=(int)(rect.right/FKEYCOUNT);
	int btH=50;
	int x=0;
	
	for(i=0;i<FKEYCOUNT;i++,x+=btW){
		MoveWindow(c->btSlot[i],x,0,btW,btH,TRUE);
	}
}
