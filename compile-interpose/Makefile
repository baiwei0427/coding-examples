TARGET = intc

all: $(TARGET)

$(TARGET): main.o mymalloc.o
	gcc -o $(TARGET) main.o mymalloc.o

main.o: main.c malloc.h
	gcc -c -I. -o main.o main.c

mymalloc.o: mymalloc.c
	gcc -c -o mymalloc.o mymalloc.c

clean:
	rm -f *.o $(TARGET)
