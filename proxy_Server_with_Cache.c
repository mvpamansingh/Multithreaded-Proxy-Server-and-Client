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
#define MAX_BYTES 4096
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

    // this store list of currently connected clients
    
    int i = 0; // Iterator for thread_id (tid) and Accepted Client_Socket for each thread
	int Connected_socketId[MAX_CLIENTS];   // This array stores socket descriptors of connected clients

       // Infinite Loop for accepting connections
	while(1)
	{
		
		bzero((char*)&client_addr, sizeof(client_addr));			// Clears struct client_addr
		client_len = sizeof(client_addr); 

        // Accepting the connections
        // Below code try to acceept a new connection to proxy_socketId and once athe connection is established it create unique 
        // it return socket descriptor which can be used to share data from server to cleint on another socket not prxysocketID
		client_socketId = accept(proxy_socketId, (struct sockaddr*)&client_addr,(socklen_t*)&client_len);	// Accepts connection
		if(client_socketId < 0)
		{
			fprintf(stderr, "Error in Accepting connection !\n");
			exit(1);
		}
		else{
			Connected_socketId[i] = client_socketId; // Storing accepted client into array
		}

		// Getting IP address and port number of client
		struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
		struct in_addr ip_addr = client_pt->sin_addr;
		char str[INET_ADDRSTRLEN];										// INET_ADDRSTRLEN: Default ip address size
		inet_ntop( AF_INET, &ip_addr, str, INET_ADDRSTRLEN );
		printf("Client is connected with port number: %d and ip address: %s \n",ntohs(client_addr.sin_port), str);
		//printf("Socket values of index %d in main function is %d\n",i, client_socketId);
		pthread_create(&tid[i],NULL,thread_fn, (void*)&Connected_socketId[i]); // Creating a thread for each client accepted
		i++; 
	}
	close(proxy_socketId);									// Close socket
 	return 0;
}


void* thread_fn(void* socketNew)
{
	// subtracts 1 from semaphore value and if it is negative it waits and if not it do further work
	sem_wait(&seamaphore); 
	int p;
	// getting semaphore value in p to check 
	sem_getvalue(&seamaphore,&p);
	printf("semaphore value:%d\n",p);
    int* t= (int*)(socketNew);
	int socket=*t;           // Socket is socket descriptor of the connected Client
	int bytes_send_client,len;	  // Bytes Transferred by cleint http req to server

	
	char *buffer = (char*)calloc(MAX_BYTES,sizeof(char));	// Creating buffer of 4kb for a client
	
	
	bzero(buffer, MAX_BYTES);								// Making buffer zero

	bytes_send_client = recv(socket, buffer, MAX_BYTES, 0); // Receiving the Request of client by proxy server and storing it in buffer and it also return no of bites transffered
	
	while(bytes_send_client > 0)
	{
		len = strlen(buffer);
        //loop until u find "\r\n\r\n" in the buffer
		if(strstr(buffer, "\r\n\r\n") == NULL)
		{	
			bytes_send_client = recv(socket, buffer + len, MAX_BYTES - len, 0);
		}
		else{
			break;
		}
	}

	// creating another buffer to store complete cleint request (http link)
	char *tempReq = (char*)malloc(strlen(buffer)*sizeof(char)+10);

    //tempReq, buffer both store the http request sent by client

	for (int i = 0; i < strlen(buffer); i++)

	{

		tempReq[i] = buffer[i];

	}

	

	//checking for the request in cache i.e   
	//passing Url stored in temp req to find method to check wether the url exist in LRU cache 

	struct cache_element* temp = find(tempReq);



	if( temp != NULL){

        //request found in cache, so sending the response to client from proxy's cache

		int size=temp->len/sizeof(char);

		int pos=0;

		char response[MAX_BYTES];

		while(pos<size){

			bzero(response,MAX_BYTES);

			// Url exist so sending data corresponding to   that url stored in cache , bit by bit
			for(int i=0;i<MAX_BYTES;i++){

				response[i]=temp->data[pos];

				pos++;

			}
			// sending the request to client on respective socket
			send(socket,response,MAX_BYTES,0);

		}

		printf("Data retrived from the Cache :  \n\n");

		printf("%s\n\n",response);

		//return NULL;

	}

	

	

	else if(bytes_send_client > 0)

	{// this executes when url is not found in cache, we do a check if URL recueved from client >0
	// then parse the http call to the WWW (google.co)
		len = strlen(buffer); 

		//Parsing the request

		struct ParsedRequest* request = ParsedRequest_create();

		

        //ParsedRequest_parse returns 0 on success and -1 on failure.On success it stores parsed request in

        // the request
		
		// passing the URL in buffer to 
		if (ParsedRequest_parse(request, buffer, len) < 0) 

		{

		   	printf("Parsing failed\n");

		}

		else

		{	

			bzero(buffer, MAX_BYTES);

			if(!strcmp(request->method,"GET"))							

			{

                

				if( request->host && request->path && (checkHTTPversion(request->version) == 1) )

				{

					bytes_send_client = handle_request(socket, request, buffer, tempReq);		// Handle GET request

					if(bytes_send_client == -1)

					{	

						sendErrorMessage(socket, 500);

					}



				}

				else

					sendErrorMessage(socket, 500);			// 500 Internal Error



			}

            else

            {

                printf("This code doesn't support any method other than GET\n");

            }

    

		}

        //freeing up the request pointer

		ParsedRequest_destroy(request);



	}

	else if( bytes_send_client < 0)

	{

		perror("Error in receiving from client.\n");

	}

	else if(bytes_send_client == 0)

	{

		printf("Client disconnected!\n");

	}



	shutdown(socket, SHUT_RDWR);

	close(socket);

	free(buffer);

	sem_post(&seamaphore);	

	sem_getvalue(&seamaphore,&p);

	printf("Semaphore post value:%d\n",p);

	return NULL;



}




