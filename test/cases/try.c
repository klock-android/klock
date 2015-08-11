#include "compiler.h"
//int setTIssue(int a) {
//	int b = 0;
//	a += 4;
//	b += 15;
//	klock_setT((void*)&a);
//	return b;
//}

int getTIssue() {
	int* a = (int*)klock_getT2();
	//int* b = (int*)klock_getT2();
	int b = 14;
	int c = *a - b;
	//c = klock_getT2();
	return c;
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
	int a = 14;
	a = getTIssue();
//	setTIssue(a);
//	setGetIssue(a);
//	getTCorr();
	return a;
}
