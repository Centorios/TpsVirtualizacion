#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024
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
			//aca va la logica del help
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
char buffer[BUFFER_SIZE];

sockfd = socket(AF_INET,SOCK_STREAM,0);

if(sockfd < 0){
	perror("fallo la creacion del socket");
	exit(EXIT_FAILURE);
}

server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(portNumber);
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









}
