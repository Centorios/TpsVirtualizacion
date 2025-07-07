/*
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
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

#define FALSE 0
#define TRUE 1
#define MAX_FRASES 3
#define MAX_LINEA 30


typedef struct {
	char nickname[30];
	char ultimaLetra;
	char palabra[30];
	char palabraCamuflada[30];
	int intentos;
	char estadoPartida[30];

} SharedMemory;


bool termProcess = TRUE;

void sigusrHandler(){
	//preparar el manejador para displayear el raking en pantalla
	return 0;
}

void devolverPalabraJuego(char *destino, char *original) {
    int i;
    for (i = 0; original[i] != '\0'; i++) {
        destino[i] = (original[i] == ' ') ? ' ' : '_';
    }
    destino[i] = '\0';
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
int cantidad = 0;
bool b_cantidad = FALSE;
bool b_file = FALSE;
while(i < argc)
{
	if(argv[i] != NULL) {
	        if(strcmp(argv[i],"-help") == 0 || strcmp(argv[i],"-h") == 0){
			//printf("-puerto o -p seguido del numero de puerto\n");
			printf("-cantidad o -u seguido del numero maximo de usuarios que maneja el server\n");
			printf("-archivo o -a seguido del path donde se encuentra el archivo con las palabras\n");
			return 0;
	        }

		if(strcmp(argv[i],"-cantidad") == 0 || strcmp(argv[i],"-c") ==0){
			cantidad = atoi(argv[i+1]);
			i++;
			b_cantidad = TRUE;
		}

		if(strcmp(argv[i],"-archivo") == 0 || strcmp(argv[i],"-a") == 0) {
			filePath = argv[i+1];
			i++;
			b_file = TRUE;
		}
	}




i++;
}

if(b_cantidad == FALSE){
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
		perror("Fallo al reservar memoria\n");
		fclose(file);
		return 1;
	}
	cantidadFrases++;
}

fclose(file);


////////////////////////////////////////////////////////////////////////////



SharedMemory* memoriaCompartida;

int sharedMemInt = shm_open("SHARED_MEM", O_RDWR | O_CREAT, 0600);

if(sharedMemInt == -1){
	printf("error abriendo la memoria compartida\n");
	exit(1);
}

ftruncate(sharedMemInt,sizeof(SharedMemory));

memoriaCompartida = (SharedMemory*)mmap(NULL,sizeof(SharedMemory),PROT_WRITE,MAP_SHARED,sharedMemInt,0);


memoriaCompartida->intentos = cantidad;
strcpy(memoriaCompartida->estadoPartida,"00000000");


close(sharedMemInt);

sem_t* mutex = sem_open("MUTEX",O_CREAT,0600,1);

if(mutex == SEM_FAILED){
	printf("error abriendo sem mutex\n");
	exit(1);
}


sem_t* cliente = sem_open("CLIENTE",O_CREAT,0600,0);

if(cliente == SEM_FAILED){
	printf("error abriendo sem cliente\n");
	exit(1);
}

sem_t* servidor = sem_open("SERVIDOR",O_CREAT,0600,0);

if(servidor == SEM_FAILED){
	printf("error abriendo sem servidor\n");
	exit(1);
}


while(1){
	printf("esperando al cliente\n");
	int indice = rand() % cantidadFrases;
	strcpy(memoriaCompartida->palabra,frases[indice]);
	sem_wait(cliente); //espero que se conecte 1 cliente

	printf("entró el cliente %s\n",memoriaCompartida->nickname);
	devolverPalabraJuego(memoriaCompartida->palabraCamuflada,memoriaCompartida->palabra);


	int estadoPartida = FALSE;


	TAG:
	while(memoriaCompartida->intentos > 0){
		sem_post(servidor); //le aviso al cliente que ya estoy
		sem_wait(cliente); //espero que el cliente haya metido una palabra


		if(strcmp(memoriaCompartida->estadoPartida,"exit")==0){
			//falta hacer funcion de contabilizar puntaje (el append en el file y sarasa)
			break;
		}

		//bandera generica
		int acierto=0;


		//loop para validar la letra que entro con la palabra posta y reflejar el cambio en la palabra camuflada
		for (int i = 0; memoriaCompartida->palabra[i] != '\0';i++){
			if(memoriaCompartida->palabra[i] == memoriaCompartida->ultimaLetra &&  memoriaCompartida->palabraCamuflada[i] != memoriaCompartida->ultimaLetra){
				memoriaCompartida->palabraCamuflada[i] = memoriaCompartida->ultimaLetra;
				acierto++;
			}
		}

		if (!acierto){
			memoriaCompartida->intentos--;
		}



		//si el cliente termino de hacer bien la palabra
		if (strcmp(memoriaCompartida->palabraCamuflada,memoriaCompartida->palabra) == 0){
			//falta implementar sumar 1 al puntaje
			
			indice = rand() % cantidadFrases;
		        strcpy(memoriaCompartida->palabra,frases[indice]);
		        devolverPalabraJuego(memoriaCompartida->palabraCamuflada,memoriaCompartida->palabra);
			goto TAG;
		}
	}

	//falta resetear las variables para que se pueda conectar el proximo cliente

}











return 0;
}
