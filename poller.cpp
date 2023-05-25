#include <iostream>
#include <set>
#include <string> 
#include <stdlib.h> 
#include <pthread.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h> 
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <bits/stdc++.h>

#define perror2(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))
using namespace std;

void perror_exit(char *message){
    perror(message);
    exit(EXIT_FAILURE);
}

void* worker(void* arg);

void get_stats(int signo);

pthread_mutex_t mtx;            
pthread_cond_t empty,full; 
int cur;                        //First "free" place in buffer array
ofstream poll_log, poll_stats;
int total_votes,loop;
int* buffer;
set<char*> names;
map<char*,int> parties;

int main (int argc, char* argv[]) {

    //Process command line arguments
    if(argc < 6) {
        printf("Give 6 command line arguments!");
        exit(1);
    }    
    int portNum = atoi(argv[1]);
    int numWorkers = atoi(argv[2]);
    if(numWorkers <= 0) {
        printf("Number of worker threads should be positive!");
        exit(1);        
    }
    int bufferSize = atoi(argv[3]);
    if(bufferSize <= 0) {
        printf("Buffer size should be positive!");
        exit(1);        
    }    
    buffer = (int*) malloc(bufferSize * sizeof(int));                          //Buffer with the socket descriptors of the accepted connections
    pthread_t* t_ids = (pthread_t*) malloc(numWorkers * sizeof(pthread_t));         //Array with thread ids

    ofstream poll_log(argv[3]);
    ofstream poll_stats(argv[4]);
    int err;
    //Create & Initialize sigaction structure, we work with it since it is more reliable
    static struct sigaction act ;
    act.sa_handler = get_stats;                      
    sigfillset(&(act.sa_mask));                     // ignore every signal when you are in signal handler
    sigaction (SIGINT , &act , NULL );  
           
    cur = 0;
    //Create Socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        perror_exit((char*)"Failed to create socket in server!");

    struct sockaddr_in server,client;    
    server.sin_family = AF_INET;                        /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portNum);                   /* The given port */
    unsigned int clientlen = sizeof(client);
        
    //Bind
    if(bind(sock, (struct sockaddr *) &server,sizeof (server)) == -1)
        perror_exit((char*)"Error in binding !");
    //Listen
    if(listen(sock,128) < 0)     
        perror_exit((char*)"Error in listening!");

    for(int i = 0 ; i < numWorkers; i++) {                                                      //Create threads
        if((err = pthread_create(t_ids + i, NULL, worker , (void*) worker_arg))) {
            perror2("Error in server's pthread_create", err);
            exit(1);            
        }
    }
    //Initialize mutex and condition variables
    pthread_mutex_init(&mtx, NULL); 
    pthread_cond_init(&empty, NULL);  
    pthread_cond_init(&full, NULL);  
    loop = 1;
    while(loop) {

        if (err = pthread_mutex_lock(&mtx)) {                                                // Lock mutex since we are accessing buffer & counter
            perror2("pthread_mutex_lock", err); exit(1); }

        while(cur == bufferSize)                                                            //Wait until buffer isn't full to insert an element
            pthread_cond_wait(&full, &mtx);

        if ((buffer[cur++] = accept(sock,(struct sockaddr*) &client, &clientlen)) < 0)      //Accept and store in buffer
            perror_exit((char*)"Error in Accept");

        pthread_cond_signal(&empty);                                                        //If buffer had been empty it no longer is (in that case there is only one element and therefore we don't use broadcast)
        if (err = pthread_mutex_unlock(&mtx)) {                                             // Unlock mutex => we are exiting critical section
            perror2("pthread_mutex_unlock", err); exit(1);  }       
    }

    for(int i = 0 ; i < numWorkers; i++) {
        if((err = pthread_join(t_ids[i],NULL))) {
            perror2("Error in server's pthread_join", err);
            exit(1);
        }         
    }


    poll_log.close();            //Close file descriptors
    poll_stats.close();
    close(sock);
    free(t_ids);                //Free memory
    free(buffer);
    //Destroy mutex and condition variables
    if (err = pthread_mutex_destroy(&mtx)) {
    perror2("pthread_mutex_destroy", err); exit(1); }
    if (err = pthread_cond_destroy(&empty)) {
    perror2("pthread_cond_destroy on empty ", err); exit(1); }
    if (err = pthread_cond_destroy(&full)) {
    perror2("pthread_cond_destroy on full", err); exit(1); }    
    return 0;
}

//Responsible for inter-socket communication, updating poll-log file and related data structures
void* worker(void* arg) {

    int err;
    char full_name[26]; 
    char party[100];

    if (err = pthread_mutex_lock(&mtx)) {                                                // Lock mutex since we are accessing buffer & counter
        perror2("pthread_mutex_lock", err); exit(1); } 

    while(cur == 0)                                                                     //Wait until you have at least one socket descriptor
        pthread_cond_wait(&empty, &mtx);
    
    int sock = buffer[cur--];                                                           //Get last socket descriptor (LIFO)
    pthread_cond_signal(&full);                                                        //If buffer had been full it no longer is (we can add only 1 more element therefore the signal)                                                                         
    if (err = pthread_mutex_unlock(&mtx)) {                                             // Unlock mutex => we are exiting critical section
        perror2("pthread_mutex_unlock", err); exit(1);  }     

    write(sock,"SEND NAME PLEASE",25);
    read(sock,full_name,26);
    auto pos = names.find(full_name);

    if(pos == names.end()) {                                                       //If the person has already voted exit the thread
        write(sock,"ALREADY VOTED",25);
        pthread_exit(NULL); 
    }
    names.insert(full_name);
    //Write in poll_log file
    write(sock,"SEND VOTE PLEASE",25);
    read(sock,party,100);                                                             //Read given string(name & surname can be max 12 characters each + 1 the space between them + null terminator)    
    poll_log << full_name;
    poll_log << " " ;
    poll_log << party ;
    poll_log << "\n";
    //Write in files if the name doesn't already exist and update data structures + maybe the need for extra synch(is current mutex too much?s)
    total_votes++;
    if(parties.find(party) == parties.end())                                        //If party doesn't exist in map we have to insert it
        parties[party] = 1;   
    else 
        parties[party] = parties[party] + 1;

    //Send message before terminating the connection                                    
    poll_log << "VOTE for Party ";          
    poll_log << party;
    poll_log << " RECORDED \n";
    pthread_exit(NULL); 
}


void get_stats(int signo) {

    //Write the collected data regarding parties & votes in poll-stat
    for (auto i = parties.begin(); i != parties.end(); i++) {

        poll_stats << i->first;
        poll_stats << " ";
        poll_stats << to_string( i->second );
        poll_stats << "\n";
    }

    poll_stats << "TOTAL ";
    poll_stats << to_string(total_votes);
    loop = 0;
}
