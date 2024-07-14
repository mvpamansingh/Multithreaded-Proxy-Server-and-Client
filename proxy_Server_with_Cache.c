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