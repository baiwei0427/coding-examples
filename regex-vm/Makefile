CC = g++
CFLAGS = -Wall
TARGET = regex-vm
OBJS = main.o backtracking.o thompson.o

all: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

main.o: main.cpp backtracking.h common.h
	$(CC) -c -o main.o main.cpp $(CFLAGS)	

backtracking.o: backtracking.cpp backtracking.h common.h
	$(CC) -c -o backtracking.o backtracking.cpp $(CFLAGS)

thompson.o: thompson.cpp thompson.h common.h
	$(CC) -c -o thompson.o thompson.cpp $(CFLAGS)

clean:
	rm -f $(TARGET)