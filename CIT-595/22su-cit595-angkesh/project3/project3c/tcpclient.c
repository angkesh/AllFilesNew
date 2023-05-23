#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 100

void sendMessage(int s, int x);
void receiveMessage(int s, int x, struct sockaddr_in sin);
int findInteger(char* message);

int main (int argc, char *argv[]) {
  char* host_addr = argv[1];
  int port = atoi(argv[2]);
  int x = atoi(argv[3]);

  /* Open a socket */
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0){
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

  /* Connect to the server */
  if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }

  /*main loop: get and send lines of text */
  
  
  sendMessage(s, x);

  
  receiveMessage(s, x, sin);


  return 0;
}



void sendMessage(int s, int x){

    char buf[MAX_LINE];

    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));

    strcpy(buf, "HELLO ");
   
    char str[100];

    sprintf(str, "%d", x);
    
    strcat(buf, str);

    int len = strlen(buf)+1;
    
    send(s, buf, len, 0);

}


void receiveMessage(int s, int x, struct sockaddr_in sin){
    char buf[MAX_LINE];
    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));

    socklen_t message = sizeof(sin);


    //recieve the message from the server
    message = recv(s, buf, sizeof(buf), 0);

    strcat(buf, "\n");

    //print the message
    fputs(buf, stdout);

    //flush
    fflush(stdout);

    //find the integer reicieved from the server
    int y = findInteger(buf);

    if (y != (x+1)){
        fputs("ERROR", stdout);
        fputc('\n', stdout);
        fflush(stdout);
        
    }else{
        int z = y + 1;
        struct timeval timestamp;
        gettimeofday(&timestamp, NULL); // get current timestamp
        srand((int)timestamp.tv_usec);
        int ranSleep_ms = (rand() % 100);
        usleep(1000 * ranSleep_ms);

        sendMessage(s, z);
    }
   
    close(s);
}


int findInteger(char *message){

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