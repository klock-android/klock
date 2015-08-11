#include "compiler.h"
int *a = 0;
void getTIssue1() {
	a = (int*)klock_getT2();
}

int main(){
	int* b;
	b = (int*)klock_getT2();
	return (*a - *b);
}
