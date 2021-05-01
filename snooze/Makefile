TARGET = snooze
CC = gcc

all: $(TARGET)

$(TARGET): main.o
	$(CC) -o $(TARGET) main.o

main.o: main.c
	$(CC) -c -o main.o main.c

clean:
	rm -f *.o $(TARGET)
