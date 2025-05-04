#!/usr/bin/env pwsh

# Integrantes del grupo:
# - Berti Rodrigo
# - Burnowicz Alejo
# - Fernandes Leonel
# - Federico Agustin

param(
    [string]$directorio,
    [string]$backup,
    [int]$cantidad,
    [switch]$kill,
    [switch]$daemon,
    [switch]$help
)

# Variables globales
$PID_DIR = "/tmp"
$SCRIPT_NAME = Split-Path -Leaf $MyInvocation.MyCommand.Path
$SELF_PATH = (Get-Item $MyInvocation.MyCommand.Path).FullName

# Función para mostrar uso
function Mostrar-Uso {
    Write-Output "Uso:"
    Write-Output "  .\$SCRIPT_NAME -directorio <directorio> -backup <backup_dir> -cantidad <cantidad>"
    Write-Output "  .\$SCRIPT_NAME -directorio <directorio> -kill"
    exit 0
}

# Función para lanzar el demonio en segundo plano
function Lanzar-Demonio {
    Start-Process -FilePath "pwsh" -ArgumentList "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "`"$SELF_PATH`" --daemon -directorio `"$directorio`" -backup `"$backup`" -cantidad $cantidad"
    Write-Output "Demonio lanzado para el directorio $directorio"
    exit 0
}

# Función para detener el demonio
function Detener-Demonio {
    $pidFile = Join-Path $PID_DIR ("$(Split-Path $directorio -Leaf).pid")
    if (Test-Path $pidFile) {
        $pidOriginal = Get-Content $pidFile
        try {
            Stop-Process -Id $pidOriginal -ErrorAction Stop
            Write-Output "Demonio detenido correctamente."
            Remove-Item $pidFile -Force
        } catch {
            Write-Output "No se pudo detener el demonio."
        }
    } else {
        Write-Output "No hay demonio corriendo para el directorio $directorio."
    }
    exit 0
}

# Función para ordenar archivos existentes
function Ordenar-Archivos {
    Get-ChildItem -Path $directorio -File | ForEach-Object {
        Procesar-Archivo $_.Name
    }
}

# Función para procesar un archivo
function Procesar-Archivo($archivo) {
    $extension = ($archivo | Split-Path -Extension).TrimStart('.')
    $extensionUpper = $extension.ToUpper()
    $destino = Join-Path $directorio $extensionUpper
    if (-not (Test-Path $destino)) {
        New-Item -ItemType Directory -Path $destino | Out-Null
    }
    Move-Item -Path (Join-Path $directorio $archivo) -Destination $destino

    $script:contador++
    if ($script:contador -ge $cantidad) {
        Generar-Backup
        $script:contador = 0
    }
}

# Función para generar backup
function Generar-Backup {
    $fecha = Get-Date -Format "yyyyMMdd_HHmmss"
    $nombreBackup = "$(Split-Path $directorio -Leaf)_$fecha.zip"
    Compress-Archive -Path (Join-Path $directorio '*') -DestinationPath (Join-Path $backup $nombreBackup)
    Write-Output "Backup generado: $nombreBackup"
}

# Función principal del demonio
function Demonio {
    $pidFile = Join-Path $PID_DIR ("$(Split-Path $directorio -Leaf).pid")
    $PID | Out-File $pidFile -Force

    Ordenar-Archivos

    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = (Resolve-Path $directorio).Path
    $watcher.IncludeSubdirectories = $false
    $watcher.EnableRaisingEvents = $true
    $watcher.Filter = '*'

    Register-ObjectEvent $watcher Created -SourceIdentifier FileCreated -Action {
        $nombreArchivo = $Event.SourceEventArgs.Name
        Start-Sleep -Milliseconds 500
        Procesar-Archivo $nombreArchivo
    }

    Register-ObjectEvent $watcher Changed -SourceIdentifier FileChanged -Action {
        $nombreArchivo = $Event.SourceEventArgs.Name
        Start-Sleep -Milliseconds 500
        Procesar-Archivo $nombreArchivo
    }

    while ($true) {
        Start-Sleep -Seconds 5
    }
}

# ===========================
#          MAIN
# ===========================

# Validaciones
if ($help) {
    Mostrar-Uso
}

if ($kill) {
    if (-not $directorio) {
        Write-Output "Necesita especificar el directorio asignado al script para matar"
        exit 1
    }
    Detener-Demonio
}

if (-not $directorio -or -not $cantidad -or -not $backup) {
    Write-Output "Faltan elementos a especificar para ejecutar el script"
    exit 1
}

# Verificar si ya hay demonio corriendo
$pidFile = Join-Path $PID_DIR ("$(Split-Path $directorio -Leaf).pid")
if (-not $daemon) {
    if (Test-Path $pidFile) {
        $pidOriginal = Get-Content $pidFile
        if (Get-Process -Id $pidOriginal -ErrorAction SilentlyContinue) {
            Write-Output "Ya existe un demonio corriendo para $directorio (PID: $pidOriginal)"
            exit 1
        } else {
            Write-Output "PID muerto encontrado, limpiando..."
            Remove-Item $pidFile -Force
        }
    }
    Lanzar-Demonio
}

# Si llegamos aquí, estamos en modo demonio
$script:contador = 0
Demonio