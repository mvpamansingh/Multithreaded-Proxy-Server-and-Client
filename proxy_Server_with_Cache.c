//
// Created by ASUS on 12-07-2024.
//

#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include<time.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>

#include<netdb.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<errno.h>



typedef struct cache_element cache_element;
#define MAX_CLIENT 10

struct cache_element
{
    char* data;
    int len;
    char* url;
    time_t lru_time_track;
    cache_element *next;
};


// Method to find the cache with the associated URL  in cache linklist
cache_element* find(char* url);

//Method to add the receiveing data from WWW into cache linklist
int add_cache_element(char* data, int size, char* url);

// Method to remove element from cache linklist
void remove_cache_element();

// Port number on which our Proxy server is running
int port_number= 8080;



int proxy_socketId;

pthread_t tid[MAX_CLIENT];

sem_t semaphore;

//lock for handing shared resource - here LRU cache is shared resources and when multiple thread   try to write it in RACE CONDITION occurs
//lock has only 2 value  -   0 means resource not accuired, 1 mean resource is currently accuired
//                           Story :)
// at first a thread will come to write data from WWW to LRU cache, it check wether the cache is 
// ocupied or not with help of lock, if not ocuppied it will accire the resource and set LOCK to 1 and
// when work is over it setback LOCK to 0

// is us to handle shared resources we uses mutex
pthread_mutex_t lock;

cache_element* head;
int cache_size;






int main(int argc, char * argv[])
{
    // This is use to store socket id and lenth of address of client
    int client_socketId, client_len;

    struct sockaddr_in server_addr, client_addr;

    //Intializing semaphore with address , min and max vaalue
    sem_init(&semaphore,0, MAX_CLIENT);
    pthread_mutex_init(&lock, NULL);

    if(argv ==2)
    {
        port_number = atoi(argv[1]);
    }
    else
    {
        printf("Too less argument provided");
        exit(1);
    }

    




    printf("Starting Proxy Server at port: %d\n",port_number);

    //This is the main and only Proxy server socket ,we need to creates socket id of proxy server that listens
    //incomming request --> if req accepts   --> then a new thread is spawn    an socket is open
    proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);

	if( proxy_socketId < 0)
	{
		perror("Failed to create socket.\n");
		exit(1);
	}

    // This is only socket that listen for connection request so, we have to ad the functio nality to use it again , for different client


    int reuse =1;
	if (setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEADDR) failed\n");

    //set the value to zero , bydefault in c garbage value is stored
    bzero((char*)&server_addr, sizeof(server_addr));  

    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number); // Assigning port to the Proxy
	server_addr.sin_addr.s_addr = INADDR_ANY; // Any available adress assigned



    //Binding the socket
	if( bind(proxy_socketId, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 )
	{
		perror("Port is not free\n");
		exit(1);
	}
	printf("Binding on port: %d\n",port_number);



    //// Proxy socket listening to the requests - this code setup proxy server to listen
	int listen_status = listen(proxy_socketId, MAX_CLIENTS);

	if(listen_status < 0 )
	{
		perror("Error while Listening !\n");
		exit(1);
	}
}