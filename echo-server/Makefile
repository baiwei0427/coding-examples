all: tcpcli tcpserv
tcpcli: tcpcli.c unp.h
	gcc -o tcpcli tcpcli.c unp.h
tcpserv: tcpserv.c unp.h
	gcc -o tcpserv tcpserv.c unp.h
clean:
	rm tcpserv tcpcli
