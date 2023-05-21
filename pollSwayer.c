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
#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

void* ask_server(void* arg);

typedef struct  {

    char line[200];   
    struct sockaddr_in server;
    int sock;

} info;

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

int main (int argc, char* argv[]) {

    //Process input arguments
    if(argc < 4) {
        printf("Give 4 command line arguments!");
        exit(1);
    }
    int portNum = argv[2];
    info connection;
    FILE* inputFile = fopen(argv[3],"r");
    if (inputFile == NULL) {
        printf("Error: could not open file %s", argv[3]);
        exit(1);
    }    

    if ((connection.sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        perror_exit("Failed to create socket in client!");

    //Get hostname as IP address    
    struct hostent *server_ip ;
    if ( (server_ip = gethostbyname(argv[1])) == NULL)
        perror_exit("Can't resolve hostname!");;

    struct sockaddr_in server ;
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, server_ip->h_addr, server_ip->h_length);    
    server.sin_port = htons(portNum);
    connection.server = server ;

    //For every line in the file we create a thread that attempts to connect to server and write the line to
    char line[200];
    int err;
    int* t_ids = (int*) malloc(200 * sizeof(int));
    int line_num = 0;
    int capacity = 200 ;
    while (fgets(line, sizeof(line), inputFile) != NULL) {  

        strcpy(connection.line,line);      

        if(err = pthread_create(t_ids + line_num, NULL, ask_server, (void*) &connection)) {
            perror2("Error in client's pthread_create", err);
            exit(1);
        } 
        if(++line_num == capacity) {
            capacity *= 2;
            t_ids = (int*) realloc(t_ids,capacity * sizeof(int));
        }
    }
    for(int i = 0 ; i < line_num; i++) {
        if(err = pthread_join(t_ids[i],NULL)) {
            perror2("Error in client's pthread_create", err);
            exit(1);
        }         
    }

    free(t_ids);
    return 0;
}


void* ask_server(void* arg) {

    info* con = (info*) arg;
    if(connect(con->sock, (struct sockaddr *) &(con->server),sizeof (con->server)) < 0)
        perror_exit("Can't connect to socket!");
    if (write(con->sock, con->line, strlen(con->line) + 1) < 0)             //SOCKET SHOULD PROBABLY BE THE INE FROM ACCEPT
        perror_exit("write in client");
        
}
