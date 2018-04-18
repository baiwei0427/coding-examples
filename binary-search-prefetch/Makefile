all: main.c
	gcc main.c -DDO_PREFETCH -o with-prefetch -std=c11 -O3
	gcc main.c -o no-prefetch -std=c11 -O3

clean:
	rm -f with-prefetch no-prefetch