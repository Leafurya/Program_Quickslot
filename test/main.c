#include <stdio.h>
#include <stdlib.h>
#include "quickslot.h"


QuickSlot slot[KEYCOUNT];

int main() {
	CheckVersion();
	LoadQuickslot(&slot,sizeof(slot));
	ShowSlotData(slot);
	SaveQuickslot(slot,sizeof(slot));
	return 0;
}
