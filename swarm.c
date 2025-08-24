#include "swarm.h"
#include "communication.h"

// Variables globales para configuración
extern int config_num_blancos;
extern int config_num_enjambres;
extern int config_probabilidad_defensa;
extern int config_probabilidad_comunicacion;
extern int config_timeout_reconexion;

/**
 * Crea un nuevo enjambre
 * @param id ID del enjambre
 * @param id_blanco ID del blanco asignado
 * @param zona_ensamblaje Coordenadas de la zona de ensamblaje
 * @param posicion_blanco Coordenadas del blanco
 * @return Puntero al enjambre creado o NULL si hay error
 */
Enjambre* crear_enjambre(int id, int id_blanco, Coordenada zona_ensamblaje, Coordenada posicion_blanco) {
    Enjambre* enjambre = malloc(sizeof(Enjambre));
    if (!enjambre) {
        log_mensaje("Error: No se pudo asignar memoria para el enjambre");
        return NULL;
    }
    
    // Inicializar estructura
    enjambre->id = id;
    enjambre->id_blanco = id_blanco;
    enjambre->estado = ENJAMBRE_INCOMPLETO;
    enjambre->num_drones = 0;
    enjambre->num_drones_activos = 0;
    enjambre->num_drones_ataque = 0;
    enjambre->num_drones_camara = 0;
    enjambre->zona_ensamblaje = zona_ensamblaje;
    enjambre->posicion_blanco = posicion_blanco;
    
    // Inicializar array de drones
    for (int i = 0; i < MAX_DRONES; i++) {
        enjambre->drones[i] = NULL;
    }
    
    // Inicializar mutex
    if (pthread_mutex_init(&enjambre->mutex_enjambre, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del enjambre");
        free(enjambre);
        return NULL;
    }
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d creado - Blanco: %d", id, id_blanco);
    log_mensaje(mensaje);
    
    return enjambre;
}

/**
 * Destruye un enjambre liberando recursos
 * @param enjambre Enjambre a destruir
 */
void destruir_enjambre(Enjambre* enjambre) {
    if (!enjambre) return;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    // Liberar drones
    for (int i = 0; i < MAX_DRONES; i++) {
        if (enjambre->drones[i]) {
            destruir_drone(enjambre->drones[i]);
            enjambre->drones[i] = NULL;
        }
    }
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    pthread_mutex_destroy(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d destruido", enjambre->id);
    log_mensaje(mensaje);
    
    free(enjambre);
}

/**
 * Inicializa un enjambre
 * @param enjambre Enjambre a inicializar
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int inicializar_enjambre(Enjambre* enjambre) {
    if (!enjambre) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    enjambre->estado = ENJAMBRE_INCOMPLETO;
    enjambre->num_drones_activos = 0;
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d inicializado", enjambre->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Agrega un drone a un enjambre
 * @param enjambre Enjambre al que agregar el drone
 * @param drone Drone a agregar
 * @return 0 si se agregó correctamente, -1 en caso de error
 */
int agregar_drone_enjambre(Enjambre* enjambre, Drone* drone) {
    if (!enjambre || !drone) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    // Buscar posición libre
    int posicion = -1;
    for (int i = 0; i < MAX_DRONES; i++) {
        if (enjambre->drones[i] == NULL) {
            posicion = i;
            break;
        }
    }
    
    if (posicion == -1) {
        pthread_mutex_unlock(&enjambre->mutex_enjambre);
        log_mensaje("Error: No hay espacio en el enjambre para más drones");
        return -1;
    }
    
    // Agregar drone
    enjambre->drones[posicion] = drone;
    enjambre->num_drones++;
    enjambre->num_drones_activos++;
    
    if (drone->tipo == DRONE_TIPO_ATAQUE) {
        enjambre->num_drones_ataque++;
    } else {
        enjambre->num_drones_camara++;
    }
    
    // Actualizar ID del enjambre en el drone
    drone->id_enjambre = enjambre->id;
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d agregado al enjambre %d", drone->id, enjambre->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Remueve un drone de un enjambre
 * @param enjambre Enjambre del que remover el drone
 * @param id_drone ID del drone a remover
 * @return 0 si se removió correctamente, -1 en caso de error
 */
int remover_drone_enjambre(Enjambre* enjambre, int id_drone) {
    if (!enjambre) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    // Buscar drone
    int posicion = -1;
    for (int i = 0; i < MAX_DRONES; i++) {
        if (enjambre->drones[i] && enjambre->drones[i]->id == id_drone) {
            posicion = i;
            break;
        }
    }
    
    if (posicion == -1) {
        pthread_mutex_unlock(&enjambre->mutex_enjambre);
        return -1;
    }
    
    Drone* drone = enjambre->drones[posicion];
    
    // Actualizar contadores
    if (drone->tipo == DRONE_TIPO_ATAQUE) {
        enjambre->num_drones_ataque--;
    } else {
        enjambre->num_drones_camara--;
    }
    
    enjambre->num_drones--;
    enjambre->num_drones_activos--;
    
    // Remover drone
    enjambre->drones[posicion] = NULL;
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d removido del enjambre %d", id_drone, enjambre->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Verifica si un enjambre está completo (5 drones: 4 ataque + 1 cámara)
 * @param enjambre Enjambre a verificar
 * @return 1 si está completo, 0 si no
 */
int verificar_ensamblaje_completo_enjambre(Enjambre* enjambre) {
    if (!enjambre) return 0;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    int completo = (enjambre->num_drones_ataque == 4 && enjambre->num_drones_camara == 1);
    
    if (completo && enjambre->estado == ENJAMBRE_INCOMPLETO) {
        enjambre->estado = ENJAMBRE_COMPLETO;
        char mensaje[MAX_STRING];
        snprintf(mensaje, sizeof(mensaje), "Enjambre %d completo - Listo para ataque", enjambre->id);
        log_mensaje(mensaje);
    }
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    return completo;
}

/**
 * Activa un enjambre para iniciar el ataque
 * @param enjambre Enjambre a activar
 * @return 0 si se activó correctamente, -1 en caso de error
 */
int activar_enjambre(Enjambre* enjambre) {
    if (!enjambre) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    if (enjambre->estado != ENJAMBRE_COMPLETO) {
        pthread_mutex_unlock(&enjambre->mutex_enjambre);
        log_mensaje("Error: No se puede activar enjambre incompleto");
        return -1;
    }
    
    enjambre->estado = ENJAMBRE_ATAQUE;
    
    // Activar todos los drones del enjambre
    for (int i = 0; i < MAX_DRONES; i++) {
        if (enjambre->drones[i]) {
            actualizar_estado_drone(enjambre->drones[i], DRONE_VOLANDO_BLANCO);
        }
    }
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d activado para ataque", enjambre->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Crea el control de enjambres
 * @return Puntero al control de enjambres o NULL si hay error
 */
ControlEnjambres* crear_control_enjambres(void) {
    ControlEnjambres* control = malloc(sizeof(ControlEnjambres));
    if (!control) {
        log_mensaje("Error: No se pudo asignar memoria para el control de enjambres");
        return NULL;
    }
    
    control->num_enjambres = 0;
    
    // Inicializar array de enjambres
    for (int i = 0; i < MAX_ENJAMBRES; i++) {
        control->enjambres[i] = NULL;
    }
    
    // Inicializar mutex
    if (pthread_mutex_init(&control->mutex_sistema, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del sistema de enjambres");
        free(control);
        return NULL;
    }
    
    log_mensaje("Control de enjambres creado exitosamente");
    return control;
}

/**
 * Destruye el control de enjambres
 * @param control Control a destruir
 */
void destruir_control_enjambres(ControlEnjambres* control) {
    if (!control) return;
    
    pthread_mutex_lock(&control->mutex_sistema);
    
    // Destruir todos los enjambres
    for (int i = 0; i < MAX_ENJAMBRES; i++) {
        if (control->enjambres[i]) {
            destruir_enjambre(control->enjambres[i]);
            control->enjambres[i] = NULL;
        }
    }
    
    pthread_mutex_unlock(&control->mutex_sistema);
    pthread_mutex_destroy(&control->mutex_sistema);
    
    log_mensaje("Control de enjambres destruido");
    free(control);
}

/**
 * Coordina el ataque de un enjambre
 * @param enjambre Enjambre que realiza el ataque
 * @return 0 si el ataque se coordina correctamente, -1 en caso de error
 */
int coordinar_ataque_enjambre(Enjambre* enjambre) {
    if (!enjambre) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    if (enjambre->estado != ENJAMBRE_ATAQUE) {
        pthread_mutex_unlock(&enjambre->mutex_enjambre);
        return -1;
    }
    
    int drones_atacantes = 0;
    
    // Contar drones de ataque activos
    for (int i = 0; i < MAX_DRONES; i++) {
        if (enjambre->drones[i] && enjambre->drones[i]->tipo == DRONE_TIPO_ATAQUE) {
            if (enjambre->drones[i]->estado != DRONE_DESTRUIDO) {
                drones_atacantes++;
            }
        }
    }
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d: %d drones atacando blanco %d", 
             enjambre->id, drones_atacantes, enjambre->id_blanco);
    log_mensaje(mensaje);
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    return 0;
}

/**
 * Obtiene el estado actual de un enjambre
 * @param enjambre Enjambre a consultar
 * @return Estado del enjambre
 */
EstadoEnjambre obtener_estado_enjambre(Enjambre* enjambre) {
    if (!enjambre) return ENJAMBRE_DESTRUIDO;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    EstadoEnjambre estado = enjambre->estado;
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    return estado;
}

/**
 * Actualiza el estado de un enjambre
 * @param enjambre Enjambre a actualizar
 * @param nuevo_estado Nuevo estado del enjambre
 * @return 0 si se actualizó correctamente, -1 en caso de error
 */
int actualizar_estado_enjambre(Enjambre* enjambre, EstadoEnjambre nuevo_estado) {
    if (!enjambre) return -1;
    
    pthread_mutex_lock(&enjambre->mutex_enjambre);
    
    EstadoEnjambre estado_anterior = enjambre->estado;
    enjambre->estado = nuevo_estado;
    
    pthread_mutex_unlock(&enjambre->mutex_enjambre);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Enjambre %d: Estado cambiado de %d a %d", 
             enjambre->id, estado_anterior, nuevo_estado);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Hilo de control principal del enjambre
 * @param arg Puntero al enjambre
 * @return NULL
 */
void* hilo_control_enjambre(void* arg) {
    Enjambre* enjambre = (Enjambre*)arg;
    if (!enjambre) return NULL;
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Hilo de control iniciado para enjambre %d", enjambre->id);
    log_mensaje(mensaje);
    
    while (enjambre->estado != ENJAMBRE_DESTRUIDO) {
        pthread_mutex_lock(&enjambre->mutex_enjambre);
        
        // Verificar estado de los drones
        int drones_activos = 0;
        for (int i = 0; i < MAX_DRONES; i++) {
            if (enjambre->drones[i] && enjambre->drones[i]->estado != DRONE_DESTRUIDO) {
                drones_activos++;
            }
        }
        
        enjambre->num_drones_activos = drones_activos;
        
        // Si no quedan drones activos, marcar enjambre como destruido
        if (drones_activos == 0 && enjambre->estado == ENJAMBRE_ATAQUE) {
            enjambre->estado = ENJAMBRE_DESTRUIDO;
            pthread_mutex_unlock(&enjambre->mutex_enjambre);
            break;
        }
        
        pthread_mutex_unlock(&enjambre->mutex_enjambre);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(1000);
    }
    
    snprintf(mensaje, sizeof(mensaje), "Hilo de control terminado para enjambre %d", enjambre->id);
    log_mensaje(mensaje);
    
    return NULL;
}

/**
 * Identifica enjambres incompletos en el sistema
 * @param control Control de enjambres
 * @param enjambres_incompletos Array para almacenar IDs de enjambres incompletos
 * @param num_incompletos Puntero al número de enjambres incompletos encontrados
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int identificar_enjambres_incompletos(ControlEnjambres* control, int* enjambres_incompletos, int* num_incompletos) {
    if (!control || !enjambres_incompletos || !num_incompletos) return -1;
    
    pthread_mutex_lock(&control->mutex_sistema);
    
    *num_incompletos = 0;
    
    for (int i = 0; i < control->num_enjambres; i++) {
        if (control->enjambres[i] && control->enjambres[i]->estado == ENJAMBRE_INCOMPLETO) {
            enjambres_incompletos[*num_incompletos] = i;
            (*num_incompletos)++;
        }
    }
    
    pthread_mutex_unlock(&control->mutex_sistema);
    
    return 0;
}

/**
 * Ejecuta la reconformación de enjambres incompletos
 * @param control Control de enjambres
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int ejecutar_reconformacion_enjambres(ControlEnjambres* control) {
    if (!control) return -1;
    
    int enjambres_incompletos[MAX_ENJAMBRES];
    int num_incompletos = 0;
    
    identificar_enjambres_incompletos(control, enjambres_incompletos, &num_incompletos);
    
    if (num_incompletos == 0) {
        return 0; // No hay enjambres incompletos
    }
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Iniciando reconformación de %d enjambres incompletos", num_incompletos);
    log_mensaje(mensaje);
    
    // Aquí se implementaría la lógica de reconformación alternada
    // Por simplicidad, registramos el evento
    for (int i = 0; i < num_incompletos; i++) {
        snprintf(mensaje, sizeof(mensaje), "Enjambre %d marcado para reconformación", enjambres_incompletos[i]);
        log_mensaje(mensaje);
    }
    
    return 0;
}