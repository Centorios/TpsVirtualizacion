#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

void devolverPalabraJuego(char *destino, int largo);
int partida(const char *palabra);

int main()
{
    char palabra[] = "bienvenido";
    int resultadoPartida;
    int contadorVictorias = 0;
    char aux[MAX_LEN];

    while (1)
    {
        resultadoPartida = partida(palabra);
        if (!resultadoPartida)
            contadorVictorias++;

        printf("Su puntuacion actual es: %d\n¿Desea continuar? (si/no): ", contadorVictorias);
        scanf("%s", aux);

        if (strcmp(aux, "no") == 0)
            break;
    }

    return 0;
}

int partida(const char *palabra)
{
    int contador = 0;
    int largoPalabra = strlen(palabra);
    char palabraJuego[MAX_LEN];
    char palabraLeida[MAX_LEN];
    int coincidioLetra;

    devolverPalabraJuego(palabraJuego, largoPalabra);

    while (contador < 5)
    {
        printf("Palabra actual: %s\n", palabraJuego);
        printf("Ingresa una letra o la palabra completa: ");
        scanf("%s", palabraLeida);
        coincidioLetra = 0;

        if (strlen(palabraLeida) > 1)
        {
            if (strcmp(palabraLeida, palabra) == 0)
            {
                printf("¡Felicidades, has ganado la partida! La palabra era: %s\n", palabra);
                return 0;
            }
            else
                contador++;
        }
        else
        {
            for (int i = 0; i < largoPalabra; i++)
            {
                if (palabraJuego[i] != palabra[i] && palabra[i] == palabraLeida[0])
                {
                    palabraJuego[i] = palabraLeida[0];
                    coincidioLetra = 1;
                }
            }

            if (!coincidioLetra)
                contador++;
            else if (strcmp(palabraJuego, palabra) == 0)
            {
                printf("¡Felicidades, has ganado la partida! La palabra era: %s\n", palabra);
                return 0;
            }
        }
    }

    printf("Qué pena, has perdido la partida. La palabra era: %s\n", palabra);
    return 1;
}

void devolverPalabraJuego(char *destino, int largo)
{
    for (int i = 0; i < largo; i++)
        destino[i] = '_';
    destino[largo] = '\0';
}