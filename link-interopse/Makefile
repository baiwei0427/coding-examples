TARGET = intl

all: $(TARGET)

$(TARGET): main.o mymalloc.o
	gcc -Wl,--wrap,malloc -Wl,--wrap,free -o $(TARGET) main.o mymalloc.o

main.o: main.c
	gcc -c -o main.o main.c

mymalloc.o: mymalloc.c
	gcc -c -o mymalloc.o mymalloc.c

clean:
	rm -f *.o $(TARGET)
