#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 10
#define MAX_LINE 100

void sendMessage(int s, int x);
void recieveMessage(int s, struct sockaddr_in sin);
int findInteger(char* message);

int main(int argc, char *argv[]) {
  
  char* host_addr = "127.0.0.1";
 
  int port = atoi(argv[1]);
  
  
  /*setup passive open*/
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr(host_addr);
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Bind the socket to the address */
  if((bind(s, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  //listen(s, MAX_PENDING);  

  /* wait for connection, then receive and print text */

  
  recieveMessage(s, sin);
  

  return 0;
}



void sendMessage(int s, int x){
    char buf[MAX_LINE] ;
    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));
    strcpy(buf, "HELLO ");

    
    char str[100];
    sprintf(str, "%d", x);
    strcat(buf, str);
    int len = strlen(buf)+1;
   
    send(s, buf, len, 0);

}


void recieveMessage(int s, struct sockaddr_in sin){

    int new_s;
    socklen_t len = sizeof(sin);
    int count = 1;
    int y;
    int z;
    listen(s, MAX_PENDING);
     

    while(1) {
         
        char buf[MAX_LINE];
        memset(buf, '\0', MAX_LINE*sizeof(buf[0]));
       
        if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
            perror("simplex-talk: accept");
            exit(1);
        }
        
        while(len = recv(new_s, buf, sizeof(buf), 0)){
            strcat(buf, "\n");
            fputs(buf, stdout);
            fflush(stdout);
            

            if(count == 1){

                count++;
                y = findInteger(buf);
                y = y+1;
                sendMessage(new_s, y); 
            }
            else{

                count--;
                z = findInteger(buf);

                if(z != (y+1)){

                    fputs("ERROR", stdout);
                    fputc('\n', stdout);
                    fflush(stdout);
                }  
            }
        }
        
        
        close(new_s);   
  }
}


int findInteger(char* message){

    char *token = strtok(message, " ");
    char *arr[2];
    int i = 0;
    while(token != NULL){
        
        arr[i++] = token;
        token = strtok(NULL, " ");
    }

    int y = atoi(arr[1]);
    return y;
}