#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "cliente_handler.h"

#define BUFFER_SIZE 2048

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg); // Release memory from malloc in main
    char buffer[BUFFER_SIZE];

    printf("Handling client in thread (fd: %d)\n", client_fd);

	while(1){
		memset(buffer, 0, BUFFER_SIZE);
		ssize_t n =recv(client_fd,buffer,BUFFER_SIZE-1,0);
    		if(n<=0){
			break;
		}
		printf("%s\n",buffer);
    		char response[BUFFER_SIZE];
		snprintf(response,BUFFER_SIZE, "ack");
    		send(client_fd, response, strlen(response), 0);
	}
    close(client_fd);
    printf("Client connection closed (fd: %d)\n", client_fd);
    return NULL;
}
