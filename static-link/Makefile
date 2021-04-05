CC = gcc
TARGET = prog

all: $(TARGET)

$(TARGET): main.o libvector.a
	$(CC) -static -o $(TARGET) main.o -L. -lvector

libvector.a: addvec.o multvec.o
	ar rs libvector.a addvec.o multvec.o

main.o: main.c vector.h
	$(CC) -c -o main.o main.c

addvec.o: addvec.c
	$(CC) -c -o addvec.o addvec.c

multvec.o: multvec.c
	$(CC) -c -o multvec.o multvec.c

clean:
	rm $(TARGET) *.o *.a
