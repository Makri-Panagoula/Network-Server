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

pthread_mutex_t buf_mtx,file_mtx;            
pthread_cond_t empty,full;
//Index of the first "free" place in buffer array 
int cur;                        
ofstream poll_log,poll_stats;
int total_votes,exit_cond;
int* buffer;
//Corresponds name to its "turn" to vote
map<string,int> names;
//Corresponds party name to count of votes
map<string,int> parties;

void exit_server(int signo);

int main (int argc, char* argv[]) {

    //Process command line arguments
    if(argc < 6) {
        printf("Give 6 command line arguments!");
        exit(1);
    }    
    int portNum = atoi(argv[1]);
    int bufferSize = atoi(argv[2]);
    if(bufferSize <= 0) {
        printf("Buffer size should be positive!");
        exit(1);        
    }   
    int numWorkers;   
    if((numWorkers = atoi(argv[3])) <= 0) {
        printf("Number of worker threads should be positive!");
        exit(1);        
    }
    //Buffer with the socket descriptors of the accepted connections
    buffer = (int*) malloc(bufferSize * sizeof(int));       
    cur = 0;                   
    pthread_t* t_ids = (pthread_t*) malloc(numWorkers * sizeof(pthread_t));         //Array with thread ids
    //Create and open files to write
    poll_log.open(argv[4], ios::out | ios::trunc );
    poll_stats.open(argv[5], ios::out | ios::trunc );
    //Initialize mutexes and condition variables
    pthread_mutex_init(&buf_mtx, NULL); 
    pthread_mutex_init(&file_mtx, NULL); 
    pthread_cond_init(&empty, NULL);  
    pthread_cond_init(&full, NULL);      
    int err;
    signal(SIGPIPE, SIG_IGN);
    //Create & Initialize signal handler since we have to exit server for Ctrl-C
    static struct sigaction act ;
    act.sa_handler = exit_server;                      
    sigfillset(&(act.sa_mask));                    
    sigaction(SIGINT , &act , NULL );  
    //Determines whether to terminate threads
    exit_cond = 0;            
    //Create Socket
    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
        perror_exit((char*)"Failed to create socket in server!");
    //Rebind to listening port without TIME-WAIT problems 
    int reuse_addr = 1;   
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse_addr, sizeof(reuse_addr));
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
    //Create worker threads    
    for(int i = 0 ; i < numWorkers; i++) {                                                      //Create threads  
        if((err = pthread_create(t_ids + i, NULL, worker , (void*) NULL))) {
            perror2("Error in server's pthread_create", err);
            exit(1);            
        }
    }              
    while(!exit_cond) {
        // Lock mutex since we are accessing buffer & counter
        if (err = pthread_mutex_lock(&buf_mtx)) {                                                
            perror2("pthread_mutex_lock", err); exit(1); }
        //Wait until buffer isn't full to insert an element
        while(cur == bufferSize)                                                            
            pthread_cond_wait(&full, &buf_mtx);
        //Accept and store in buffer
        int val = accept(sock,(struct sockaddr*) &client, &clientlen);
        if (val == -1  && errno != EINTR)                                           //For when we receive Ctrl-C
            perror_exit((char*)"Error in Accept");
        else
            buffer[cur++] = val;
        //If buffer had been empty it no longer is (in that case there is only one element and therefore we don't use broadcast)
        pthread_cond_signal(&empty); 
        // Unlock mutex => we are exiting critical section                                                       
        if (err = pthread_mutex_unlock(&buf_mtx)) {                                             
            perror2("pthread_mutex_unlock", err); exit(1);  }       
    }
    for(int i = 0 ; i < numWorkers; i++) {
        if(pthread_join(t_ids[i],NULL)) {
            exit(1);
        }             
    } 
    //Write the collected data regarding parties & votes in poll-stat
    for (auto i = parties.begin(); i != parties.end(); i++) 
        poll_stats << " " << i->first << " " << to_string( i->second ) << endl;
       
    poll_stats << "TOTAL : " << total_votes <<endl;    
    //Close file descriptors
    poll_log.close();            
    poll_stats.close();
    close(sock);
    //Free memory
    free(t_ids);                
    free(buffer);
    //Destroy mutex and condition variables
    if (err = pthread_mutex_destroy(&buf_mtx)) {
    perror2("pthread_mutex_destroy", err); exit(1); }
    if (err = pthread_mutex_destroy(&file_mtx)) {
    perror2("pthread_mutex_destroy", err); exit(1); }       
    if (err = pthread_cond_destroy(&empty)) {
    perror2("pthread_cond_destroy on empty ", err); exit(1); }
    if (err = pthread_cond_destroy(&full)) {
    perror2("pthread_cond_destroy on full", err); exit(1); }    
    exit(0);    
}

