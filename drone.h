#ifndef DRONE_H
#define DRONE_H

#include "common.h"
#include "communication.h"

// Estructura para los componentes del drone
typedef struct {
    int id_componente;
    int activo;
    pthread_t hilo;
    Drone* drone;
    pthread_mutex_t mutex;
} ComponenteDrone;

// Estructura para el control de navegación
typedef struct {
    ComponenteDrone base;
    Coordenada posicion_actual;
    Coordenada posicion_objetivo;
    int velocidad;
    int direccion_actual;
} ControlNavegacion;

// Estructura para el control de combustible
typedef struct {
    ComponenteDrone base;
    int combustible_restante;
    int consumo_por_unidad;
    int nivel_critico;
} ControlCombustible;

// Estructura para el control de armas
typedef struct {
    ComponenteDrone base;
    int armas_activadas;
    int municion_disponible;
    int objetivo_bloqueado;
} ControlArmas;

// Estructura para el control de grabación
typedef struct {
    ComponenteDrone base;
    int camara_activada;
    int calidad_grabacion;
    int almacenamiento_disponible;
} ControlGrabacion;

// Funciones de Creación de Drones
Drone* crear_drone(int id, int id_enjambre, TipoDrone tipo, Coordenada posicion_inicial);
void destruir_drone(Drone* drone);
int inicializar_drone(Drone* drone);

// Funciones de Control de Componentes
ControlNavegacion* crear_control_navegacion(Drone* drone);
ControlCombustible* crear_control_combustible(Drone* drone);
ControlArmas* crear_control_armas(Drone* drone);
ControlGrabacion* crear_control_grabacion(Drone* drone);

// Funciones de Operación del Drone
int despegar_drone(Drone* drone);
int aterrizar_drone(Drone* drone);
int mover_drone(Drone* drone, Coordenada nueva_posicion);
int activar_armas_drone(Drone* drone);
int activar_camara_drone(Drone* drone);

// Hilos de Componentes
void* hilo_control_navegacion(void* arg);
void* hilo_control_combustible(void* arg);
void* hilo_control_armas(void* arg);
void* hilo_control_grabacion(void* arg);

// Funciones de Estado del Drone
EstadoDrone obtener_estado_drone(Drone* drone);
int actualizar_estado_drone(Drone* drone, EstadoDrone nuevo_estado);
int verificar_combustible_drone(Drone* drone);
int verificar_comunicacion_drone(Drone* drone);

// Funciones de Comunicación del Drone
int enviar_estado_drone(Drone* drone, ClienteComunicacion* cliente);
int procesar_comando_drone(Drone* drone, MensajeComunicacion* mensaje);
int enviar_heartbeat_drone(Drone* drone, ClienteComunicacion* cliente);

// Funciones de Lógica de Vuelo
int calcular_ruta_drone(Drone* drone, Coordenada destino);
int verificar_zona_defensa(Drone* drone, Coordenada posicion);
int verificar_ensamblaje_completo(Drone* drone);
int ejecutar_ataque_drone(Drone* drone);
int ejecutar_reporte_drone(Drone* drone);

#endif // DRONE_H
