all: multi-tcpserver tcpclient

tcpclient: tcpclient.c
	gcc -o tcpclient tcpclient.c

multi-tcpserver: multi-tcpserver.c
	gcc -pthread -o multi-tcpserver multi-tcpserver.c 
clean:
	$(RM) tcpclient multi-tcpserver 
