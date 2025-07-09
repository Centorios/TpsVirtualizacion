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
#include <errno.h>
#include <time.h>

#define FALSE 0
#define TRUE 1
#define MAX_FRASES 3
#define MAX_LINEA 30

typedef struct
{
	char nickname[30];
	char ultimaLetra;
	char palabra[30];
	char palabraCamuflada[30];
	int intentos;
	char estadoPartida[30];

} SharedMemory;

typedef struct
{
	char nickname[30];
	int puntaje;
	int tiempoJuego; // en segundos
} Jugador;

bool termProcess = FALSE;
bool estadoPartida = FALSE;
bool finalizarPartida = FALSE;

void devolverPalabraJuego(char *destino, char *original)
{
	int i;
	for (i = 0; original[i] != '\0'; i++)
	{
		destino[i] = (original[i] == ' ') ? ' ' : '_';
	}
	destino[i] = '\0';
}

void sigusrHandler(int signal)
{
	// necesito una variable para poder finalizar al servidor. completar
	switch (signal)
	{
	case SIGUSR1:
		printf("SIGUSR1 recibido, cerrando servidor...\n");
		finalizarPartida = TRUE;
		termProcess = TRUE;
		break;
	case SIGUSR2:
		printf("SIGUSR2 recibido, esperando a finalizar la partida para cerrar el servidor...\n");
		finalizarPartida = TRUE;
	}
}

// Función de comparación para qsort
int compararJugadores(const void *a, const void *b)
{
	const Jugador *jugadorA = (const Jugador *)a;
	const Jugador *jugadorB = (const Jugador *)b;

	// Primero comparar por puntaje (de mayor a menor)
	if (jugadorB->puntaje != jugadorA->puntaje)
	{
		return jugadorB->puntaje - jugadorA->puntaje;
	}

	// En caso de empate, comparar por tiempo de juego (de menor a mayor)
	return jugadorA->tiempoJuego - jugadorB->tiempoJuego;
}

void generarRanking(Jugador *jugadores, int cantidadJugadores)
{
	// Ordenar el arreglo de jugadores usando qsort
	qsort(jugadores, cantidadJugadores, sizeof(Jugador), compararJugadores);
}

