#!/bin/bash

# Script de Compilación para Sistema de Drones - Drone Wars 2
# Alternativa al Makefile para sistemas que no tengan make instalado

echo "=== COMPILANDO SISTEMA DE DRONES - DRONE WARS 2 ==="

# Verificar que gcc esté instalado
if ! command -v gcc &> /dev/null; then
    echo "Error: gcc no está instalado. Por favor instale GCC primero."
    exit 1
fi

echo "GCC encontrado: $(gcc --version | head -n1)"

# Crear directorios si no existen
mkdir -p obj
mkdir -p bin

# Flags de compilación
CFLAGS="-Wall -Wextra -std=c99 -g -O2"
LDFLAGS="-lpthread -lm"

# Archivos fuente
SOURCES=("utils.c" "communication.c" "drone.c" "swarm.c" "command_center.c" "main.c")
OBJECTS=()

echo "Compilando archivos fuente..."

# Compilar cada archivo fuente
for source in "${SOURCES[@]}"; do
    if [ -f "$source" ]; then
        obj_file="obj/${source%.c}.o"
        echo "  Compilando $source -> $obj_file"
        
        if gcc $CFLAGS -c "$source" -o "$obj_file"; then
            OBJECTS+=("$obj_file")
            echo "    ✓ Compilado exitosamente"
        else
            echo "    ✗ Error al compilar $source"
            exit 1
        fi
    else
        echo "  Advertencia: Archivo $source no encontrado"
    fi
done

echo ""
echo "Enlazando archivos objeto..."

# Enlazar todos los archivos objeto
if gcc "${OBJECTS[@]}" -o "bin/dronewars3" $LDFLAGS; then
    echo "  ✓ Enlazado exitosamente"
    echo ""
    echo "=== COMPILACIÓN COMPLETADA ==="
    echo "Ejecutable creado: bin/dronewars3"
    echo ""
    echo "Para ejecutar el programa:"
    echo "  ./bin/dronewars3"
    echo ""
    echo "Para ejecutar en modo interactivo:"
    echo "  ./bin/dronewars3 -i"
    echo ""
    echo "Para ver la ayuda:"
    echo "  ./bin/dronewars3 -h"
else
    echo "  ✗ Error al enlazar"
    exit 1
fi

# Verificar que el ejecutable se creó correctamente
if [ -f "bin/dronewars3" ]; then
    echo "Verificando ejecutable..."
    if file "bin/dronewars3" | grep -q "executable"; then
        echo "  ✓ Ejecutable verificado correctamente"
        
        # Mostrar información del ejecutable
        echo ""
        echo "Información del ejecutable:"
        ls -lh "bin/dronewars3"
        
        # Verificar dependencias
        echo ""
        echo "Verificando dependencias del ejecutable:"
        ldd "bin/dronewars3" 2>/dev/null || echo "  No se pudo verificar dependencias dinámicas"
        
    else
        echo "  ✗ El archivo generado no parece ser un ejecutable válido"
        exit 1
    fi
else
    echo "  ✗ El ejecutable no se creó"
    exit 1
fi

echo ""
echo "=== COMPILACIÓN FINALIZADA EXITOSAMENTE ==="
