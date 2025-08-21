#include "communication.h"

/**
 * Crea un servidor de comunicación en el puerto especificado
 * @param puerto Puerto donde escuchará el servidor
 * @return Puntero al servidor creado o NULL si hay error
 */
ServidorComunicacion* crear_servidor(int puerto) {
    ServidorComunicacion* servidor = malloc(sizeof(ServidorComunicacion));
    if (!servidor) {
        log_mensaje("Error: No se pudo asignar memoria para el servidor");
        return NULL;
    }
    
    // Inicializar estructura
    servidor->puerto = puerto;
    servidor->activo = 0;
    servidor->socket_fd = -1;
    
    // Inicializar mutex
    if (pthread_mutex_init(&servidor->mutex_socket, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del servidor");
        free(servidor);
        return NULL;
    }
    
    return servidor;
}

/**
 * Inicia el servidor y comienza a escuchar conexiones
 * @param servidor Servidor a iniciar
 * @return 0 si se inició correctamente, -1 en caso de error
 */
int iniciar_servidor(ServidorComunicacion* servidor) {
    if (!servidor) return -1;
    
    pthread_mutex_lock(&servidor->mutex_socket);
    
    // Crear socket
    servidor->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor->socket_fd < 0) {
        log_mensaje("Error: No se pudo crear el socket del servidor");
        pthread_mutex_unlock(&servidor->mutex_socket);
        return -1;
    }
    
    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(servidor->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_mensaje("Error: No se pudo configurar el socket del servidor");
        close(servidor->socket_fd);
        pthread_mutex_unlock(&servidor->mutex_socket);
        return -1;
    }
    
    // Configurar dirección
    memset(&servidor->direccion, 0, sizeof(servidor->direccion));
    servidor->direccion.sin_family = AF_INET;
    servidor->direccion.sin_addr.s_addr = INADDR_ANY;
    servidor->direccion.sin_port = htons(servidor->puerto);
    
    // Vincular socket a la dirección
    if (bind(servidor->socket_fd, (struct sockaddr*)&servidor->direccion, sizeof(servidor->direccion)) < 0) {
        log_mensaje("Error: No se pudo vincular el socket del servidor");
        close(servidor->socket_fd);
        pthread_mutex_unlock(&servidor->mutex_socket);
        return -1;
    }
    
    // Escuchar conexiones
    if (listen(servidor->socket_fd, 10) < 0) {
        log_mensaje("Error: No se pudo iniciar la escucha del servidor");
        close(servidor->socket_fd);
        pthread_mutex_unlock(&servidor->mutex_socket);
        return -1;
    }
    
    servidor->activo = 1;
    log_mensaje("Servidor iniciado exitosamente");
    
    pthread_mutex_unlock(&servidor->mutex_socket);
    return 0;
}

/**
 * Cierra el servidor y libera recursos
 * @param servidor Servidor a cerrar
 */
void cerrar_servidor(ServidorComunicacion* servidor) {
    if (!servidor) return;
    
    pthread_mutex_lock(&servidor->mutex_socket);
    
    if (servidor->socket_fd >= 0) {
        close(servidor->socket_fd);
        servidor->socket_fd = -1;
    }
    
    servidor->activo = 0;
    
    pthread_mutex_unlock(&servidor->mutex_socket);
    pthread_mutex_destroy(&servidor->mutex_socket);
    
    free(servidor);
    log_mensaje("Servidor cerrado");
}

/**
 * Acepta una nueva conexión de cliente
 * @param servidor Servidor que acepta la conexión
 * @return Descriptor del socket del cliente o -1 si hay error
 */
int aceptar_conexion(ServidorComunicacion* servidor) {
    if (!servidor || !servidor->activo) return -1;
    
    struct sockaddr_in cliente_addr;
    socklen_t addr_len = sizeof(cliente_addr);
    
    int cliente_fd = accept(servidor->socket_fd, (struct sockaddr*)&cliente_addr, &addr_len);
    if (cliente_fd < 0) {
        log_mensaje("Error: No se pudo aceptar la conexión del cliente");
        return -1;
    }
    
    char ip_cliente[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cliente_addr.sin_addr, ip_cliente, INET_ADDRSTRLEN);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Nueva conexión aceptada desde %s:%d", ip_cliente, ntohs(cliente_addr.sin_port));
    log_mensaje(mensaje);
    
    return cliente_fd;
}

/**
 * Crea un cliente de comunicación
 * @param ip Dirección IP del servidor
 * @param puerto Puerto del servidor
 * @return Puntero al cliente creado o NULL si hay error
 */
