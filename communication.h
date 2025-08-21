#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "common.h"

// Estructura para el servidor de comunicación
typedef struct {
    int socket_fd;
    int puerto;
    struct sockaddr_in direccion;
    int activo;
    pthread_mutex_t mutex_socket;
} ServidorComunicacion;

// Estructura para el cliente de comunicación
typedef struct {
    int socket_fd;
    struct sockaddr_in direccion;
    int conectado;
    pthread_mutex_t mutex_socket;
} ClienteComunicacion;

// Funciones del Servidor
ServidorComunicacion* crear_servidor(int puerto);
int iniciar_servidor(ServidorComunicacion* servidor);
void cerrar_servidor(ServidorComunicacion* servidor);
int aceptar_conexion(ServidorComunicacion* servidor);
int enviar_mensaje_servidor(ServidorComunicacion* servidor, int cliente_fd, MensajeComunicacion* mensaje);

// Funciones del Cliente
ClienteComunicacion* crear_cliente(const char* ip, int puerto);
int conectar_cliente(ClienteComunicacion* cliente);
void cerrar_cliente(ClienteComunicacion* cliente);
int enviar_mensaje_cliente(ClienteComunicacion* cliente, MensajeComunicacion* mensaje);
int recibir_mensaje_cliente(ClienteComunicacion* cliente, MensajeComunicacion* mensaje);

// Funciones de Utilidad de Comunicación
void inicializar_mensaje(MensajeComunicacion* mensaje, int tipo, int id_drone, int id_enjambre);
int serializar_mensaje(MensajeComunicacion* mensaje, char* buffer);
int deserializar_mensaje(char* buffer, MensajeComunicacion* mensaje);
void imprimir_mensaje(MensajeComunicacion* mensaje);

// Hilos de Comunicación
void* hilo_servidor_centro_comando(void* arg);
void* hilo_cliente_drone(void* arg);
void* hilo_heartbeat(void* arg);

#endif // COMMUNICATION_H
