#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

int main (int argc, char* argv[]) {

    //Process command line arguments
    if(argc < 6) {
        printf("Give 6 command line arguments!");
        exit(1);
    }    
    int portNum = argv[1];
    int numWorkers = argv[2];
    if(numWorkers <= 0) {
        printf("Number of worker threads should be positive!");
        exit(1);        
    }
    int bufferSize = argv[3];
    if(bufferSize <= 0) {
        printf("Buffer size should be positive!");
        exit(1);        
    }    
    char* poll_log= malloc(strlen(argv[4]) + 1);
    strcpy(poll_log,argv[4]);    
    char* poll_stats = malloc(strlen(argv[5]) + 1);
    strcpy(poll_stats,argv[5]);    

    //Create Socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        perror_exit("Failed to create socket in server!");
    struct sockaddr_in server;    
    server.sin_family = AF_INET;                        /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portNum);                   /* The given port */
        
    //Bind
    if(bind(sock, (struct sockaddr *) &server,sizeof (server)) == -1)
        perror_exit("Error in binding !");
    int* t_ids = (int*) malloc(numWorkers * sizeof(int));
    //Listen
    if(listen(sock,bufferSize) < 0)     //MAYBE BACKLOG SIZE SHOULD BE CHANGED
        perror_exit("Error in listening!");
    //Accept
    //Read
    //Write
    free(poll_log);
    free(poll_stats);
    free(t_ids);
    return 0;
}