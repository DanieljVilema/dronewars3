#ifndef SWARM_H
#define SWARM_H

#include "common.h"
#include "drone.h"

// Forward declaration - la estructura Enjambre ya está definida en common.h

// Estructura para el control de enjambres
typedef struct {
    int num_enjambres;
    Enjambre* enjambres[MAX_ENJAMBRES];
    pthread_mutex_t mutex_sistema;
    pthread_t hilo_reconformacion;
} ControlEnjambres;

// Funciones de Creación de Enjambres
Enjambre* crear_enjambre(int id, int id_blanco, Coordenada zona_ensamblaje, Coordenada posicion_blanco);
void destruir_enjambre(Enjambre* enjambre);
int inicializar_enjambre(Enjambre* enjambre);

// Funciones de Gestión de Drones en Enjambres
int agregar_drone_enjambre(Enjambre* enjambre, Drone* drone);
int remover_drone_enjambre(Enjambre* enjambre, int id_drone);
int verificar_ensamblaje_completo_enjambre(Enjambre* enjambre);
int activar_enjambre(Enjambre* enjambre);

// Funciones de Control de Enjambres
ControlEnjambres* crear_control_enjambres(void);
void destruir_control_enjambres(ControlEnjambres* control);
int agregar_enjambre_control(ControlEnjambres* control, Enjambre* enjambre);
int remover_enjambre_control(ControlEnjambres* control, int id_enjambre);

// Funciones de Reconformación de Enjambres
int identificar_enjambres_incompletos(ControlEnjambres* control, int* enjambres_incompletos, int* num_incompletos);
int buscar_drones_disponibles(ControlEnjambres* control, int id_enjambre_origen, int* drones_disponibles, int* num_disponibles);
int reasignar_drone_enjambre(Enjambre* enjambre_origen, Enjambre* enjambre_destino, int id_drone);
int ejecutar_reconformacion_enjambres(ControlEnjambres* control);

// Funciones de Estado del Enjambre
EstadoEnjambre obtener_estado_enjambre(Enjambre* enjambre);
int actualizar_estado_enjambre(Enjambre* enjambre, EstadoEnjambre nuevo_estado);
int verificar_estado_enjambre(Enjambre* enjambre);

// Funciones de Coordinación
int coordinar_ataque_enjambre(Enjambre* enjambre);
int coordinar_ensamblaje_enjambre(Enjambre* enjambre);
int coordinar_reensamblaje_enjambre(Enjambre* enjambre);

// Hilos de Control
void* hilo_control_enjambre(void* arg);
void* hilo_reconformacion_enjambres(void* arg);

// Funciones de Utilidad
int calcular_centro_enjambre(Enjambre* enjambre, Coordenada* centro);
int verificar_proximidad_drones(Enjambre* enjambre, int radio_proximidad);
int asignar_objetivos_enjambre(Enjambre* enjambre);

#endif // SWARM_H
