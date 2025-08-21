#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>

// Constantes del Sistema
#define MAX_DRONES 100
#define MAX_ENJAMBRES 50
#define MAX_BLANCOS 50
#define MAX_STRING 256
#define MAX_COORDENADAS 1000

// Estados del Drone
typedef enum {
    DRONE_ESPERANDO = 0,
    DRONE_VOLANDO_ENSAMBLAJE,
    DRONE_ENSAMBLANDO,
    DRONE_VOLANDO_BLANCO,
    DRONE_ATAQUE,
    DRONE_REPORTE,
    DRONE_DESTRUIDO,
    DRONE_SIN_COMBUSTIBLE
} EstadoDrone;

// Tipos de Drone
typedef enum {
    DRONE_TIPO_ATAQUE = 0,
    DRONE_TIPO_CAMARA
} TipoDrone;

// Estados del Enjambre
typedef enum {
    ENJAMBRE_INCOMPLETO = 0,
    ENJAMBRE_COMPLETO,
    ENJAMBRE_ATAQUE,
    ENJAMBRE_DESTRUIDO
} EstadoEnjambre;

// Estados del Blanco
typedef enum {
    BLANCO_INTACTO = 0,
    BLANCO_PARCIALMENTE_DESTRUIDO,
    BLANCO_TOTALMENTE_DESTRUIDO
} EstadoBlanco;

// Estructura de Coordenadas
typedef struct {
    int x;
    int y;
} Coordenada;

// Estructura de Mensaje de Comunicación
typedef struct {
    int tipo_mensaje;
    int id_drone;
    int id_enjambre;
    EstadoDrone estado;
    Coordenada posicion;
    int combustible;
    char datos_adicionales[MAX_STRING];
} MensajeComunicacion;

// Estructura del Drone
typedef struct {
    int id;
    int id_enjambre;
    TipoDrone tipo;
    EstadoDrone estado;
    Coordenada posicion;
    Coordenada posicion_objetivo;
    int combustible;
    int combustible_inicial;
    int distancia_recorrida;
    int armas_activadas;
    int camara_activada;
    int comunicacion_activa;
    time_t ultimo_heartbeat;
    pthread_mutex_t mutex_drone;
} Drone;

// Estructura del Enjambre
typedef struct {
    int id;
    int id_blanco;
    EstadoEnjambre estado;
    int num_drones;
    int num_drones_ataque;
    int num_drones_camara;
    Drone* drones[MAX_DRONES];
    Coordenada zona_ensamblaje;
    Coordenada posicion_blanco;
    pthread_mutex_t mutex_enjambre;
} Enjambre;

// Estructura del Blanco
typedef struct {
    int id;
    Coordenada posicion;
    EstadoBlanco estado;
    int enjambres_asignados;
    pthread_mutex_t mutex_blanco;
} Blanco;

// Estructura del Camión
typedef struct {
    int id;
    Coordenada posicion;
    int num_drones;
    Drone* drones[MAX_DRONES];
    pthread_mutex_t mutex_camion;
} Camion;

// Estructura del Centro de Comando
typedef struct {
    int num_enjambres;
    int num_camiones;
    int num_blancos;
    Enjambre* enjambres[MAX_ENJAMBRES];
    Camion* camiones[MAX_DRONES];
    Blanco* blancos[MAX_BLANCOS];
    pthread_mutex_t mutex_sistema;
    int sistema_activo;
} CentroComando;

// Tipos de Mensajes
#define MSG_DESPEGAR 1
#define MSG_ENSAMBLAJE_COMPLETO 2
#define MSG_REASIGNACION 3
#define MSG_ESTADO_DRONE 4
#define MSG_ATAQUE 5
#define MSG_REPORTE 6
#define MSG_HEARTBEAT 7
#define MSG_DESTRUCCION 8

// Funciones de Utilidad
int calcular_distancia(Coordenada c1, Coordenada c2);
int generar_probabilidad(int porcentaje);
void sleep_ms(int milliseconds);
void log_mensaje(const char* mensaje);
void limpiar_pantalla(void);

// Funciones de Configuración
int cargar_configuracion(const char* archivo);
extern int config_num_blancos;
extern int config_num_enjambres;
extern int config_probabilidad_defensa;
extern int config_probabilidad_comunicacion;
extern int config_timeout_reconexion;

#endif // COMMON_H
