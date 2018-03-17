CC = gcc
CFLAGS = -Wall
TARGET = regex-vm
OBJS = main.o backtracking.o

all: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

main.o: main.c backtracking.h common.h
	$(CC) -c -o main.o main.c $(CFLAGS)	

backtracking.o: backtracking.c backtracking.h common.h
	$(CC) -c -o backtracking.o backtracking.c $(CFLAGS)

clean:
	rm -f $(TARGET)