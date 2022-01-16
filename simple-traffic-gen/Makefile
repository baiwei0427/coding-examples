.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g
APPS = traffic_gen_client traffic_gen_server
OBJS = client.o server.o common.o
DEPS = common.h

all: ${APPS}

traffic_gen_client: client.o common.o
	$(CC) $(CFLAGS) -o $@ $^

traffic_gen_server: server.o common.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(APPS)
