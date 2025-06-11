#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 2048
#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[]){

////////////////////////////////////////////////////////////////////////////
if(argc > 7) {
        printf("parametros invalidos\n");
        return 1;
 }

char* serverIp;
char* nickName;
int portNumber;
struct sockaddr_in socketAddr;

int i = 0;
bool port = FALSE;
bool b_nickname = FALSE;
bool server = FALSE;
while(i <= argc)
{
	if(argv[i] != NULL) {
	        if(strcmp(argv[i],"-help") == 0 || strcmp(argv[i],"-h") == 0){
			printf("-puerto o -p para indicar el peurto del servidor de destino\n");
			printf("-servidor o -s para indicar la ipv4 del servidor de destino\n");
			printf("-nickname o -n para indicar el nickname del cliente\n");
			return 0;
	        }

		if(strcmp(argv[i],"-puerto") == 0 || strcmp(argv[i],"-p") == 0){
			portNumber = atoi(argv[i+1]);
			i++;
			port = TRUE;
		}

		if(strcmp(argv[i],"-servidor") == 0 || strcmp(argv[i],"-s") ==0){
			serverIp = argv[i+1];
			int res = inet_pton(AF_INET, serverIp,&(socketAddr.sin_addr));
			if( res <= 0 ){
				printf("direccion ipv4 invalida: %s\n",serverIp);
				return 1;
			}
			i++;
			server = TRUE;
		}

		if(strcmp(argv[i],"-nickname") == 0 || strcmp(argv[i],"-n") == 0) {
			nickName = argv[i+1];
			i++;
			b_nickname = TRUE;
		}
	}




i++;
}

if(b_nickname == FALSE){
	printf("parametro nickname invalido o faltante\n");
	return 1;
}
if(server == FALSE){
	printf("parametro server invalido o faltante\n");
	return 1;
}
if(port == FALSE){
	printf("parametro puerto invalido o faltante\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////

int sockfd;
struct sockaddr_in server_addr;
char send_buffer[BUFFER_SIZE];
char recv_buffer[BUFFER_SIZE];

sockfd = socket(AF_INET,SOCK_STREAM,0);

if(sockfd < 0){
	perror("fallo la creacion del socket");
	exit(EXIT_FAILURE);
}

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(portNumber);
server_addr.sin_addr.s_addr = INADDR_ANY;
if(inet_pton(AF_INET,serverIp,&server_addr.sin_addr) <= 0){
	perror("dirección de server invalida");
	close(sockfd);
	exit(EXIT_FAILURE);
}

if(connect(sockfd, (struct sockaddr*)&server_addr,sizeof(server_addr)) < 0){
	perror("fallo al establecer la conexion");
	close(sockfd);
	exit(EXIT_FAILURE);
}

printf("Conectado al servidor en %s:%d\n",serverIp, portNumber);
ssize_t n = 0;

n = recv(sockfd,recv_buffer,BUFFER_SIZE,0);

if(n<=0){
	perror("fallo algo");
}

if(strcmp(recv_buffer,"INICIO_PARTIDA") != 0){
	perror("no llego el inicio partida");
}

snprintf(send_buffer,BUFFER_SIZE,"ACK");
send(sockfd,send_buffer,strlen(send_buffer),0);
memset(send_buffer,0,BUFFER_SIZE);

snprintf(send_buffer,BUFFER_SIZE,"%s", nickName);
send(sockfd,send_buffer,strlen(send_buffer),0);
memset(send_buffer,0,BUFFER_SIZE);

memset(recv_buffer,0,BUFFER_SIZE);
n = recv(sockfd,recv_buffer,BUFFER_SIZE,0);

if(n<=0){
	perror("fallo la primera entrada de la palabra con ___");
	exit(1);	
}

printf("[server]: %s\n",recv_buffer);

memset(recv_buffer,0,BUFFER_SIZE);

while(1){
	printf("> ");
	fflush(stdout);
	if (fgets(send_buffer,BUFFER_SIZE,stdin) == NULL) {
		//aca va la logica de error en el buffer
		break;
	}

	//remover newline del mensaje
	send_buffer[strcspn(send_buffer, "\n")] = '\0';

	if(strcmp(send_buffer,"exit") == 0 || strcmp(send_buffer,"EXIT") == 0){
		printf("desconectando...\n");
		//aca va la logica de desconexion de cliente controlada
		break;
	}

	if (send(sockfd,send_buffer,strlen(send_buffer),0) < 0 ){
		//aca va la logiva de recupero de mensaje o notificacion de error
		perror("fallo el envio del mensaje al server");
		break;
	}

	memset(recv_buffer,0,BUFFER_SIZE);
	n = recv(sockfd,recv_buffer,BUFFER_SIZE -1,0);

	if(n <= 0){
		printf("server desconextado o ocurrio un error\n");
		break;
	}


	printf("[server]: %s\n",recv_buffer);
}


close(sockfd);
return 0;


}
