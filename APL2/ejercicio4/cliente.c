
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
#include <arpa/inet.h>
#define BUFFER_SIZE 2048
#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[]){

    ////////////////////////////////////////////////////////////////////////////
    if (argc > 7)
    {
        printf("parametros invalidos\n");
        return 1;
    }

    char nickName[40];

    int i = 0;
    bool b_nickname = FALSE;
    while (i <= argc){
        if (argv[i] != NULL){
            if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0){
                printf("-puerto o -p para indicar el peurto del servidor de destino\n");
                printf("-servidor o -s para indicar la ipv4 del servidor de destino\n");
                printf("-nickname o -n para indicar el nickname del cliente\n");
                return 0;
            }

            if (strcmp(argv[i], "-nickname") == 0 || strcmp(argv[i], "-n") == 0){
                strcpy(nickName, argv[i + 1]);
                i++;
                b_nickname = TRUE;
            }
        }

        i++;
    }

    if (b_nickname == FALSE){
        printf("parametro nickname invalido o faltante\n");
        return 1;
    }

////////////////////////////////////////////////////////////////////////////