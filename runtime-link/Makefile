CC = gcc
TARGET = prog

all: $(TARGET) libvector.so

$(TARGET): main.o
	$(CC) -rdynamic -o $(TARGET) main.o -ldl

libvector.so: addvec.o multvec.o
	$(CC) -shared -o libvector.so addvec.o multvec.o

main.o: main.c
	$(CC) -c -o main.o main.c

addvec.o: addvec.c
	$(CC) -c -fpic -o addvec.o addvec.c

multvec.o: multvec.c
	$(CC) -c -fpic -o multvec.o multvec.c

clean:
	rm $(TARGET) *.o *.so
