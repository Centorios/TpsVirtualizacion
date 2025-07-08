
/*
Integrantes del grupo :
-Berti Rodrigo
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
#include <errno.h>

#define FALSE 0
#define TRUE 1

typedef struct
{
    char nickname[30];
    char ultimaLetra;
    char palabra[30];
    char palabraCamuflada[30];
    int intentos;
    char estadoPartida[30];

} SharedMemory;

int main(int argc, char *argv[])
{

    signal(SIGINT, SIG_IGN);

    ////////////////////////////////////////////////////////////////////////////
    if (argc > 7)
    {
        printf("parametros invalidos\n");
        return 1;
    }

    char nickName[40];

    int i = 0;
    bool b_nickname = FALSE;
    while (i <= argc)
    {
        if (argv[i] != NULL)
        {
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            {
                // printf("-puerto o -p para indicar el peurto del servidor de destino\n");
                // printf("-servidor o -s para indicar la ipv4 del servidor de destino\n");
                printf("Opciones :\n");
                printf("--nickname o -n para indicar el nickname del cliente\n");
                printf("--help o -h para mostrar esta ayuda\n");
                printf("Ejemplo: ./cliente -n nickName\n");
                return 0;
            }

            if (strcmp(argv[i], "-nickname") == 0 || strcmp(argv[i], "-n") == 0)
            {
                strcpy(nickName, argv[i + 1]);
                i++;
                b_nickname = TRUE;
            }
        }

        i++;
    }

    if (b_nickname == FALSE)
    {
        printf("parametro nickname invalido o faltante\n");
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////

    SharedMemory *memoriaCompartida;

    int sharedMemInt = shm_open("SHARED_MEM", O_RDWR, 0600);

    if (sharedMemInt == -1)
    {
        perror("shm_open"); // Imprime el error
        if (errno == EACCES)
        {
            printf("Error: Permisos insuficientes\n");
        }
        else if (errno == ENOENT)
        {
            printf("Error: Objeto no encontrado\n");
            printf("Asegúrate de que el servidor esté corriendo antes de iniciar el cliente.\n");
            exit(1);
        }
        else if (errno == EEXIST)
        {
            printf("Error: El objeto ya existe.\n");
        }
        else if (errno == EINVAL)
        {
            printf("Error: Nombre no válido\n");
        }
        else if (errno == EMFILE || errno == ENFILE)
        {
            printf("Error: Demasiados descriptores o archivos abiertos.\n");
        }
        exit(1);
    }

    ftruncate(sharedMemInt, sizeof(SharedMemory));

    memoriaCompartida = (SharedMemory *)mmap(NULL, sizeof(SharedMemory), PROT_WRITE, MAP_SHARED, sharedMemInt, 0);

    strcpy(memoriaCompartida->nickname, nickName);

    printf("nickname en la mem compartyda %s\n", memoriaCompartida->nickname);

    close(sharedMemInt);

    sem_t *cliente = sem_open("CLIENTE", O_RDWR, 0600, 0);

    if (cliente == SEM_FAILED)
    {
        printf("error abriendo sem cliente\n");
        exit(1);
    }

    sem_t *servidor = sem_open("SERVIDOR", O_RDWR, 0600, 0);

    if (servidor == SEM_FAILED)
    {
        printf("error abriendo sem servidor\n");
        exit(1);
    }

    sem_t *mutex = sem_open("MUTEX", O_CREAT, 0600, 1);
	// no se pudo abrir el semaforo MUTEX
	if (mutex == SEM_FAILED)
	{
		perror("error abriendo sem mutex");
		if (errno == EEXIST)
		{
			printf("el semaforo MUTEX ya existe, no se puede abrir\n");
		}
		else if (errno == EACCES)
		{
			printf("el semaforo MUTEX no se puede abrir, no tengo permisos\n");
		}
		else
		{
			printf("error desconocido abriendo sem MUTEX\n");
		}

		exit(1);
	}


    // se debe validar que un cliente no se conecte si no hay servidor activo
    

    sem_post(cliente); // me conecto a server

    while (memoriaCompartida->intentos > 0)
    {
        sem_wait(servidor); // espero a que el server haya seteado todo

        char palabraAEnviar[30] = "";

        fflush(stdin);
        fflush(stdout);
        
        printf("[server]: %s\n - Intentos restantes: %d\n", memoriaCompartida->palabraCamuflada, memoriaCompartida->intentos);

        fflush(stdin);
        fflush(stdout);
        scanf("%s", palabraAEnviar);
        fflush(stdin);
        fflush(stdout);

        //printf("[server]: %s\n", memoriaCompartida->palabraCamuflada);

        if (strcmp(palabraAEnviar, "exit") != 0)
        {
            memoriaCompartida->ultimaLetra = palabraAEnviar[0];
        }
        else
        {
            strcpy(memoriaCompartida->estadoPartida, palabraAEnviar);
            sem_post(cliente); // le aviso al servidor que meti exit asi lee la rta
            break;
        }

        sem_post(cliente); // le aviso a server que meti una palabra
    }

    return 0;
}
