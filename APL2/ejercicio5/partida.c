/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 100

void devolverPalabraJuego(char *destino, char *original);
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
    char palabraJuego[MAX_LEN];
    char palabraLeida;
    int coincidioLetra;

    devolverPalabraJuego(palabraJuego, palabra);

    while (contador < 5)
    {
        printf("Palabra actual: %s\n", palabraJuego);
        printf("Ingresa una letra: ");
        scanf("%s", palabraLeida);
        coincidioLetra = 0;

        int i = 0;
        while (palabraJuego[i] != "\0")
        {
            if (palabraJuego[i] != palabra[i] && palabra[i] == palabraLeida)
            {
                palabraJuego[i] = palabraLeida;
                coincidioLetra = 1;
            }

            if (!coincidioLetra)
                contador++;
            else if (strcmp(palabraJuego, palabra) == 0)
            {
                printf("¡Felicidades, has ganado la partida! La palabra era: %s\n", palabra);
                return 0;
            }
            i++;
        }

        printf("Qué pena, has perdido la partida. La palabra era: %s\n", palabra);
        return 1;
    }
}

void devolverPalabraJuego(char *destino, char *original)
{
    int i = 0;
    while (i >= 0)
    {
        if (original[i] == " ")
        {
            destino[i] = " ";
        }
        else if (original[i] == "\0")
        {
            destino[i] = "\0";
            i = -1;
        }
        else
        {
            destino[i] = "_";
        }
    }
}*/