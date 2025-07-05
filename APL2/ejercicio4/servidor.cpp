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
#include <chrono>
#include <string.h>

using namespace std;

#define NOMBRE_SEMAFORO_SERVIDOR "semaforoServidor"
#define NOMBRE_SEMAFORO_CLIENTE "semaforoCliente"
#define NOMBRE_SEMAFORO_CLIENTE_UNICO "semaforoClienteUnico"
#define NOMBRE_MEMORIA "miMemoria"
#define NOMBRE_MEMORIA_RESPUESTA "miMemoriaRespuesta"
#define NOMBRE_MEMORIA_NICKNAME "miMemoriaNickname"

struct respuestaCliente {
    bool letraCorrecta;
    int intentosRestantes;
    bool partidaTerminada;
};

bool partida_en_curso = false;
bool finalizar_servidor = false;
bool finalizar_por_sigursr1 = false;

sem_t *semServidorGlobal = nullptr;

void manejador_seniales(int sig) {
    if (sig == SIGUSR1) {
        if (!partida_en_curso) {
            finalizar_servidor = true;
            cout << "[SIGUSR1] Finalizando servidor (no hay partida en curso)..." << endl;
        } else {
            cout << "[SIGUSR1] Esperando a que finalice la partida..." << endl;
            finalizar_por_sigursr1 = true;
        }
    } else if (sig == SIGUSR2) {
        finalizar_servidor = true;
        cout << "[SIGUSR2] Finalizando servidor inmediatamente..." << endl;
    }

    if(semServidorGlobal != nullptr){
        sem_post(semServidorGlobal); // Desbloquea el servidor si está esperando
    }
}

void mostrarAyuda() {
    cout << "Uso: ./servidor [OPCIONES]" << endl;
    cout << "Descripción:" << endl;
    cout << "Servidor para el juego de adivinanza de frases." << endl;
    cout << "Opciones:" << endl;
    cout << " -h                               Muestra este mensaje de ayuda" << endl;
    cout << " -a / --archivo <ruta archivo>    Archivo con frases (una por línea)" << endl;
    cout << " -c / --cantidad <num>            Intentos por partida (entero positivo)" << endl;
    cout << "Ejemplos:" << endl;
    cout << "  ./servidor -a frases.txt -c 5" << endl;
}

vector<string> obtenerFrases(const string &archivo) {
    vector<string> frases;
    ifstream file(archivo);
    string linea;
    while (getline(file, linea)) {
        if (!linea.empty()) frases.push_back(linea);
    }
    return frases;
}

void mostrarRanking(const vector<pair<pair<bool, string>, chrono::microseconds>> &ranking, const string &ganador, chrono::microseconds tiempoGanador) {
    cout << "\n=== RANKING ===" << endl;
    for (const auto &r : ranking) {
        cout << "Jugador: " << r.first.second << " - Resultado: " << (r.first.first ? "Ganó" : "Perdió") << " - Tiempo: " << chrono::duration_cast<chrono::milliseconds>(r.second).count() << " ms" << endl;
    }
    if (!ganador.empty()) {
        cout << "\nGanador: " << ganador << " con " << chrono::duration_cast<chrono::milliseconds>(tiempoGanador).count() << " ms" << endl;
    }
}

