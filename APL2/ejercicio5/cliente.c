#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 2048
#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[])
{

	////////////////////////////////////////////////////////////////////////////
	if (argc > 7)
	{
		printf("parametros invalidos\n");
		return 1;
	}

	char *serverIp;
	char *nickName;
	int portNumber;
	struct sockaddr_in socketAddr;

	int i = 0;
	bool port = FALSE;
	bool b_nickname = FALSE;
	bool server = FALSE;
	while (i <= argc)
	{
		if (argv[i] != NULL)
		{
			if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0)
			{
				printf("-puerto o -p para indicar el peurto del servidor de destino\n");
				printf("-servidor o -s para indicar la ipv4 del servidor de destino\n");
				printf("-nickname o -n para indicar el nickname del cliente\n");
				return 0;
			}

			if (strcmp(argv[i], "-puerto") == 0 || strcmp(argv[i], "-p") == 0)
			{
				portNumber = atoi(argv[i + 1]);
				i++;
				port = TRUE;
			}

			if (strcmp(argv[i], "-servidor") == 0 || strcmp(argv[i], "-s") == 0)
			{
				serverIp = argv[i + 1];
				int res = inet_pton(AF_INET, serverIp, &(socketAddr.sin_addr));
				if (res <= 0)
				{
					printf("direccion ipv4 invalida: %s\n", serverIp);
					return 1;
				}
				i++;
				server = TRUE;
			}

			if (strcmp(argv[i], "-nickname") == 0 || strcmp(argv[i], "-n") == 0)
			{
				nickName = argv[i + 1];
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
	if (server == FALSE)
	{
		printf("parametro server invalido o faltante\n");
		return 1;
	}
	if (port == FALSE)
	{
		printf("parametro puerto invalido o faltante\n");
		return 1;
	}

	////////////////////////////////////////////////////////////////////////////

	int sockfd;
	struct sockaddr_in server_addr;
	char msg_buffer[BUFFER_SIZE - 1024];
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];

	memset(msg_buffer, 0, BUFFER_SIZE - 1024);
	memset(send_buffer, 0, BUFFER_SIZE);
	memset(recv_buffer, 0, BUFFER_SIZE);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
	{
		perror("fallo la creacion del socket");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portNumber);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if (inet_pton(AF_INET, serverIp, &server_addr.sin_addr) <= 0)
	{
		perror("dirección de server invalida");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fallo al establecer la conexion");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Conectado al servidor en %s:%d\n Bienvenido al ahorcado!\n", serverIp, portNumber);

	while (1)
	{
		ssize_t n = recv(sockfd, recv_buffer, BUFFER_SIZE - 1, 0);

		if (n <= 0)
		{
			printf("No se recibio la señal de inicio de juego\n");
			break;
		}
		snprintf(send_buffer, BUFFER_SIZE, "%s", nickName);

		if (send(sockfd, send_buffer, strlen(send_buffer), 0) < 0)
		{
			// aca va la logiva de recupero de mensaje o notificacion de error
			perror("fallo el envio del nickname al server");
			break;
		}

		while (1)
		{
			ssize_t n = recv(sockfd, recv_buffer, BUFFER_SIZE - 1, 0);

			if (n <= 0)
			{
				printf("No se recibio la frase\n");
				break;
			}

			if (strcmp(recv_buffer, "PARTIDA_GANADA"))
			{
				printf("Felicidades! Has ganado la partida\n");
				break;
			}
			else if (strcmp(recv_buffer, "PARTIDA_PERDIDA"))
			{
				printf("Que pena! Has perdido la partida\n");
				break;
			}

			printf("La frase es: %s", recv_buffer);
			printf("Ingrese una letra: ");
			fflush(stdout);
			if (fgets(msg_buffer, BUFFER_SIZE - 1024, stdin) == NULL)
			{
				// aca va la logica de error en el buffer
				break;
			}

			// remover newline del mensaje
			msg_buffer[strcspn(msg_buffer, "\n")] = '\0';

			while (strlen(msg_buffer) > 1)
			{
				printf("Ingreso mas de un caracter\n Por favor ingrese una letra: ");
				fflush(stdout);
				if (fgets(msg_buffer, BUFFER_SIZE - 1024, stdin) == NULL)
				{
					// aca va la logica de error en el buffer
					break;
				}

				// remover newline del mensaje
				msg_buffer[strcspn(msg_buffer, "\n")] = '\0';
			}

			if (strcmp(send_buffer, "exit") == 0 || strcmp(send_buffer, "EXIT") == 0)
			{
				printf("desconectando");
				// aca va la logica de desconexion de cliente controlada
				break;
			}

			if (send(sockfd, msg_buffer, strlen(send_buffer), 0) < 0)
			{
				// aca va la logiva de recupero de mensaje o notificacion de error
				perror("Error al enviar la letra");
				break;
			}
		}
		printf("Desea iniciar otra partida?\nEscriba S o N\n");

		fflush(stdout);
		if (fgets(msg_buffer, BUFFER_SIZE - 1024, stdin) == NULL)
		{
			// aca va la logica de error en el buffer
			break;
		}

		// remover newline del mensaje
		msg_buffer[strcspn(msg_buffer, "\n")] = '\0';

		while (strlen(msg_buffer) > 1 || (msg_buffer[0] != 'S' && msg_buffer[0] != 'N'))
		{
			printf("Ingreso mas de un caracter y/o no ingreso un caracter valido\n Por favor ingrese S para continuar o N para finalizar el juego: ");
			fflush(stdout);
			if (fgets(msg_buffer, BUFFER_SIZE - 1024, stdin) == NULL)
			{
				// aca va la logica de error en el buffer
				break;
			}

			// remover newline del mensaje
			msg_buffer[strcspn(msg_buffer, "\n")] = '\0';
		}

		if (send(sockfd, msg_buffer, strlen(send_buffer), 0) < 0)
		{
			// aca va la logiva de recupero de mensaje o notificacion de error
			perror("Error al enviar la letra");
			break;
		}

		if (msg_buffer[0] == 'S')
			break;
	}

	close(sockfd);
	return 0;
}
