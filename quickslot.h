#ifndef _quickslot__h_
#define _quickslot__h_

#include "data.h"
#define SLOT_MAXSIZE	10

typedef struct _quickslot{
	Data slotData[SLOT_MAXSIZE];
	char itemCount;
}QuickSlot, *LPQuickSlot;
//slot key is VK-VK_F1

void LoadSlot(LPQuickSlot);

#endif
