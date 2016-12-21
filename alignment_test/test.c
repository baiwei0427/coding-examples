#include <stdio.h>
#include <stdlib.h>

struct test {
	char a, b, c;
	int d;
	char f;
	long e;
};

int main() {
	struct test t;
	int x1 = (unsigned int)(void*)&(t.a) - (unsigned int)(void*)&t;
	int x2 = (unsigned int)(void*)&(t.b) - (unsigned int)(void*)&t;
	int x3 = (unsigned int)(void*)&(t.c) - (unsigned int)(void*)&t;
	int x4 = (unsigned int)(void*)&(t.d) - (unsigned int)(void*)&t;
	int x5 = (unsigned int)(void*)&(t.e) - (unsigned int)(void*)&t;

	printf("a:%d b:%d c:%d d:%d e:%d\n", x1, x2, x3, x4, x5);
	printf("%d\n", sizeof(t));
	return 0;
}