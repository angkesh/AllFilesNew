#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/ip.h>



/* This is a reference socket server implementation that prints out the messages
 * received from clients. */
int first_counter = 0;
int second_counter = 0;
#define MAX_PENDING 1000
#define MAX_LINE 100
#define MAX_CLIENTS 100

typedef struct{
    int fd;
    int Y;
    int Z;
    char status[20];
} client;

int handle_first_shake(client **c, struct sockaddr_in sin);
int handle_second_shake(client **c, int y, struct sockaddr_in sin);
void sendMessage(int s, int x);
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
        
    listen(s, MAX_PENDING);


    client client_state_arrray[MAX_CLIENTS];

    for(int k = 0; k <= MAX_CLIENTS; k++){
        client_state_arrray[k].fd = -1;
        client_state_arrray[k].Y = -1;
        client_state_arrray[k].Z = -1;
        strcpy(client_state_arrray[k].status, "Not Initiallized");
    }

    int fd_max = s;
    fd_set master_set;
    fd_set ready_set;

    int new_s;
    int y;
    int z;
    socklen_t len = sizeof(sin);

    FD_ZERO(&master_set);
    fcntl(s, F_SETFL, O_NONBLOCK);
    FD_SET(s, &master_set);
    int client_count = 0;

    while(1){

        fd_set ready_set;
        FD_ZERO(&ready_set);
        ready_set = master_set;


        int res_select = pselect(fd_max+1, &ready_set, NULL, NULL, NULL, NULL);

		if(res_select < 0)
		{
			perror("pselect");
			exit(1);
		}
		else if(res_select == 0)
			continue;
        
        for(int i = 0; i <= fd_max; i++){

            if(FD_ISSET(i, &ready_set)){
               
                if(i == s){
                    
                    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
                        perror("simplex-talk: accept");
                        exit(1);
                    }
                    
                    
                    
                    fcntl(new_s, F_SETFL, O_NONBLOCK);
                    FD_SET(new_s, &master_set);
                    
                    client_state_arrray[client_count].fd = new_s;
                   
                    strcpy(client_state_arrray[client_count].status, "handle first shake");
                    
                    client_count++;

                    if(new_s > fd_max){
                        fd_max = new_s;
                    }
  
                }

                else{
                 
                    int j = -1;
                    
                    while((client_state_arrray[++j].fd) != i);
                    
                    client *cp = &client_state_arrray[j];
                    client **p = &cp;
                    
                    if(strcmp(client_state_arrray[j].status, "handle first shake") == 0){
                       
                        
                        y = handle_first_shake(p, sin);
                        
                        if(y<0){
                            break;
                        }
                        else{
                            
                            client_state_arrray[j].Y = y;
                        }

                       
                    }
                    else{
                       
                        z = handle_second_shake(p, client_state_arrray[j].Y, sin);
                        
                        if(z<0){
                            break;
                        }else{
                            client_state_arrray[j].Z = z;
                        }
                        close(i);
                        FD_CLR(i, &master_set);
                        
                        client_state_arrray[j].fd = -1;
                        strcpy(client_state_arrray[j].status, "Not Initiallized");
                       
                    }

                    
                }

                if(i == fd_max){
                        
                    break;
                }
                
            }

            else{
                continue;
            }     
        }

    }

    return 0;
}





int handle_first_shake(client **c, struct sockaddr_in sin){
    first_counter++;
    socklen_t len = sizeof(sin);
    int y;
    int s = (*c)->fd;
    char buf[MAX_LINE];
    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));

    
    len = recv(s, buf, sizeof(buf), 0);
    
    if(len < 0){
        perror("recv");
        return len;
    }
    else{


        strcat(buf, "\n");
        fputs(buf, stdout);
        fflush(stdout);

        y = findInteger(buf);
        y = y+1;
        

        sendMessage(s, y);

        strcpy((*c)->status, "handle second shake");
        return y;
    }
    
}


int handle_second_shake(client **c, int y, struct sockaddr_in sin){
    second_counter++;
    socklen_t len = sizeof(sin);
    int z;
    int s = (*c)->fd;
    char buf[MAX_LINE];
    memset(buf, '\0', MAX_LINE*sizeof(buf[0]));

    len = recv(s, buf, sizeof(buf), 0);
    if(len < 0){
        perror("recv");
        return len;
    }
    else{
        

        strcat(buf, "\n");
        fputs(buf, stdout);
        fflush(stdout);

      
        z = findInteger(buf);

        if(z != (y+1)){

            fputs("ERROR", stdout);
            fputc('\n', stdout);
            fflush(stdout);
        }  
    }
   
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