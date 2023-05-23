#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 100
#define MAX_LINE 100

struct thread_values{
    int socket;
    socklen_t len;
};

void sendMessage(int s, int x);
void acceptMessage(int s, struct sockaddr_in sin);
void *recieveMessage(void *arg);
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

  

  /* wait for connection, then receive and print text */

  
  acceptMessage(s, sin);



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


void acceptMessage(int s, struct sockaddr_in sin){

    pthread_t tids[100];
    int new_s;
    socklen_t len = sizeof(sin);
    int i = 0;

    

    listen(s, MAX_PENDING);
     

    while(1) {
         
        
        if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
            perror("simplex-talk: accept");
            exit(1);
        }
        
        struct thread_values *args;
        args = malloc(sizeof(struct thread_values));
        args->socket = new_s;
        args->len = len;
        

       
       
        pthread_create(&tids[i++], NULL, recieveMessage, (void*) args);


  }
  
  pthread_exit(NULL);
}



void *recieveMessage(void *args){

        
    
    int count = 1;
    int y;
    int z;
    struct thread_values *thread = ((struct thread_values*) args);
    int *s =  &thread->socket;
    socklen_t *len = &thread->len;
    
    char buf[MAX_LINE];
    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));
   
    while(*len = recv(*s, buf, sizeof(buf), 0)){
            
        strcat(buf, "\n");
        fputs(buf, stdout);
        fflush(stdout);
            

        if(count == 1){

            count++;
            y = findInteger(buf);
            y = y+1;
            sendMessage(*s, y); 
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
    
    close(thread->socket);
    free(args);
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