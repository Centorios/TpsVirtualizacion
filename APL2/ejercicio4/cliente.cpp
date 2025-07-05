/*
# Integrantes del grupo:
# - Berti Rodrigo
# - Burnowicz Alejo
# - Fernandes Leonel
# - Federico Agustin
*/
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <algorithm>
#include <signal.h>
#include <csignal>
#include <cstring>

using namespace std;

#define NOMBRE_SEMAFORO_SERVIDOR "semaforoServidor"
#define NOMBRE_SEMAFORO_CLIENTE "semaforoCliente"
#define NOMBRE_MEMORIA "miMemoria"
#define NOMBRE_MEMORIA_RESPUESTA "miMemoriaRespuesta"
#define NOMBRE_MEMORIA_NICKNAME "miMemoriaNickname"

struct RespuestaServidor {
    bool letraCorrecta;
    int intentosRestantes;
    bool partidaTerminada;
};

char* letraADivinar = nullptr;
char* nicknameCliente = nullptr;
RespuestaServidor* respuesta = nullptr;
sem_t *semServidor = nullptr, *semCliente = nullptr;
int shmLetra, shmNick, shmResp;

void liberarRecursos() {
    if (respuesta) munmap(respuesta, sizeof(RespuestaServidor));
    if (nicknameCliente) munmap(nicknameCliente, 20);
    if (letraADivinar) munmap(letraADivinar, sizeof(char));
    if (semServidor) sem_close(semServidor);
    if (semCliente) sem_close(semCliente);
    close(shmLetra);
    close(shmNick);
    close(shmResp);
    // No hacer unlink acá
}



void mostrarAyuda() {
    cout << "Uso: ./cliente -n <nickname>" << endl;
    cout << "Descripción:" << endl;
    cout << "Cliente para el juego de adivinanza de frases." << endl;
    cout << "Opciones:" << endl;
    cout << " -h               Muestra este mensaje de ayuda" << endl;
    cout << " -n <nickname>    Nombre del jugador (requerido, max 20 chars)" << endl;
    cout << "Ejemplo:" << endl;
    cout << "  ./cliente -n jugador1" << endl;
}

int main(int argc, char* argv[]) {
    //ignorar SIGINT
    signal(SIGINT, SIG_IGN);

    string nick;
    if (argc < 2) {
        mostrarAyuda(); 
        return 1;
    }
    // procesar argumentos
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mostrarAyuda();
             return 0;
        } else if ((strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nickname") == 0) && i + 1 < argc) {
            nick = argv[++i];
        }
    }
    // validar nickname
    if (nick.empty() || nick.length() > 20) {
        cerr << "Nickname inválido." << endl;
        return 1;
    }
    //abro memorias compartidas
    shmLetra = shm_open(NOMBRE_MEMORIA, O_RDWR, 0600);
    shmNick  = shm_open(NOMBRE_MEMORIA_NICKNAME, O_RDWR, 0600);
    shmResp  = shm_open(NOMBRE_MEMORIA_RESPUESTA, O_RDWR, 0600);

    //verifico sus aperturas
    if (shmLetra == -1 || shmNick == -1 || shmResp == -1) {
        cerr << "Servidor no iniciado o error accediendo a memoria compartida." << endl;
        return 1;
    }
    //defino tamaños de las memorias
    ftruncate(shmLetra, sizeof(char));
    ftruncate(shmNick, 20);
    ftruncate(shmResp, sizeof(RespuestaServidor));

    //mapeamos las memorias compartidas
    letraADivinar = (char*)mmap(NULL, sizeof(char), PROT_WRITE, MAP_SHARED, shmLetra, 0);
    nicknameCliente = (char*)mmap(NULL, 20, PROT_WRITE, MAP_SHARED, shmNick, 0);
    respuesta = (RespuestaServidor*)mmap(NULL, sizeof(RespuestaServidor), PROT_READ, MAP_SHARED, shmResp, 0);

    //abro semaforos
    semServidor = sem_open(NOMBRE_SEMAFORO_SERVIDOR, 0);
    semCliente  = sem_open(NOMBRE_SEMAFORO_CLIENTE, 0);

    //verifico memorias y semaforos
    if (!letraADivinar || !nicknameCliente || !respuesta || semServidor == SEM_FAILED || semCliente == SEM_FAILED) {
        cerr << "Error al mapear recursos o abrir semáforos." << endl;
        liberarRecursos(); return 1;
    }
    //copio nombre cliente
    strncpy(nicknameCliente, nick.c_str(), 19);
    nicknameCliente[19] = '\0';
    //arranca el juego
    cout << "Conectado como: " << nicknameCliente << endl;
    sem_post(semServidor); // Avisar al servidor que estamos listos
    sem_wait(semCliente); // Esperar inicio

    cout << "Juego iniciado. Escriba letras (o 'exit' para salir)." << endl;
    string entrada;
    while (true) {
        cout << "Letra: ";
        cin >> entrada;

        if (entrada == "exit") {
            *letraADivinar = '#'; // Marca de salida
            sem_post(semServidor);
            break;
        }

        if (entrada.length() != 1 || !isalpha(entrada[0])) {
            cout << "Ingrese una sola letra válida." << endl;
            continue;
        }

        *letraADivinar = entrada[0];
        sem_post(semServidor);
        sem_wait(semCliente);

        cout << (respuesta->letraCorrecta ? "Correcto." : "Incorrecto.") << endl;
        cout << "Intentos restantes: " << respuesta->intentosRestantes << endl;

        if (respuesta->partidaTerminada) {
            cout << (respuesta->intentosRestantes ? "Ganaste!" : "Perdiste.") << endl;
            sem_post(semServidor); // Confirmar fin
            break;
        }
    }

    liberarRecursos();
    return 0;
}

