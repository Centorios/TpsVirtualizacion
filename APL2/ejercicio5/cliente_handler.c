#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "cliente_handler.h"

#define BUFFER_SIZE 1024

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg); // Release memory from malloc in main
    char buffer[BUFFER_SIZE];

    printf("Handling client in thread (fd: %d)\n", client_fd);

    memset(buffer, 0, BUFFER_SIZE);
    recv(client_fd, buffer, BUFFER_SIZE, 0);
    printf("Client says: %s\n", buffer);

    const char* response = "Hello from server thread!";
    send(client_fd, response, strlen(response), 0);

    close(client_fd);
    printf("Client connection closed (fd: %d)\n", client_fd);
    return NULL;
}
