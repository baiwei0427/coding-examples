CC = g++
CFLAGS = -I. -lpcre -Wall
TARGET = detect-jit

all: main.cpp
	$(CC) -o $(TARGET) main.cpp $(CFLAGS)

clean:
	rm -f $(TARGET)