//Responsible for inter-socket communication, updating poll-log file and related data structures
void* worker(void* arg) {

    //Block SIG-INT signal in all working threads so that it's gonna be executed from main thread
    sigset_t set;                                  
    sigemptyset(&set);
    sigaddset(&set, SIGINT);       
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
        printf("Error in mask\n");   
    int err;
    char full_name[200] , party[100] , exiting[200];

    //Keep on serving requests from client until you must exit and you don't have any left
    while(! exit_cond || cur > 0) {                                                
        // Lock mutex since we are accessing buffer & counter
        if (err = pthread_mutex_lock(&buf_mtx)) {                                                
            perror2("pthread_mutex_lock", err); exit(1); } 
        //Wait until you have at least one socket descriptor
        while( ! cur  && ! exit_cond)   {                                                             
            pthread_cond_wait(&empty, &buf_mtx);
        }
        if( ! cur) {                                                                //There are no remaining requests => thread only woke up to exit        
            if (err = pthread_mutex_unlock(&buf_mtx)) {                             // Unlock mutex => we are exiting critical section
                perror2("pthread_mutex_unlock", err); exit(1);  } 
            pthread_exit(NULL); 
        }
        //Get last socket descriptor (LIFO)
        int sock = buffer[--cur];                                                          //Cur points to first free element, the previous one is the last that got in                                        
        pthread_cond_signal(&full);                                                        //If buffer had been full it no longer is (we can add only 1 more element therefore the signal)                                                                         
        if (err = pthread_mutex_unlock(&buf_mtx)) {                                             // Unlock mutex => we are exiting critical section
            perror2("pthread_mutex_unlock", err); exit(1);  }      

        write(sock,"SEND NAME PLEASE",25);
        read(sock,full_name,200);
        //If the person has already voted exit the thread
        // Lock mutex since we are accessing common data structures and file
        if (err = pthread_mutex_lock(&file_mtx)) {                                                
            perror2("pthread_mutex_lock", err); exit(1); } 

        if(names.find(full_name) != names.end()) {  
            write(sock,"ALREADY VOTED",25);     
            if (err = pthread_mutex_unlock(&file_mtx)) {                                             // Unlock mutex => we are exiting critical section
                perror2("pthread_mutex_unlock", err); exit(1);  }               
            continue;
        }       

        names.insert(make_pair(full_name,total_votes));       
        write(sock,"SEND VOTE PLEASE",25);
        read(sock,party,100);
        if( party[strlen(party) - 1] == '\n' || party[strlen(party) - 1] == ' ')                   //Ignore newline & white space
            party[strlen(party) - 1] = '\0';  

        poll_log <<full_name << party << endl;                                                   //Write in poll_log file  
        total_votes++;
        if(parties.find(party) == parties.end())                                        //If party doesn't exist in map we have to insert it,otherwise update its value
            parties[party] = 1;   
        else 
            parties[party] = parties[party] + 1;
        if (err = pthread_mutex_unlock(&file_mtx)) {                                             // Unlock mutex => we are done with the file
            perror2("pthread_mutex_unlock", err); exit(1);  } 
        //Send message before terminating the connection   
        strcpy(exiting,"VOTE for Party ");
        strcat(exiting,party); 
        strcat(exiting," RECORDED");  
        write(sock,exiting,200);       
    }                      
    pthread_exit(NULL); 
}

void exit_server(int signo) {
    exit_cond = 1;                         //Exit threads
}