void mostrarRanking(Jugador *jugadores)
{
	printf("Ranking de jugadores:\n");
	for (int i = 0; i < 50; i++)
	{
		if (jugadores[i].puntaje > 0)
		{
			printf("Jugador: %s, Puntaje: %d, Tiempo de juego: %d segundos\n",
				   jugadores[i].nickname, jugadores[i].puntaje, jugadores[i].tiempoJuego);
		}
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, SIG_IGN);
	signal(SIGUSR1, sigusrHandler);
	signal(SIGUSR2, sigusrHandler);

	/////////////////////////////////////////////////////////////////////
	if (argc > 5)
	{
		printf("parametros invalidos\n");
		return 1;
	}

	char *filePath = NULL;

	int i = 0;
	int cantidad = 0;
	bool b_cantidad = FALSE;
	bool b_file = FALSE;
	while (i < argc)
	{
		if (argv[i] != NULL)
		{
			if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
			{
				printf("Opciones :\n");
				printf("--help o -h para mostrar la ayuda\n");
				printf("--cantidad o -c seguido del numero maximo de usuarios que maneja el server\n");
				printf("--archivo o -a seguido del path donde se encuentra el archivo con las palabras\n");
				printf("Ejemplo: ./server -c 5 -a ./archivos/prueba.txt\n");
				return 0;
			}

			if (strcmp(argv[i], "--cantidad") == 0 || strcmp(argv[i], "-c") == 0)
			{
				cantidad = atoi(argv[i + 1]);
				i++;
				b_cantidad = TRUE;
			}

			if (strcmp(argv[i], "--archivo") == 0 || strcmp(argv[i], "-a") == 0)
			{
				filePath = argv[i + 1];
				i++;
				b_file = TRUE;
			}
		}
		i++;
	}

	if (b_cantidad == FALSE)
	{
		printf("parametro usuarios invalido o faltante\n");
		return 1;
	}

	if (b_file == FALSE)
	{
		printf("parametro archivo invalido o faltante\n");
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////
	FILE *file = fopen(filePath, "r");
	if (!file)
	{
		perror("Error abriendo archivo");
		return 1;
	}
	char *frases[MAX_FRASES];
	char bufferArchivo[MAX_LINEA];
	int cantidadFrases = 0;

	while (fgets(bufferArchivo, MAX_LINEA, file) && cantidadFrases < MAX_FRASES)
	{
		// Eliminar el salto de línea final si existe
		bufferArchivo[strcspn(bufferArchivo, "\n")] = '\0';
		// Reservar memoria y copiar la línea
		frases[cantidadFrases] = strdup(bufferArchivo);
		if (!frases[cantidadFrases])
		{
			perror("Fallo al reservar memoria\n");
			fclose(file);
			return 1;
		}
		cantidadFrases++;
	}

	fclose(file);

	////////////////////////////////////////////////////////////////////////////

	SharedMemory *memoriaCompartida;

	int sharedMemInt = shm_open("SHARED_MEM", O_RDWR | O_CREAT, 0600);

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

	memoriaCompartida->intentos = cantidad;
	strcpy(memoriaCompartida->estadoPartida, "00000000");

	close(sharedMemInt);

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

	sem_t *finalizacion = sem_open("FINALIZACION", O_CREAT, 0600, 0);

	if (finalizacion == SEM_FAILED)
	{
		perror("error abriendo sem finalizacion");
		if (errno == EEXIST)
		{
			printf("el semaforo FINALIZACION ya existe, no se puede abrir\n");
		}
		else if (errno == EACCES)
		{
			printf("el semaforo FINALIZACION no se puede abrir, no tengo permisos\n");
		}
		else
		{
			printf("error desconocido abriendo sem FINALIZACION\n");
		}
		exit(1);
	}

	sem_t *cliente = sem_open("CLIENTE", O_CREAT, 0600, 0);
	// no se pudo abrir el semaforo CLIENTE
	if (cliente == SEM_FAILED)
	{
		perror("error abriendo sem cliente");
		if (errno == EEXIST)
		{
			printf("el semaforo CLIENTE ya existe, no se puede abrir\n");
		}
		else if (errno == EACCES)
		{
			printf("el semaforo CLIENTE no se puede abrir, no tengo permisos\n");
		}
		else
		{
			printf("error desconocido abriendo sem CLIENTE\n");
		}
		exit(1);
	}

	sem_t *servidor = sem_open("SERVIDOR", O_CREAT, 0600, 0);
	// no se pudo abrir el semaforo SERVIDOR
	if (servidor == SEM_FAILED)
	{
		perror("error abriendo sem servidor");
		if (errno == EEXIST)
		{
			printf("el semaforo SERVIDOR ya existe, no se puede abrir\n");
		}
		else if (errno == EACCES)
		{
			printf("el semaforo SERVIDOR no se puede abrir, no tengo permisos\n");
		}
		else
		{
			printf("error desconocido abriendo sem SERVIDOR\n");
		}
		exit(1);
	}

	if (servidor == SEM_FAILED)
	{
		perror("error abriendo sem servidor");
		if (errno == EEXIST)
		{
			printf("el semaforo SERVIDOR ya existe, no se puede abrir\n");
		}
		else if (errno == EACCES)
		{
			printf("el semaforo SERVIDOR no se puede abrir, no tengo permisos\n");
		}
		else
		{
			printf("error desconocido abriendo sem SERVIDOR\n");
		}
		exit(1);
	}
	Jugador jugadores[50];	  // arreglo de jugadores, maximo 50 jugadores
	jugadores[0].puntaje = 0; // inicializo el primer jugador con puntaje 0

	int cantJugadores = 0;

	struct timespec ts;
	int valorSemSer;

	// lógica del servidor
TAG2:
	while (!finalizarPartida)
	{
		printf("esperando al cliente\n");

		int indice = rand() % cantidadFrases;
		strcpy(memoriaCompartida->palabra, frases[indice]);

		if (!estadoPartida & finalizarPartida)
		{
			printf("Finalizando servidor...\n");
			break;
		}

		// sem_wait(cliente); // espero que se conecte 1 cliente
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += 5;

		if (sem_timedwait(cliente, &ts) == -1)
		{
			goto TAG2;
		}

		printf("entró el cliente %s\n", memoriaCompartida->nickname);

		//sem_wait(finalizacion);

		devolverPalabraJuego(memoriaCompartida->palabraCamuflada, memoriaCompartida->palabra);

		// inicializo al jugador en 0
		jugadores[cantJugadores].puntaje = 0;
		jugadores[cantJugadores].tiempoJuego = 0;
		strcpy(jugadores[cantJugadores].nickname, memoriaCompartida->nickname);

		estadoPartida = TRUE; // para el sigusr1
		

		time_t tiempoInicio = time(NULL);

	TAG:
		while (memoriaCompartida->intentos > 0 && !termProcess)
		{
			sem_getvalue(servidor, &valorSemSer);
			if (!valorSemSer)
			{
				sem_post(servidor);
			} // le aviso al cliente que ya estoy

			clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += 5;

			if (sem_timedwait(cliente, &ts) == -1)
			{
				goto TAG;
			}

			// sem_wait(cliente);	// espero que el cliente haya metido una palabra

			if (strcmp(memoriaCompartida->estadoPartida, "exit") == 0)
			{
				// falta hacer funcion de contabilizar puntaje (el append en el file y sarasa)
				time_t tiempoFin = time(NULL);
				int tiempoJuego = difftime(tiempoFin, tiempoInicio);
				printf("El cliente %s ha salido del juego.\n", memoriaCompartida->nickname);
				printf("Puntaje final: %d\n", jugadores[cantJugadores].puntaje);
				printf("Tiempo de juego: %d segundos\n", tiempoJuego);
				jugadores[cantJugadores].tiempoJuego = tiempoJuego;
				// strcpy(memoriaCompartida->estadoPartida, "00000000");

				// sem_post(mutex);
				break;
			}

			// bandera generica
			int acierto = 0;

			// loop para validar la letra que entro con la palabra posta y reflejar el cambio en la palabra camuflada
			for (int i = 0; memoriaCompartida->palabra[i] != '\0'; i++)
			{
				if (memoriaCompartida->palabra[i] == memoriaCompartida->ultimaLetra && memoriaCompartida->palabraCamuflada[i] != memoriaCompartida->ultimaLetra)
				{
					memoriaCompartida->palabraCamuflada[i] = memoriaCompartida->ultimaLetra;
					acierto++;
				}
			}

			if (!acierto)
			{
				memoriaCompartida->intentos--;
			}

			// si el cliente termino de hacer bien la palabra
			if (strcmp(memoriaCompartida->palabraCamuflada, memoriaCompartida->palabra) == 0)
			{

				time_t tiempoFin = time(NULL);
				int tiempoJuego = difftime(tiempoFin, tiempoInicio);
				printf("El cliente %s ha ganado!\n", memoriaCompartida->nickname);
				printf("Tiempo de juego: %d segundos\n", tiempoJuego);

				jugadores[cantJugadores].puntaje += 1;
				jugadores[cantJugadores].tiempoJuego = tiempoJuego;

				indice = rand() % cantidadFrases;
				strcpy(memoriaCompartida->palabra, frases[indice]);

				tiempoInicio = time(NULL);

				// strcpy(memoriaCompartida->estadoPartida, "00000000");

				devolverPalabraJuego(memoriaCompartida->palabraCamuflada, memoriaCompartida->palabra);
				memoriaCompartida->intentos = cantidad;
				printf("Nueva palabra: %s\n", memoriaCompartida->palabraCamuflada);
				printf("La palabra nueva es: %s\n", memoriaCompartida->palabra);
				if (finalizarPartida)
				{
					break;
				}
				goto TAG;
			}
		}
		strcpy(memoriaCompartida->estadoPartida, "exit");
		sem_wait(finalizacion);
		if (!finalizarPartida)
		{

			strcpy(memoriaCompartida->estadoPartida, "00000000");
			// sem_post(mutex);
			cantJugadores++;

			// falta resetear las variables para que se pueda conectar el proximo cliente
			memoriaCompartida->intentos = cantidad;
			estadoPartida = FALSE;
		}
	}
	strcpy(memoriaCompartida->estadoPartida, "finalizando");


	generarRanking(jugadores, cantJugadores);
	mostrarRanking(jugadores);

	// limpiar los recursos cuando se termine el servidor
	sem_close(mutex);
	sem_close(cliente);
	sem_close(servidor);
	sem_close(finalizacion);
	sem_unlink("MUTEX");
	sem_unlink("CLIENTE");
	sem_unlink("SERVIDOR");
	sem_unlink("FINALIZACION");
	munmap(memoriaCompartida, sizeof(SharedMemory));
	close(sharedMemInt);
	shm_unlink("SHARED_MEM");
	printf("Servidor finalizado.\n");

	return 0;
}
