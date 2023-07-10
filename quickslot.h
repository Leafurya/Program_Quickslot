#ifndef _quickslot__h_
#define _quickslot__h_

#include "data.h"

#define ITEM_MAXSIZE	10
#define KEYCOUNT	12

#define STATUS_MASK				3
#define EXECUTE_FAIL			1
#define FINDING_FAIL			2
#define NOW_DATA_VERSION		2

typedef struct _quickslotv0{
	char itemCount;
	char slotName[256];
	ItemV0 item[ITEM_MAXSIZE];
}QuickSlotV0;
typedef struct _quickslotforfindingv0{
	char itemCount;
	char slotName[256];
	ItemV0 item[100];
}QuickSlotForFindingV0;

typedef struct _quickslotv1{
	char itemCount;
	char slotName[256];
	ItemV1 item[ITEM_MAXSIZE];
}QuickSlotV1;
typedef struct _quickslotforfindingv1{
	char itemCount;
	char slotName[256];
	ItemV1 item[100];
}QuickSlotForFindingV1;

typedef struct _quickslotv2{
	char itemCount;
	char slotName[256];
	ItemV2 item[ITEM_MAXSIZE];
}QuickSlotV2;
typedef struct _quickslotforfindingv2{
	char itemCount;
	char slotName[256];
	ItemV2 item[100];
}QuickSlotForFindingV2;

typedef QuickSlotV2 QuickSlot;
typedef QuickSlotForFindingV2 QuickSlotForFinding;

//char LoadQuickslot(QuickSlot (*)[],int);
//char SaveQuickslot(QuickSlot *,int);
int GetSlotIndex();
char SpreadQuickslot(QuickSlot *,int,char *[]);
void ShowItemList(Item *,int,HWND);
void ShowItemInfo(char *,Item *,HWND,HWND);
void ShowSlotData(QuickSlot *);
void CloseSlot(QuickSlot *);
char IsSlotOpened(QuickSlot);
void ForegroundSlot(QuickSlot);
BOOL FilterWindow(HWND);

BOOL IsUselessWindow(char *);
BOOL GetProcessPath(char *,HWND);

BOOL CALLBACK GetOpenedWindowProc(HWND,LPARAM);

char LoadQuickslot(QuickSlot (*)[],int);
char SaveQuickslot(QuickSlot *,int);
void ShowSlotData(QuickSlot *);
void CheckVersion();

Item OpenItem(char *,char *,char *);

#endif
