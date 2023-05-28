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

void perror_exit(char *message){
    perror(message);
    exit(1);
}

char server_name[200];
int portNum;

int main (int argc, char* argv[]) {

    //Process input arguments
    if(argc < 4) {
        printf("Give 4 command line arguments!");
        exit(1);
    }
    strcpy(server_name,argv[1]);
    portNum = atoi(argv[2]);
    FILE* inputFile = fopen(argv[3],"r");                           //Open file
    if (inputFile == NULL) {
        printf("Error: could not open file %s", argv[3]);
        exit(1);
    } 
    //Ignore Broken pipe   
    signal(SIGPIPE, SIG_IGN);
    int err;
    pthread_t* t_ids = (pthread_t*) malloc(200 * sizeof(pthread_t));
    int line_num = 0;
    int capacity = 200 ;
    char line[200];
    //For every line in the file we create a thread that attempts to connect to server and write the line to
    while (fgets(line, sizeof(line), inputFile) != NULL) {                              //Create a thread for every line of the file
        
        char* thread_line = (char*) malloc((strlen(line) + 1) * sizeof(char));
        strcpy(thread_line,line); 
        if((err = pthread_create(t_ids + line_num, NULL, ask_server, thread_line))) {
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

//Returns the full_name string along with a space in the end
char* get_fullname(char* line) {

    char* full_name = (char*) malloc(200 * sizeof(char));               //same size as line
    strcpy(full_name,line);
    int spaces = 0 ;

    for(int i = 0; i < strlen(line); i++) {                         //Iterate given line until you meet the second space (then you have gotten the full name)

        if(full_name[i] == ' ')
            spaces++;
        if(spaces == 2) {
            full_name[i + 1] = '\0';                                    //Terminate the string keeping only the wanted info (we keep the space too)
            break;
        }
    }
    return full_name;
}

void* ask_server(void* arg) {

    int sock,err;
    char answer[25],exiting[200];
    char* given_line = (char*) arg;   
    //Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)      
        perror_exit((char*)"Failed to create socket in client!");
    //Get hostname as IP address    
    struct hostent *server_ip ;
    if ( (server_ip = gethostbyname(server_name)) == NULL)                          
        perror_exit((char*)"Can't resolve hostname!");
    struct sockaddr_in server ;
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, server_ip->h_addr, server_ip->h_length);    
    server.sin_port = htons(portNum);        
    //Connect to server
    if(connect(sock, (struct sockaddr *) &(server),sizeof (server)) < 0)             //Connect to server
        perror_exit((char*)"Can't connect to socket!");
    //Wait until server is ready to process the writing request(once it has written the corresponding message)     
    read(sock,answer,25);
    //Process line        
    char* full_name = get_fullname(given_line);
    char* party = given_line + strlen(full_name);                    //After the full name we are given the party (full name keeps the space too)
    //Write to server the full name(name & surname can be max 12 characters each + 1 the space between them + null terminator)  
    write(sock,full_name, 200);  
    //Read server's answer                                               
    read(sock,answer,25);
    //Server got and processed the string we gave him release memory    
    free(full_name);
    //Terminate connection and exit thread if the person has already voted                                                                                                         
    if( ! strcmp(answer,"ALREADY VOTED")) {                                          
        close(sock);          
        free(given_line);                                           
        pthread_exit(NULL);         
    }
    write(sock,party, 100);
    //Wait until you receive the terminating message to close the connection                    
    read(sock,exiting,200);   
    //Close socket & exit                                                     
    close(sock);   
    free(given_line);                                                   
    pthread_exit(NULL); 
}