ClienteComunicacion* crear_cliente(const char* ip, int puerto) {
    ClienteComunicacion* cliente = malloc(sizeof(ClienteComunicacion));
    if (!cliente) {
        log_mensaje("Error: No se pudo asignar memoria para el cliente");
        return NULL;
    }
    
    // Inicializar estructura
    cliente->conectado = 0;
    cliente->socket_fd = -1;
    
    // Configurar dirección del servidor
    memset(&cliente->direccion, 0, sizeof(cliente->direccion));
    cliente->direccion.sin_family = AF_INET;
    cliente->direccion.sin_port = htons(puerto);
    
    if (inet_pton(AF_INET, ip, &cliente->direccion.sin_addr) <= 0) {
        log_mensaje("Error: Dirección IP inválida");
        free(cliente);
        return NULL;
    }
    
    // Inicializar mutex
    if (pthread_mutex_init(&cliente->mutex_socket, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del cliente");
        free(cliente);
        return NULL;
    }
    
    return cliente;
}

/**
 * Conecta el cliente al servidor
 * @param cliente Cliente a conectar
 * @return 0 si se conectó correctamente, -1 en caso de error
 */
int conectar_cliente(ClienteComunicacion* cliente) {
    if (!cliente) return -1;
    
    pthread_mutex_lock(&cliente->mutex_socket);
    
    // Crear socket
    cliente->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cliente->socket_fd < 0) {
        log_mensaje("Error: No se pudo crear el socket del cliente");
        pthread_mutex_unlock(&cliente->mutex_socket);
        return -1;
    }
    
    // Conectar al servidor
    if (connect(cliente->socket_fd, (struct sockaddr*)&cliente->direccion, sizeof(cliente->direccion)) < 0) {
        log_mensaje("Error: No se pudo conectar al servidor");
        close(cliente->socket_fd);
        cliente->socket_fd = -1;
        pthread_mutex_unlock(&cliente->mutex_socket);
        return -1;
    }
    
    cliente->conectado = 1;
    log_mensaje("Cliente conectado exitosamente al servidor");
    
    pthread_mutex_unlock(&cliente->mutex_socket);
    return 0;
}

/**
 * Cierra la conexión del cliente
 * @param cliente Cliente a cerrar
 */
void cerrar_cliente(ClienteComunicacion* cliente) {
    if (!cliente) return;
    
    pthread_mutex_lock(&cliente->mutex_socket);
    
    if (cliente->socket_fd >= 0) {
        close(cliente->socket_fd);
        cliente->socket_fd = -1;
    }
    
    cliente->conectado = 0;
    
    pthread_mutex_unlock(&cliente->mutex_socket);
    pthread_mutex_destroy(&cliente->mutex_socket);
    
    free(cliente);
    log_mensaje("Cliente desconectado");
}

/**
 * Envía un mensaje desde el servidor a un cliente específico
 * @param servidor Servidor que envía el mensaje
 * @param cliente_fd Descriptor del socket del cliente
 * @param mensaje Mensaje a enviar
 * @return 0 si se envió correctamente, -1 en caso de error
 */
int enviar_mensaje_servidor(ServidorComunicacion* servidor, int cliente_fd, MensajeComunicacion* mensaje) {
    if (!servidor || !mensaje || cliente_fd < 0) return -1;
    
    char buffer[sizeof(MensajeComunicacion)];
    int bytes_serializados = serializar_mensaje(mensaje, buffer);
    
    if (bytes_serializados <= 0) {
        log_mensaje("Error: No se pudo serializar el mensaje");
        return -1;
    }
    
    int bytes_enviados = send(cliente_fd, buffer, bytes_serializados, 0);
    if (bytes_enviados != bytes_serializados) {
        log_mensaje("Error: No se pudo enviar el mensaje completo");
        return -1;
    }
    
    return 0;
}

/**
 * Envía un mensaje desde el cliente al servidor
 * @param cliente Cliente que envía el mensaje
 * @param mensaje Mensaje a enviar
 * @return 0 si se envió correctamente, -1 en caso de error
 */
int enviar_mensaje_cliente(ClienteComunicacion* cliente, MensajeComunicacion* mensaje) {
    if (!cliente || !mensaje || !cliente->conectado) return -1;
    
    pthread_mutex_lock(&cliente->mutex_socket);
    
    char buffer[sizeof(MensajeComunicacion)];
    int bytes_serializados = serializar_mensaje(mensaje, buffer);
    
    if (bytes_serializados <= 0) {
        log_mensaje("Error: No se pudo serializar el mensaje");
        pthread_mutex_unlock(&cliente->mutex_socket);
        return -1;
    }
    
    int bytes_enviados = send(cliente->socket_fd, buffer, bytes_serializados, 0);
    if (bytes_enviados != bytes_serializados) {
        log_mensaje("Error: No se pudo enviar el mensaje completo");
        pthread_mutex_unlock(&cliente->mutex_socket);
        return -1;
    }
    
    pthread_mutex_unlock(&cliente->mutex_socket);
    return 0;
}

/**
 * Recibe un mensaje en el cliente desde el servidor
 * @param cliente Cliente que recibe el mensaje
 * @param mensaje Buffer donde se almacenará el mensaje recibido
 * @return 0 si se recibió correctamente, -1 en caso de error
 */
