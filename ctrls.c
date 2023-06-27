#include "ctrls.h"

int fontSize=15;
char *fontName=(char *)"맑은 고딕";
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
	int i=0;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	HMENU btSlotID;
	
	c->liItems=CreateWindow("listbox",NULL,LBS_NOTIFY|WS_VSCROLL|WS_CHILD|WS_BORDER|LBS_MULTIPLESEL|LBS_NOINTEGRALHEIGHT,0,0,0,0,hWnd,(HMENU)SAVECTRLS_LI_ITEMS,hInst,NULL);
	c->btSave=CreateWindow("button","저장",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_SAVE,hInst,NULL);
	c->btChange=CreateWindow("button","교체",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_CHANGE,hInst,NULL);
	c->btModi=CreateWindow("button","수정",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_MODI,hInst,NULL);
	c->btRemove=CreateWindow("button","삭제",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_REMOVE,hInst,NULL);
	c->btRefind=CreateWindow("button","다시 감지",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_REFIND,hInst,NULL);
	
	c->stInfo=CreateWindow("static","test text",WS_CHILD|SS_LEFT|WS_VISIBLE,0,0,0,0,hWnd,(HMENU)-1,hInst,NULL);
	c->cbDetecting=CreateWindow("button","감지하기",WS_CHILD|WS_VISIBLE |BS_CHECKBOX,0,0,0,0,hWnd,(HMENU)SAVECTRLS_CB_DETECTING,hInst,NULL);
	
	c->btAddItem=CreateWindow("button","아이템 추가",WS_CHILD|WS_BORDER|WS_VISIBLE,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_ADDITEM,hInst,NULL);
	
	SetFontAll((HWND *)c,sizeof(SaveCtrls),font);
}
void MoveSaveCtrls(void *ctrls,RECT rect){
	int i;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	int btW=(int)(rect.right/FKEYCOUNT);
	int btH=25;//50
	int x=0;
	int underBtW=(int)(rect.right/3),underBtH=40,underBtY=rect.bottom-underBtH;
	int liW=200,liH=rect.bottom-btH-underBtH;
	int cbW=80,cbH=40;
	
	int margin=20;
	int stW=rect.right-liW-(margin*2), stH=liH-(margin*2)-cbH;
	int stX=liW+margin, stY=margin+cbH;
	
//	for(i=0;i<FKEYCOUNT;i++,x+=btW){
//		MoveWindow(c->btSlot[i],x,0,btW,btH,TRUE);
//	}
	MoveWindow(c->btAddItem,0,0,liW,btH,TRUE); 
	MoveWindow(c->liItems,0,btH,liW,liH,TRUE);
	MoveWindow(c->cbDetecting,stX,margin,cbW,cbH,TRUE);
	MoveWindow(c->stInfo,stX,stY,stW,stH,FALSE);
	
	x=0;
	MoveWindow(c->btSave,x,underBtY,underBtW,underBtH,TRUE);
	MoveWindow(c->btChange,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btModi,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btRemove,x,underBtY,underBtW,underBtH,TRUE);
	MoveWindow(c->btRefind,x,underBtY,underBtW,underBtH,TRUE);
}
