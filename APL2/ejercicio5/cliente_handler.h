#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

typedef struct 
{
	char *nickname;
	int *valor;
} Resultado;

// Declare the thread function
void* handle_client(void* arg);
int partida(char *palabra,int cliente_fd,char* buffer_entrada, char* buffer_salida,Resultado* Resultado);
void devolverPalabraJuego(char *destino,char *original);

#endif
