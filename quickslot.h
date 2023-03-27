#ifndef _quickslot__h_
#define _quickslot__h_

#include "data.h"

#define ITEM_MAXSIZE	10
#define KEYCOUNT	12

#define STATUS_MASK				3
#define EXECUTE_FAIL			1
#define FINDING_FAIL			2

typedef struct _quickslot{
	Item item[ITEM_MAXSIZE];
	char slotName[256];
	char itemCount;
}QuickSlot;

char LoadQuickslot(QuickSlot (*)[],int);
char SaveQuickslot(QuickSlot *,int);
int GetSlotIndex();
char SpreadQuickslot(QuickSlot *,int,char *[]);
void ShowItemList(QuickSlot,HWND);
void ShowItemInfo(char *,Item *,HWND);
void ShowSlotData(QuickSlot *);
void CloseSlot(QuickSlot *);
char IsSlotOpened(QuickSlot);
void ForegroundSlot(QuickSlot);
BOOL FilterWindow(HWND);

BOOL IsUselessWindow(char *);
BOOL GetProcessPath(char *,HWND);

BOOL CALLBACK GetOpenedWindowProc(HWND,LPARAM);

#endif
