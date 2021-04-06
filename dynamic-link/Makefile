CC = gcc
TARGET = prog

all: $(TARGET)

$(TARGET): main.o libvector.so
	$(CC) -o $(TARGET) main.o ./libvector.so

libvector.so: addvec.o multvec.o
	$(CC) -shared -o libvector.so addvec.o multvec.o

main.o: main.c vector.h
	$(CC) -c -o main.o main.c

addvec.o: addvec.c
	$(CC) -c -fpic -o addvec.o addvec.c

multvec.o: multvec.c
	$(CC) -c -fpic -o multvec.o multvec.c

clean:
	rm $(TARGET) *.o *.so
