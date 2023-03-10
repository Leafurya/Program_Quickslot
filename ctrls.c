#include "ctrls.h"

int fontSize=15;
char *fontName=(char *)"???? ????";
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
	
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F1,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F2,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F3,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F4,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F5,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F6,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F7,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F8,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F9,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F10,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F11,hInst,NULL);i++;
//	c->btSlot[i]=CreateWindow("button",keys[i],WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_F12,hInst,NULL);
	
	c->liItems=CreateWindow("listbox",NULL,LBS_NOTIFY|WS_VSCROLL|WS_CHILD|WS_BORDER|LBS_MULTIPLESEL|LBS_NOINTEGRALHEIGHT,0,0,0,0,hWnd,(HMENU)SAVECTRLS_LI_ITEMS,hInst,NULL);
	c->btSave=CreateWindow("button","????",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_SAVE,hInst,NULL);
	c->btFind=CreateWindow("button","??ü",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_FIND,hInst,NULL);
	c->btModi=CreateWindow("button","????",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_MODI,hInst,NULL);
	c->btRemove=CreateWindow("button","????",WS_CHILD|WS_BORDER,0,0,0,0,hWnd,(HMENU)SAVECTRLS_BT_REMOVE,hInst,NULL);
	
	c->stInfo=CreateWindow("static","test text",WS_CHILD|SS_LEFT|WS_VISIBLE,0,0,0,0,hWnd,(HMENU)-1,hInst,NULL);
	
	SetFontAll((HWND *)c,sizeof(SaveCtrls),font);
}
void MoveSaveCtrls(void *ctrls,RECT rect){
	int i;
	SaveCtrls *c=(SaveCtrls *)ctrls;
	int btW=(int)(rect.right/FKEYCOUNT);
	int btH=0;//50
	int x=0;
	int underBtW=(int)(rect.right/3),underBtH=40,underBtY=rect.bottom-underBtH;
	int liW=200,liH=rect.bottom-btH-underBtH;
	
	int margin=20;
	int stW=rect.right-liW-(margin*2), stH=liH-(margin*2);
	int stX=liW+margin, stY=btH+margin;
	
//	for(i=0;i<FKEYCOUNT;i++,x+=btW){
//		MoveWindow(c->btSlot[i],x,0,btW,btH,TRUE);
//	}
	MoveWindow(c->liItems,0,btH,liW,liH,TRUE);
	MoveWindow(c->stInfo,stX,stY,stW,stH,FALSE);
	
	x=0;
	MoveWindow(c->btSave,x,underBtY,underBtW,underBtH,TRUE);
	MoveWindow(c->btFind,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btModi,x,underBtY,underBtW,underBtH,TRUE);
	x+=underBtW;
	MoveWindow(c->btRemove,x,underBtY,underBtW,underBtH,TRUE);
}
