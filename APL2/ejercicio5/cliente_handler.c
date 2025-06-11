#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "cliente_handler.h"

#define BUFFER_SIZE 2048
#define MAX_LINEA 2048
#define MAX_FRASES 100
#define CANT_INTENTOS 6

typedef struct {
	int *cantidadFrases;
	char **frases;
	int *cliente_id; 
}ParametrosThread;


void* handle_client(void* arg) {
    
	//ParametrosThread *parametros = (ParametrosThread*)arg;
	ParametrosThread *parametros;
	parametros=malloc(sizeof(ParametrosThread));
	memcpy(parametros,arg,sizeof(ParametrosThread));

    char buffer_entrada[BUFFER_SIZE];
    char buffer_salida[BUFFER_SIZE];
    char palabraAEvaluar[MAX_LINEA];

    int indice = rand() % *(parametros->cantidadFrases);
    strcpy(palabraAEvaluar, parametros->frases[indice]);

    printf("Handling client in thread (fd: %d)\n", *(parametros->cliente_id));

    memset(buffer_entrada, 0, BUFFER_SIZE);
    memset(buffer_salida, 0, BUFFER_SIZE);

    Resultado resultado;
    resultado.nickname = malloc(100); // Asigna memoria para el nickname
    int puntaje = 0;
    resultado.valor = &puntaje;

    while (1) {
        partida(palabraAEvaluar, *(parametros->cliente_id), buffer_entrada, buffer_salida, &resultado);

        ssize_t n = recv(*(parametros->cliente_id), buffer_entrada, BUFFER_SIZE - 1, 0);
        if (n <= 0 || buffer_entrada[0] == 'N') {
            break;
        }
    }

    printf("Cliente %s obtuvo puntaje: %d\n", resultado.nickname, *(resultado.valor));

    free(resultado.nickname);
    close(*(parametros->cliente_id));
    printf("Client connection closed (fd: %d)\n", *(parametros->cliente_id));
    free(parametros); // Liberar la memoria pasada como argumento
    return NULL;
}

int partida(char *palabra, int cliente_fd, char* buffer_entrada, char* buffer_salida, Resultado* resultado) {
    int contador = 0;
    char palabraJuego[MAX_LINEA];
    char letraLeida;
    int coincidioLetra;

    snprintf(buffer_salida, BUFFER_SIZE, "INICIO_PARTIDA");
    if (send(cliente_fd, buffer_salida, strlen(buffer_salida), 0) < 0) {
        perror("fallo el envio de inicio de partida a Cliente");
        return 1;
    }

    ssize_t n = recv(cliente_fd, buffer_entrada, BUFFER_SIZE - 1, 0);
    if (n <= 0) return 1;

    strcpy(resultado->nickname, buffer_entrada);

    devolverPalabraJuego(palabraJuego, palabra);

    while (contador < CANT_INTENTOS) {
        snprintf(buffer_salida, BUFFER_SIZE, "%s", palabraJuego);
        send(cliente_fd, buffer_salida, strlen(buffer_salida), 0);

        n = recv(cliente_fd, buffer_entrada, BUFFER_SIZE - 1, 0);
        if (n <= 0) return 1;

        letraLeida = buffer_entrada[0];
        coincidioLetra = 0;

        for (int i = 0; palabraJuego[i] != '\0'; i++) {
            if (palabraJuego[i] != palabra[i] && palabra[i] == letraLeida) {
                palabraJuego[i] = letraLeida;
                coincidioLetra = 1;
            }
        }

        if (!coincidioLetra)
            contador++;
        else if (strcmp(palabraJuego, palabra) == 0) {
            strcpy(buffer_salida, "PARTIDA_GANADA");
            send(cliente_fd, buffer_salida, strlen(buffer_salida), 0);
            (*resultado->valor)++;
            return 0;
        }
    }

    strcpy(buffer_salida, "PARTIDA_PERDIDA");
    send(cliente_fd, buffer_salida, strlen(buffer_salida), 0);
    return 1;
}

void devolverPalabraJuego(char *destino, char *original) {
    int i;
    for (i = 0; original[i] != '\0'; i++) {
        destino[i] = (original[i] == ' ') ? ' ' : '_';
    }
    destino[i] = '\0';
}