int main(int argc, char *argv[]) {
    // Señales
    signal(SIGINT, SIG_IGN);
    signal(SIGUSR1, manejador_seniales);
    signal(SIGUSR2, manejador_seniales);

    if (argc != 5) {
        mostrarAyuda();
        return 1;
    }

    string rutaArchivo;
    int intentosMax = 0;

    // Parámetros
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if ((arg == "-a" || arg == "--archivo") && i + 1 < argc) {
            rutaArchivo = argv[++i];
        } else if ((arg == "-c" || arg == "--cantidad") && i + 1 < argc) {
            intentosMax = stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            mostrarAyuda();
            return 0;
        }
    }

    if (rutaArchivo.empty() || intentosMax < 1) {
        cerr << "Parámetros inválidos." << endl;
        return 1;
    }

    // Limpieza automática si quedaron recursos colgados
    sem_t* testSem = sem_open(NOMBRE_SEMAFORO_CLIENTE_UNICO, 0);
    if (testSem != SEM_FAILED) {
        sem_close(testSem);
        cout << "[Aviso] Recursos anteriores encontrados. Se limpian automáticamente..." << endl;
        sem_unlink(NOMBRE_SEMAFORO_SERVIDOR);
        sem_unlink(NOMBRE_SEMAFORO_CLIENTE);
        sem_unlink(NOMBRE_SEMAFORO_CLIENTE_UNICO);
        shm_unlink(NOMBRE_MEMORIA);
        shm_unlink(NOMBRE_MEMORIA_NICKNAME);
        shm_unlink(NOMBRE_MEMORIA_RESPUESTA);
    }

    vector<string> frases = obtenerFrases(rutaArchivo);
    if (frases.empty()) {
        cerr << "El archivo de frases está vacío o no se pudo leer." << endl;
        return 1;
    }

    // Crear y mapear memoria compartida
    int shmLetra = shm_open(NOMBRE_MEMORIA, O_CREAT | O_RDWR, 0600);
    ftruncate(shmLetra, sizeof(char));
    char *letra = (char *)mmap(NULL, sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, shmLetra, 0);

    int shmNick = shm_open(NOMBRE_MEMORIA_NICKNAME, O_CREAT | O_RDWR, 0600);
    ftruncate(shmNick, 20);
    char *nickname = (char *)mmap(NULL, 20, PROT_READ | PROT_WRITE, MAP_SHARED, shmNick, 0);

    int shmResp = shm_open(NOMBRE_MEMORIA_RESPUESTA, O_CREAT | O_RDWR, 0600);
    ftruncate(shmResp, sizeof(respuestaCliente));
    respuestaCliente *respuesta = (respuestaCliente *)mmap(NULL, sizeof(respuestaCliente), PROT_READ | PROT_WRITE, MAP_SHARED, shmResp, 0);

    
    // Semáforos
    sem_t *semServidor = sem_open(NOMBRE_SEMAFORO_SERVIDOR, O_CREAT, 0600, 0);
    semServidorGlobal = semServidor;
    sem_t *semCliente = sem_open(NOMBRE_SEMAFORO_CLIENTE, O_CREAT, 0600, 0);
    sem_t *semClienteUnico = sem_open(NOMBRE_SEMAFORO_CLIENTE_UNICO, O_CREAT | O_EXCL, 0600, 1);
    if (semClienteUnico == SEM_FAILED) {
        cerr << "Ya hay un servidor corriendo (semáforo cliente único)." << endl;
        return 1;
    }

    // Ranking
    vector<pair<pair<bool, string>, chrono::microseconds>> ranking;
    string mejorJugador;
    chrono::microseconds mejorTiempo(0);

    cout << "Servidor iniciado. Esperando clientes..." << endl;

    while (!finalizar_servidor) {
        // Esperar a que un cliente se conecte
        sem_wait(semServidor);

        if (finalizar_servidor) {
            cout << "Servidor finalizado por señal." << endl;
            break;
        }

        if (sem_trywait(semClienteUnico) == -1) {
            cerr << "Cliente rechazado: ya hay uno conectado." << endl;
            continue;
        }

        partida_en_curso = true;
        auto inicio = chrono::high_resolution_clock::now();

        string frase = frases[rand() % frases.size()];
        string fraseAux = frase;
        fraseAux.erase(remove(fraseAux.begin(), fraseAux.end(), ' '), fraseAux.end());

        int intentos = intentosMax;
        bool gano = false;

        respuesta->letraCorrecta = false;
        respuesta->intentosRestantes = intentos;
        respuesta->partidaTerminada = false;

        cout << "Nueva partida con: " << nickname << endl;

        while (intentos > 0 && !gano && !finalizar_servidor) {
            sem_post(semCliente);
            sem_wait(semServidor);

            if(finalizar_servidor) {
                cout << "Partida cancelada por señal." << endl;
                break;
            }

            char l = *letra;
            bool ok = false;

            cout << "Frase restante: " << fraseAux << endl;

            auto it = find(fraseAux.begin(), fraseAux.end(), l);
            if (it != fraseAux.end()) {
                ok = true;
                fraseAux.erase(it);
                if (fraseAux.empty()) gano = true;
                cout << "Letra encontrada y eliminada: " << l << endl;
                cout << "Frase restante ahora: " << fraseAux << endl;
            }

            if (!ok) intentos--;  // Solo descuento si se equivocó

            respuesta->letraCorrecta = ok;
            respuesta->intentosRestantes = intentos;
            respuesta->partidaTerminada = (gano || intentos == 0);
            sem_post(semCliente);

            if (respuesta->partidaTerminada) {
                sem_wait(semServidor);
                break;
            }

            if(ok){
                ok = false;
            }

        }

        if (finalizar_por_sigursr1) {
            cout << "Partida cancelada por SIGUSR1." << endl;
            finalizar_servidor = true;
            break;
        }

        auto fin = chrono::high_resolution_clock::now();
        auto duracion = chrono::duration_cast<chrono::microseconds>(fin - inicio);
        ranking.push_back({{gano, string(nickname)}, duracion});
        if (gano && (mejorTiempo.count() == 0 || duracion < mejorTiempo)) {
            mejorTiempo = duracion;
            mejorJugador = string(nickname);
        }

        partida_en_curso = false;
        sem_post(semClienteUnico);

        //limpio la respuesta para el próximo cliente
        respuesta->letraCorrecta = false;
        respuesta->intentosRestantes = 0;
        respuesta->partidaTerminada = false;
    }

    mostrarRanking(ranking, mejorJugador, mejorTiempo);

    // Limpieza
    sem_close(semServidor);
    sem_close(semCliente);
    sem_close(semClienteUnico);
    sem_unlink(NOMBRE_SEMAFORO_SERVIDOR);
    sem_unlink(NOMBRE_SEMAFORO_CLIENTE);
    sem_unlink(NOMBRE_SEMAFORO_CLIENTE_UNICO);

    munmap(letra, sizeof(char));
    munmap(nickname, 20);
    munmap(respuesta, sizeof(respuestaCliente));
    close(shmLetra);
    close(shmNick);
    close(shmResp);
    shm_unlink(NOMBRE_MEMORIA);
    shm_unlink(NOMBRE_MEMORIA_NICKNAME);
    shm_unlink(NOMBRE_MEMORIA_RESPUESTA);

    cout << "Servidor finalizado correctamente." << endl;
    return 0;
}