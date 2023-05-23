all: async-tcpserver tcpclient

tcpclient: tcpclient.c
	gcc -o tcpclient tcpclient.c

async-tcpserver: async-tcpserver.c
	gcc -o async-tcpserver async-tcpserver.c 
clean:
	$(RM) tcpclient async-tcpserver 