// ** buff error



int handle_request(int clientSocket, struct ParsedRequest *request, char *tempReq)

{
	
	
	char *buf = (char*)malloc(sizeof(char)*MAX_BYTES);
	strcpy(buf, "GET ");

	strcat(buf, request->path);

	strcat(buf, " ");

	strcat(buf, request->version);

	strcat(buf, "\r\n");



	size_t len = strlen(buf);



	if (ParsedHeader_set(request, "Connection", "close") < 0){

		printf("set header key not work\n");

	}



	if(ParsedHeader_get(request, "Host") == NULL)

	{

		if(ParsedHeader_set(request, "Host", request->host) < 0){

			printf("Set \"Host\" header key not working\n");

		}

	}



	if (ParsedRequest_unparse_headers(request, buf + len, (size_t)MAX_BYTES - len) < 0) {

		printf("unparse failed\n");

		//return -1;				// If this happens Still try to send request without header

	}



	int server_port = 80;				// Default Remote end Server Port WWW   most of the ime it exist on 80

	if(request->port != NULL)

		server_port = atoi(request->port);



	int remoteSocketID = connectRemoteServer(request->host, server_port);



	if(remoteSocketID < 0)

		return -1;



	int bytes_send = send(remoteSocketID, buf, strlen(buf), 0);



	bzero(buf, MAX_BYTES);



	bytes_send = recv(remoteSocketID, buf, MAX_BYTES-1, 0);
	char *temp_buffer = (char*)malloc(sizeof(char)*MAX_BYTES); //temp buffer
	int temp_buffer_size = MAX_BYTES;
	int temp_buffer_index = 0;

	while(bytes_send > 0)
	{
		bytes_send = send(clientSocket, buf, bytes_send, 0);
		
		for(int i=0;i<bytes_send/sizeof(char);i++){
			temp_buffer[temp_buffer_index] = buf[i];
			// printf("%c",buf[i]); // Response Printing
			temp_buffer_index++;
		}
		temp_buffer_size += MAX_BYTES;
		temp_buffer=(char*)realloc(temp_buffer,temp_buffer_size);

		if(bytes_send < 0)
		{
			perror("Error in sending data to client socket.\n");
			break;
		}
		bzero(buf, MAX_BYTES);

		bytes_send = recv(remoteSocketID, buf, MAX_BYTES-1, 0);

	} 
	temp_buffer[temp_buffer_index]='\0';
	free(buf);
	add_cache_element(temp_buffer, strlen(temp_buffer), tempReq);
	printf("Done\n");
	free(temp_buffer);
	
	
 	close(remoteSocketID);
	return 0;
}