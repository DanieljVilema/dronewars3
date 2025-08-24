#ifndef COMMAND_CENTER_H
#define COMMAND_CENTER_H

#include "common.h"
#include "communication.h"
#include "drone.h"
#include "swarm.h"

// Forward declaration - la estructura CentroComando ya está definida en common.h

// Funciones de Creación del Centro de Comando
CentroComando* crear_centro_comando(void);
void destruir_centro_comando(CentroComando* centro);
int inicializar_centro_comando(CentroComando* centro);

// Funciones de Gestión del Sistema
int iniciar_sistema_drones(CentroComando* centro);
int detener_sistema_drones(CentroComando* centro);
int pausar_sistema_drones(CentroComando* centro);
int reanudar_sistema_drones(CentroComando* centro);

// Funciones de Control de Enjambres
int crear_enjambres_sistema(CentroComando* centro);
int asignar_blancos_enjambres(CentroComando* centro);
int activar_enjambres_sistema(CentroComando* centro);
int monitorear_enjambres_sistema(CentroComando* centro);

// Funciones de Gestión de Camiones
int crear_camiones_sistema(CentroComando* centro);
int asignar_drones_camiones(CentroComando* centro);
int monitorear_camiones_sistema(CentroComando* centro);

// Funciones de Gestión de Blancos
int crear_blancos_sistema(CentroComando* centro);
int monitorear_estado_blancos(CentroComando* centro);
int evaluar_destruccion_blancos(CentroComando* centro);

// Funciones de Comunicación
int procesar_mensajes_entrantes(CentroComando* centro);
int enviar_comandos_drones(CentroComando* centro);
int verificar_estado_comunicacion(CentroComando* centro);

// Funciones de Monitoreo
int mostrar_estado_sistema(CentroComando* centro);
int generar_reporte_sistema(CentroComando* centro);
int verificar_estado_general(CentroComando* centro);

// Hilos del Sistema
void* hilo_principal_centro_comando(void* arg);
void* hilo_monitoreo_sistema(void* arg);
void* hilo_comunicacion_sistema(void* arg);

// Funciones de Utilidad
int cargar_configuracion_sistema(CentroComando* centro);
int inicializar_estructuras_sistema(CentroComando* centro);
int limpiar_recursos_sistema(CentroComando* centro);

#endif // COMMAND_CENTER_H
