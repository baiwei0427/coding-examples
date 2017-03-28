obj-m += netlink-latency-kernel.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o netlink-latency-user -lrt netlink-latency-user.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
