#ifndef _quickslot__h_
#define _quickslot__h_

#include "data.h"

#define ITEM_MAXSIZE	10
#define KEYCOUNT	12

typedef struct _quickslot{
	Item item[ITEM_MAXSIZE];
	char slotName[256];
	char itemCount;
}QuickSlot;
//slot key is VK-VK_F1

char LoadQuickslot(QuickSlot (*)[],int);
char SaveQuickslot(QuickSlot *,int);
int GetSlotIndex();
char SpreadQuickslot(QuickSlot *,int);
void ShowItemList(QuickSlot,HWND);
void ShowItemInfo(char *,Item,HWND);
void ShowSlotData(QuickSlot *);

#endif
