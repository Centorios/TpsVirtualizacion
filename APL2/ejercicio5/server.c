#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "cliente_handler.h"
#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[]){

////////////////////////////////////////////////////////////////////////////
if(argc > 7) {
        printf("parametros invalidos\n");
        return 1;
}


int serverPort = -1;
int maxUsers = -1;
char* filePath = NULL;

int i = 0;
bool port = FALSE;
bool users = FALSE;
bool b_file = FALSE;
while(i < argc)
{
	if(argv[i] != NULL) {
	        if(strcmp(argv[i],"-help") == 0 || strcmp(argv[i],"-h") == 0){
			printf("-puerto o -p seguido del numero de puerto\n");
			printf("-usuarios o -u seguido del numero maximo de usuarios que maneja el server\n");
			printf("-archivo o -a seguido del path donde se encuentra el archivo con las palabras\n");
			return 0;
	        }

		if(strcmp(argv[i],"-puerto") == 0 || strcmp(argv[i],"-p") == 0){
			serverPort = atoi(argv[i+1]);
			i++;
			port = TRUE;
			
		}

		if(strcmp(argv[i],"-usuarios") == 0 || strcmp(argv[i],"-u") ==0){
			maxUsers = atoi(argv[i+1]);
			i++;
			users = TRUE;
		}

		if(strcmp(argv[i],"-archivo") == 0 || strcmp(argv[i],"-a") == 0) {
			filePath = argv[i+1];
			i++;
			b_file = TRUE;
		}
	}




i++;
}

if(users == FALSE){
	printf("parametro usuarios invalido o faltante\n");
	return 1;
}
if(b_file == FALSE){
	printf("parametro archivo invalido o faltante\n");
	return 1;
}
if(port == FALSE){
	printf("parametro puerto invalido o faltante\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////

FILE *file = fopen(filePath,"r");

if (!file){
	perror("Error abriendo archivo");
	return 1;
}

////////////////////////////////////////////////////////////////////////////
int server_fd;
struct sockaddr_in server_addr, client_addr;
socklen_t addr_len = sizeof(client_addr);

server_fd = socket(AF_INET,SOCK_STREAM,0);

if (server_fd == -1 ) {
	perror("error creando el socket");
	exit(EXIT_FAILURE);
}

server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(serverPort);

if (bind(server_fd,(struct sockaddr*)&server_addr, sizeof(server_addr)) <0){
	perror("fallo al bindear el socket");
	close(server_fd);
	exit(EXIT_FAILURE);
}

if (listen(server_fd, maxUsers) < 0) {
	perror("Listen failed");
	close(server_fd);
	exit(EXIT_FAILURE);
}

printf("servidor a la escucha en el puerto: %d\n",serverPort);



while (1){
	int* client_fd = malloc(sizeof(int));

	if (!client_fd) {
		perror("fallo el malloc para alojar el cliente");
		continue;
	}

	*client_fd = accept(server_fd, (struct sockaddr *)&client_addr,&addr_len);

	if (*client_fd < 0){
		perror("fallo aceptar el cliente");
		free(client_fd);
		continue;
	}

	printf("Conexion aceptada de %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

	pthread_t tid;

	if(pthread_create(&tid,NULL,handle_client,client_fd) != 0){
		perror("fallo la creacion del thread");
		close(*client_fd);
		free(client_fd);
	} else {
		pthread_detach(tid);
	}



}


close(server_fd);
return 0;
}
