#include "compiler.h"
//int setTIssue(int a) {
//	int b = 0;
//	a += 4;
//	b += 15;
//	klock_setT((void*)&a);
//	return b;
//}

int getTIssue2(int y) {
	int *a;
	a = (int*)klock_getT2();
	return *a - y;
}

int getTIssue1(int x) {
	int *a;
	a = (int*)klock_getT2();
	return *a + x;
}

//void setGetIssue(int x) {
//	klock_setT((void*)&x);
//}
//
//int getTCorr() {
//	int a = klock_getT2();
//	int b = klock_getT2();
//	b = 59;
//	return a+b;
//}

int main(){
	int a, b;
	scanf("%d", &a);
	a = getTIssue1(a);
	b = getTIssue2(a);
//	setTIssue(a);
//	setGetIssue(a);
//	getTCorr();
	return (a-b);
}
