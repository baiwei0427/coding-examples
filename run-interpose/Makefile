TARGET = intr

all: $(TARGET) mymalloc.so

run:
	LD_PRELOAD="./mymalloc.so" ./$(TARGET)

$(TARGET): main.c
	gcc -o $(TARGET) main.c

mymalloc.so: mymalloc.c
	gcc -shared -fpic -o mymalloc.so mymalloc.c -ldl

clean:
	rm -f mymalloc.so $(TARGET)
