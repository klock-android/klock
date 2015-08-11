#include "compiler.h"
int *a;
void getTIssue1() {
	a = (int*)klock_getT2();
}

int main(){
	int* b;
	getTIssue1(a);
	b = (int*)klock_getT2();
	return (*a - *b);
}
