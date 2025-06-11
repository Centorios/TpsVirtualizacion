#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "cliente_handler.h"
#include "partida.h"
#define BUFFER_SIZE 2048
#define MAX_LINEA 30
#define FALSE 0
#define TRUE 1

typedef struct {
	char nickName[BUFFER_SIZE];
	int* puntuacion;
	int* cantFrases;
	char** frases;
	int* cliente_id;

} ParametrosThreadGame;


void* handle_client(void* arg) {
    ParametrosThreadGame params = *(ParametrosThreadGame*)arg;
    int client_fd = *params.cliente_id;
    free(params.cliente_id); // Release memory from malloc in main
    memset(params.nickName,0,BUFFER_SIZE);
    char recv_buffer[BUFFER_SIZE];
    char resp_buffer[BUFFER_SIZE];
    char palabraAEvaluar[MAX_LINEA];
    memset(recv_buffer,0,BUFFER_SIZE);
    memset(resp_buffer,0,BUFFER_SIZE);
    ssize_t n = 0;
    printf("Handling client in thread (fd: %d)\n", client_fd);

    //aca va el mensaje de inicio de partida
    snprintf(resp_buffer,BUFFER_SIZE,"INICIO_PARTIDA");
    send(client_fd,resp_buffer,strlen(resp_buffer),0);

    n = recv(client_fd,recv_buffer,BUFFER_SIZE,0);
    if(n<=0){
    	return NULL;
    }
    strcpy(params.nickName,recv_buffer);
    memset(recv_buffer,0,BUFFER_SIZE);

	while(1){
		int indice = rand() % *(params.cantFrases);
		strcpy(palabraAEvaluar,params.frases[indice]);
		int resPartida = 0;
		resPartida = partida(palabraAEvaluar,recv_buffer,resp_buffer,client_fd);

		if(resPartida == FALSE){
			printf("%s perdio el juego\n",params.nickName);
			snprintf(resp_buffer,BUFFER_SIZE,"PERDISTE");
    			send(client_fd,resp_buffer,strlen(resp_buffer),0);
			break;
		} else {
			printf("%s ganó el juego\n",params.nickName);
                        snprintf(resp_buffer,BUFFER_SIZE,"GANASTE");
                        send(client_fd,resp_buffer,strlen(resp_buffer),0);
			break;
		}
/*
		n =recv(client_fd,recv_buffer,BUFFER_SIZE,0);
    		if(n<=0){
			break;
		}
	snprintf(resp_buffer,BUFFER_SIZE, "ack");
    		send(client_fd, resp_buffer, strlen(resp_buffer), 0);
		memset(recv_buffer,0,BUFFER_SIZE);
*/
	}

    close(client_fd);
    printf("Client connection closed (fd: %d)\n", client_fd);
    return NULL;
}
