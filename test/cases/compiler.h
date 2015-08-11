#include <stdio.h>
int klock_CB(void (*cb_func)(void*));
int klock_setT(void* tv);
int klock_getT1(void* tv);
int klock_PC(int isAcquire, int id);

int klock_getT1(void* tv) {
	tv = malloc(4);
}

void* klock_getT2() {
	return malloc(4);
}

