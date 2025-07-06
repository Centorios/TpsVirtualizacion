*
Integrantes del grupo:
- Berti Rodrigo
- Burnowicz Alejo
- Fernandes Leonel
- Federico Agustin
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include "cliente_handler.h"
#include "partida.h"
#define FALSE 0
#define TRUE 1
#define MAX_FRASES 3
#define MAX_LINEA 30

bool termProcess = TRUE;

void sigusrHandler(){
	termProcess = FALSE;
}


int main(int argc, char *argv[]){
signal(SIGINT,sigusrHandler);
signal(SIGUSR1,sigusrHandler);

/////////////////////////////////////////////////////////////////////
if(argc > 7) {
        printf("parametros invalidos\n");
        return 1;
}

char* filePath = NULL;

int i = 0;
bool cantidad = FALSE;
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

		if(strcmp(argv[i],"-usuarios") == 0 || strcmp(argv[i],"-u") ==0){
			i++;
			cantidad = TRUE;
		}

		if(strcmp(argv[i],"-archivo") == 0 || strcmp(argv[i],"-a") == 0) {
			filePath = argv[i+1];
			i++;
			b_file = TRUE;
		}
	}




i++;
}

if(cantidad == FALSE){
	printf("parametro usuarios invalido o faltante\n");
	return 1;
}
if(b_file == FALSE){
	printf("parametro archivo invalido o faltante\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////
FILE *file = fopen(filePath, "r");
if (!file){
	perror("Error abriendo archivo");
	return 1;
}
char *frases[MAX_FRASES];
char bufferArchivo[MAX_LINEA];
int cantidadFrases = 0;

while (fgets(bufferArchivo, MAX_LINEA, file) && cantidadFrases < MAX_FRASES){
	// Eliminar el salto de línea final si existe
	bufferArchivo[strcspn(bufferArchivo, "\n")] = '\0';
	// Reservar memoria y copiar la línea
	frases[cantidadFrases] = strdup(bufferArchivo);
	if (!frases[cantidadFrases]){
		perror("Fallo al reservar memoria");
		fclose(file);
		return 1;
	}
	cantidadFrases++;
}

fclose(file);
////////////////////////////////////////////////////////////////////////////