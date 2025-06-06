#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
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
			//aca va la logica del help
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



}
