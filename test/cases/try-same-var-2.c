#include "compiler.h"
int getTIssue(int x) {
	int *a, *b;
	int c = 30;
	klock_getT1(b);
	a = b;
	while(c > 20) {
		klock_getT1(b);
		c = *a - *b;
		a = b;
	}
	return c;
}
int main(){
	int a;
	scanf("%d", &a);
	a = getTIssue(a);
	return a;
}