int recibir_mensaje_cliente(ClienteComunicacion* cliente, MensajeComunicacion* mensaje) {
    if (!cliente || !mensaje || !cliente->conectado) return -1;
    
    pthread_mutex_lock(&cliente->mutex_socket);
    
    char buffer[sizeof(MensajeComunicacion)];
    int bytes_recibidos = recv(cliente->socket_fd, buffer, sizeof(buffer), 0);
    
    if (bytes_recibidos <= 0) {
        log_mensaje("Error: No se pudo recibir el mensaje");
        pthread_mutex_unlock(&cliente->mutex_socket);
        return -1;
    }
    
    int resultado = deserializar_mensaje(buffer, mensaje);
    
    pthread_mutex_unlock(&cliente->mutex_socket);
    return resultado;
}

/**
 * Inicializa un mensaje de comunicación con valores por defecto
 * @param mensaje Mensaje a inicializar
 * @param tipo Tipo de mensaje
 * @param id_drone ID del drone
 * @param id_enjambre ID del enjambre
 */
void inicializar_mensaje(MensajeComunicacion* mensaje, int tipo, int id_drone, int id_enjambre) {
    if (!mensaje) return;
    
    mensaje->tipo_mensaje = tipo;
    mensaje->id_drone = id_drone;
    mensaje->id_enjambre = id_enjambre;
    mensaje->estado = DRONE_ESPERANDO;
    mensaje->posicion.x = 0;
    mensaje->posicion.y = 0;
    mensaje->combustible = 100;
    memset(mensaje->datos_adicionales, 0, sizeof(mensaje->datos_adicionales));
}

/**
 * Serializa un mensaje de comunicación a un buffer de bytes
 * @param mensaje Mensaje a serializar
 * @param buffer Buffer donde se almacenará el mensaje serializado
 * @return Número de bytes serializados o -1 si hay error
 */
int serializar_mensaje(MensajeComunicacion* mensaje, char* buffer) {
    if (!mensaje || !buffer) return -1;
    
    int offset = 0;
    
    // Serializar campos básicos
    memcpy(buffer + offset, &mensaje->tipo_mensaje, sizeof(int));
    offset += sizeof(int);
    
    memcpy(buffer + offset, &mensaje->id_drone, sizeof(int));
    offset += sizeof(int);
    
    memcpy(buffer + offset, &mensaje->id_enjambre, sizeof(int));
    offset += sizeof(int);
    
    memcpy(buffer + offset, &mensaje->estado, sizeof(EstadoDrone));
    offset += sizeof(EstadoDrone);
    
    memcpy(buffer + offset, &mensaje->posicion, sizeof(Coordenada));
    offset += sizeof(Coordenada);
    
    memcpy(buffer + offset, &mensaje->combustible, sizeof(int));
    offset += sizeof(int);
    
    memcpy(buffer + offset, mensaje->datos_adicionales, sizeof(mensaje->datos_adicionales));
    offset += sizeof(mensaje->datos_adicionales);
    
    return offset;
}

/**
 * Deserializa un buffer de bytes a un mensaje de comunicación
 * @param buffer Buffer con los datos serializados
 * @param mensaje Mensaje donde se almacenarán los datos deserializados
 * @return 0 si se deserializó correctamente, -1 en caso de error
 */
int deserializar_mensaje(char* buffer, MensajeComunicacion* mensaje) {
    if (!buffer || !mensaje) return -1;
    
    int offset = 0;
    
    // Deserializar campos básicos
    memcpy(&mensaje->tipo_mensaje, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    memcpy(&mensaje->id_drone, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    memcpy(&mensaje->id_enjambre, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    memcpy(&mensaje->estado, buffer + offset, sizeof(EstadoDrone));
    offset += sizeof(EstadoDrone);
    
    memcpy(&mensaje->posicion, buffer + offset, sizeof(Coordenada));
    offset += sizeof(Coordenada);
    
    memcpy(&mensaje->combustible, buffer + offset, sizeof(int));
    offset += sizeof(int);
    
    memcpy(mensaje->datos_adicionales, buffer + offset, sizeof(mensaje->datos_adicionales));
    offset += sizeof(mensaje->datos_adicionales);
    
    return 0;
}

/**
 * Imprime la información de un mensaje de comunicación
 * @param mensaje Mensaje a imprimir
 */
void imprimir_mensaje(MensajeComunicacion* mensaje) {
    if (!mensaje) return;
    
    printf("Mensaje: Tipo=%d, Drone=%d, Enjambre=%d, Estado=%d, Pos=(%d,%d), Combustible=%d\n",
           mensaje->tipo_mensaje, mensaje->id_drone, mensaje->id_enjambre, 
           mensaje->estado, mensaje->posicion.x, mensaje->posicion.y, mensaje->combustible);
}
