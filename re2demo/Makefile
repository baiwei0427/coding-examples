CC = g++
CFLAGS = -I/usr/local/include/re2 -std=c++11 -Wall -lre2 
TARGET = re2demo

all: main.cpp
	$(CC) -o $(TARGET) main.cpp $(CFLAGS)

clean:
	rm -f $(TARGET)