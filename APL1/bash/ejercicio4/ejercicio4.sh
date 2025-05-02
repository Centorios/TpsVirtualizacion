#!/bin/bash
# Integrantes del grupo:
# - Berti Rodrigo
# - Burnowicz Alejo
# - Fernandes Leonel
# - Federico Agustin

# Variables globales
PID_DIR="/tmp"
SCRIPT_NAME=$(basename "$0")
SELF_PATH="$(realpath "$0")"

# Función para mostrar uso
function mostrar_uso() {
    echo "Uso:"
    echo "  $SCRIPT_NAME -d <directorio> -s <backup_dir> -c <cantidad>"
    echo "  $SCRIPT_NAME -d <directorio> -k"
    exit 0
}

# Función para lanzar el demonio en segundo plano
function lanzar_demonio() {
    echo $@
    nohup "$SELF_PATH" --daemon "$@" &
    echo "Demonio lanzado para el directorio $DIRECTORIO"
    exit 0
}

# Función para detener el demonio
function detener_demonio() {
    PID_FILE="$PID_DIR/$(basename "$DIRECTORIO").pid"
    if [[ -f "$PID_FILE" ]]; then
        PID=$(cat "$PID_FILE")
        if kill "$PID" 2>/dev/null; then
            echo "Demonio detenido correctamente."
            rm -f "$PID_FILE"
        else
            echo "No se pudo detener el demonio."
        fi
    else
        echo "No hay demonio corriendo para el directorio $DIRECTORIO."
    fi
    exit 0
}

# Función principal del demonio
function demonio() {
    PID_FILE="$PID_DIR/$(basename "$DIRECTORIO").pid"
    echo $$ > "$PID_FILE"

    # Ordenar archivos existentes antes de empezar
    ordenar_archivos

    inotifywait -m -e create,moved_to --format "%f" "$DIRECTORIO" | while read ARCHIVO; do
        procesar_archivo "$ARCHIVO"
    done
}

# Función para ordenar archivos existentes
function ordenar_archivos() {
    for archivo in "$DIRECTORIO"/*; do
        if [[ -f "$archivo" ]]; then
            procesar_archivo "$(basename "$archivo")"
        fi
    done
}

# Función para procesar un archivo nuevo
function procesar_archivo() {
    local archivo="$1"
    local extension="${archivo##*.}"
    extension_upper=$(echo "$extension" | tr '[:lower:]' '[:upper:]')
    destino="$DESTINO/$extension_upper"
    mkdir -p "$destino"
    mv "$DIRECTORIO/$archivo" "$destino/"
    
    ((CONTADOR++))
    if (( CONTADOR >= CANTIDAD )); then
        generar_backup
        CONTADOR=0
    fi
}

# Función para generar backup
function generar_backup() {
    fecha=$(date '+%Y%m%d_%H%M%S')
    nombre_backup="$(basename "$DIRECTORIO")_${fecha}.zip"
    zip -r "$DESTINO/$nombre_backup" "$DIRECTORIO" > /dev/null
    echo "Backup generado: $nombre_backup"
}

# ===========================
#          MAIN
# ===========================

# Parseo de argumentos
DIRECTORIO=""
DESTINO=""
CANTIDAD=""
KILL_MODE=0
DAEMON_MODE=0
HELP_MODE=0

CONTADOR=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -d|--directorio)
            DIRECTORIO="$2"
            shift 2
            ;;
        -s|--salida|--backup)
            DESTINO="$2"
            shift 2
            ;;
        -c|--cantidad)
            CANTIDAD="$2"
            shift 2
            ;;
        -k|--kill)
            KILL_MODE=1
            shift
            ;;
        --daemon)
            DAEMON_MODE=1
            shift
            ;;
        -h|--help)
            HELP_MODE=1
            shift
            ;;
    esac
done
    
if ((HELP_MODE)); then
    mostrar_uso
fi

if (($KILL_MODE)); then
    if [[ -z "$DIRECTORIO" ]]; then
        echo "Necesita especificar el directorio asignado al script para matar"
    fi
    detener_demonio
fi

if [[ -z "$DIRECTORIO" || -z "$CANTIDAD" || -z "$DESTINO" ]]; then
    echo "Faltan elementos a especificar para ejecutar el script"
fi


if (( ! DAEMON_MODE )); then
    # Validar que no haya otro demonio para este directorio
    PID_FILE="$PID_DIR/$(basename "$DIRECTORIO").pid"
    if [[ -f "$PID_FILE" ]]; then
        PID=$(cat "$PID_FILE")
        if ps -p "$PID" > /dev/null 2>&1; then
            echo "Ya existe un demonio corriendo para $DIRECTORIO (PID: $PID)"
            exit 1
        else
            echo "PID muerto encontrado, limpiando..."
            rm -f "$PID_FILE"
        fi
    fi

    lanzar_demonio -d "$DIRECTORIO" -s "$DESTINO" -c "$CANTIDAD"
fi

# Si llegamos acá, es porque estamos en modo demonio
demonio
