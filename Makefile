# Makefile para el Sistema de Drones - Drone Wars 2
# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS = -lpthread -lm

# Directorios
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Archivos fuente
SOURCES = main.c utils.c communication.c drone.c swarm.c command_center.c
OBJECTS = $(SOURCES:%.c=$(OBJDIR)/%.o)

# Nombre del ejecutable
TARGET = $(BINDIR)/dronewars3

# Regla principal
all: directories $(TARGET)

# Crear directorios necesarios
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# Compilar el ejecutable principal
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Compilación completada: $@"

# Compilar archivos objeto
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Reglas específicas para archivos individuales
$(OBJDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/common.h $(SRCDIR)/communication.h $(SRCDIR)/drone.h $(SRCDIR)/swarm.h $(SRCDIR)/command_center.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/utils.o: $(SRCDIR)/utils.c $(SRCDIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/communication.o: $(SRCDIR)/communication.c $(SRCDIR)/communication.h $(SRCDIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/drone.o: $(SRCDIR)/drone.c $(SRCDIR)/drone.h $(SRCDIR)/common.h $(SRCDIR)/communication.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/swarm.o: $(SRCDIR)/swarm.c $(SRCDIR)/swarm.h $(SRCDIR)/common.h $(SRCDIR)/drone.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/command_center.o: $(SRCDIR)/command_center.c $(SRCDIR)/command_center.h $(SRCDIR)/common.h $(SRCDIR)/communication.h $(SRCDIR)/drone.h $(SRCDIR)/swarm.h
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Limpieza completada"

# Limpiar solo archivos objeto
clean-obj:
	rm -rf $(OBJDIR)
	@echo "Archivos objeto eliminados"

# Instalar el ejecutable
install: $(TARGET)
	@echo "Instalando en /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/
	@echo "Instalación completada"

# Desinstalar
uninstall:
	@echo "Desinstalando..."
	@sudo rm -f /usr/local/bin/dronewars3
	@echo "Desinstalación completada"

# Ejecutar el programa
run: $(TARGET)
	@echo "Ejecutando sistema de drones..."
	@$(TARGET)

# Ejecutar en modo interactivo
run-interactive: $(TARGET)
	@echo "Ejecutando sistema de drones en modo interactivo..."
	@$(TARGET) -i

# Ejecutar con configuración personalizada
run-config: $(TARGET)
	@echo "Ejecutando con configuración personalizada..."
	@$(TARGET) -c config.txt

# Verificar dependencias
check-deps:
	@echo "Verificando dependencias..."
	@which gcc > /dev/null || (echo "Error: gcc no encontrado" && exit 1)
	@echo "gcc encontrado: $(shell which gcc)"
	@echo "Versión: $(shell gcc --version | head -n1)"
	@echo "Dependencias verificadas correctamente"

# Mostrar información del proyecto
info:
	@echo "=== SISTEMA DE DRONES - DRONE WARS 2 ==="
	@echo "Compilador: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Librerías: $(LDFLAGS)"
	@echo "Archivos fuente: $(SOURCES)"
	@echo "Archivos objeto: $(OBJECTS)"
	@echo "Ejecutable: $(TARGET)"

# Ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  all              - Compilar todo el proyecto"
	@echo "  clean            - Limpiar todos los archivos generados"
	@echo "  clean-obj        - Limpiar solo archivos objeto"
	@echo "  install          - Instalar en /usr/local/bin"
	@echo "  uninstall        - Desinstalar del sistema"
	@echo "  run              - Ejecutar el programa"
	@echo "  run-interactive  - Ejecutar en modo interactivo"
	@echo "  run-config       - Ejecutar con configuración personalizada"
	@echo "  check-deps       - Verificar dependencias"
	@echo "  info             - Mostrar información del proyecto"
	@echo "  help             - Mostrar esta ayuda"

# Reglas especiales
.PHONY: all clean clean-obj install uninstall run run-interactive run-config check-deps info help directories

# Regla por defecto
.DEFAULT_GOAL := all
