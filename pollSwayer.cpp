#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <sys/wait.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

void* ask_server(void* arg);

typedef struct  {                                   //Struct we pass to thread function so as to connect to server
    char line[200];                                //line we will be writing
    struct sockaddr_in server;                    //Server to connect
} info;

void perror_exit(char *message){
    perror(message);
    exit(1);
}

int main (int argc, char* argv[]) {

    //Process input arguments
    if(argc < 4) {
        printf("Give 4 command line arguments!");
        exit(1);
    }
    int portNum = atoi(argv[2]);
    info connection;
    FILE* inputFile = fopen(argv[3],"r");                           //Open file
    if (inputFile == NULL) {
        printf("Error: could not open file %s", argv[3]);
        exit(1);
    }    

    struct hostent *server_ip ;
    if ( (server_ip = gethostbyname(argv[1])) == NULL)                      //Get hostname as IP address    
        perror_exit((char*)"Can't resolve hostname!");

    struct sockaddr_in server ;
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, server_ip->h_addr, server_ip->h_length);    
    server.sin_port = htons(portNum);
    connection.server = server ;

    //For every line in the file we create a thread that attempts to connect to server and write the line to
    char line[200];
    int err;
    pthread_t* t_ids = (pthread_t*) malloc(200 * sizeof(pthread_t));
    int line_num = 0;
    int capacity = 200 ;
    while (fgets(line, sizeof(line), inputFile) != NULL) {                              //Create a thread for every line of the file

        strcpy(connection.line,line);      

        if((err = pthread_create(t_ids + line_num, NULL, ask_server, (void*) &connection))) {
            perror2("Error in client's pthread_create", err);
            exit(1);
        } 
        if(++line_num == capacity) {                                                    //Readjust thread's array size if needed
            capacity *= 2;
            t_ids = (pthread_t*) realloc(t_ids,capacity * sizeof(pthread_t));
        }
    }
    for(int i = 0 ; i < line_num; i++) {
        if((err = pthread_join(t_ids[i],NULL))) {
            perror2("Error in client's pthread_join", err);
            exit(1);
        }         
    }

    free(t_ids);
    return 0;
}

char* get_fullname(char* line) {

    char* full_name = (char*) malloc( 26 * sizeof(char));               //name & surname can be max 12 characters each + 1 the space between them + null terminator
    int spaces = 0 ;          

    for(int i = 0; i < strlen(line); i++) {                         //Iterate given line until you meet the second space (then you have gotten the full name)

        if(line[i] == ' ')
            spaces++;
        if(spaces == 2) {
            line[i + 1] = '\0';                                      //Terminate the string keeping only the wanted info (we keep the space too)
            break;
        }
    }
    strcpy(full_name,line);
    return full_name;
}

void* ask_server(void* arg) {

    int sock;
    char answer[25];
    info* con = (info*) arg;
    char* full_name = get_fullname(con->line);
    char* party = con->line + strlen(full_name) + 1;                    //After the full name we are given the party (full name keeps the space too)

    //Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)      
        perror_exit((char*)"Failed to create socket in client!");
    //Connect to server
    if(connect(sock, (struct sockaddr *) &(con->server),sizeof (con->server)) < 0)             //Connect to server
        perror_exit((char*)"Can't connect to socket!");
    //Wait until server is ready to process the writing request(once it has written the corresponding message)     
    read(sock,answer,25);
    //Write to server the full name(name & surname can be max 12 characters each + 1 the space between them + null terminator)  
    write(sock,full_name, 26);  
    //Read server's answer                                               
    read(sock,answer,25);
    //Server got and processed the string we gave him release memory    
    free(full_name);
    //Terminate connection and exit thread if the person has already voted                                                                                                         
    if( ! strcmp(answer,"ALREADY VOTED")) {                                          
        close(sock);                                                     
        pthread_exit(NULL);         
    }
    write(sock,party, 100);
    //Wait until you receive the terminating message to close the connection                    
    // read(sock,)   
    //Close socket & exit                                                     
    close(sock);                                                      
    pthread_exit(NULL); 
}
