#ifndef _quickslot__h_
#define _quickslot__h_

#include "data.h"

#define ITEM_MAXSIZE	10
#define KEYCOUNT	12

#define STATUS_MASK				3
#define EXECUTE_FAIL			1
#define FINDING_FAIL			2
#define NOW_DATA_VERSION	1

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

typedef QuickSlotV1 QuickSlot;
typedef QuickSlotForFindingV1 QuickSlotForFinding;

char LoadQuickslot(QuickSlot (*)[],int);
char SaveQuickslot(QuickSlot *,int);
void ShowSlotData(QuickSlot *);
void CheckVersion();

#endif
