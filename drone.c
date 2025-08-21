#include "drone.h"

/**
 * Crea un nuevo drone con los parámetros especificados
 * @param id Identificador único del drone
 * @param id_enjambre ID del enjambre al que pertenece
 * @param tipo Tipo de drone (ataque o cámara)
 * @param posicion_inicial Posición inicial del drone
 * @return Puntero al drone creado o NULL si hay error
 */
Drone* crear_drone(int id, int id_enjambre, TipoDrone tipo, Coordenada posicion_inicial) {
    Drone* drone = malloc(sizeof(Drone));
    if (!drone) {
        log_mensaje("Error: No se pudo asignar memoria para el drone");
        return NULL;
    }
    
    // Inicializar estructura del drone
    drone->id = id;
    drone->id_enjambre = id_enjambre;
    drone->tipo = tipo;
    drone->estado = DRONE_ESPERANDO;
    drone->posicion = posicion_inicial;
    drone->posicion_objetivo = posicion_inicial;
    drone->combustible = 100;
    drone->combustible_inicial = 100;
    drone->distancia_recorrida = 0;
    drone->armas_activadas = 0;
    drone->camara_activada = 0;
    drone->comunicacion_activa = 1;
    drone->ultimo_heartbeat = time(NULL);
    
    // Inicializar mutex del drone
    if (pthread_mutex_init(&drone->mutex_drone, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del drone");
        free(drone);
        return NULL;
    }
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d creado - Tipo: %s, Enjambre: %d", 
             id, (tipo == DRONE_TIPO_ATAQUE) ? "Ataque" : "Cámara", id_enjambre);
    log_mensaje(mensaje);
    
    return drone;
}

/**
 * Destruye un drone y libera todos sus recursos
 * @param drone Drone a destruir
 */
void destruir_drone(Drone* drone) {
    if (!drone) return;
    
    // Destruir mutex
    pthread_mutex_destroy(&drone->mutex_drone);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d destruido", drone->id);
    log_mensaje(mensaje);
    
    free(drone);
}

/**
 * Inicializa un drone y activa todos sus componentes
 * @param drone Drone a inicializar
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int inicializar_drone(Drone* drone) {
    if (!drone) return -1;
    
    pthread_mutex_lock(&drone->mutex_drone);
    
    // Verificar que el drone esté en estado de espera
    if (drone->estado != DRONE_ESPERANDO) {
        log_mensaje("Error: El drone no está en estado de espera");
        pthread_mutex_unlock(&drone->mutex_drone);
        return -1;
    }
    
    // Activar comunicación
    drone->comunicacion_activa = 1;
    drone->ultimo_heartbeat = time(NULL);
    
    pthread_mutex_unlock(&drone->mutex_drone);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d inicializado correctamente", drone->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Crea el control de navegación para un drone
 * @param drone Drone al que pertenece el control
 * @return Puntero al control de navegación o NULL si hay error
 */
ControlNavegacion* crear_control_navegacion(Drone* drone) {
    if (!drone) return NULL;
    
    ControlNavegacion* control = malloc(sizeof(ControlNavegacion));
    if (!control) {
        log_mensaje("Error: No se pudo asignar memoria para el control de navegación");
        return NULL;
    }
    
    // Inicializar estructura base
    control->base.id_componente = 1;
    control->base.activo = 1;
    control->base.drone = drone;
    
    if (pthread_mutex_init(&control->base.mutex, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del control de navegación");
        free(control);
        return NULL;
    }
    
    // Inicializar parámetros específicos
    control->posicion_actual = drone->posicion;
    control->posicion_objetivo = drone->posicion;
    control->velocidad = 1;
    control->direccion_actual = 0;
    
    return control;
}

/**
 * Crea el control de combustible para un drone
 * @param drone Drone al que pertenece el control
 * @return Puntero al control de combustible o NULL si hay error
 */
ControlCombustible* crear_control_combustible(Drone* drone) {
    if (!drone) return NULL;
    
    ControlCombustible* control = malloc(sizeof(ControlCombustible));
    if (!control) {
        log_mensaje("Error: No se pudo asignar memoria para el control de combustible");
        return NULL;
    }
    
    // Inicializar estructura base
    control->base.id_componente = 2;
    control->base.activo = 1;
    control->base.drone = drone;
    
    if (pthread_mutex_init(&control->base.mutex, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del control de combustible");
        free(control);
        return NULL;
    }
    
    // Inicializar parámetros específicos
    control->combustible_restante = drone->combustible;
    control->consumo_por_unidad = 1;
    control->nivel_critico = 10;
    
    return control;
}

/**
 * Crea el control de armas para un drone
 * @param drone Drone al que pertenece el control
 * @return Puntero al control de armas o NULL si hay error
 */
ControlArmas* crear_control_armas(Drone* drone) {
    if (!drone) return NULL;
    
    ControlArmas* control = malloc(sizeof(ControlArmas));
    if (!control) {
        log_mensaje("Error: No se pudo asignar memoria para el control de armas");
        return NULL;
    }
    
    // Inicializar estructura base
    control->base.id_componente = 3;
    control->base.activo = 1;
    control->base.drone = drone;
    
    if (pthread_mutex_init(&control->base.mutex, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del control de armas");
        free(control);
        return NULL;
    }
    
    // Inicializar parámetros específicos
    control->armas_activadas = 0;
    control->municion_disponible = 1;
    control->objetivo_bloqueado = 0;
    
    return control;
}

/**
 * Crea el control de grabación para un drone
 * @param drone Drone al que pertenece el control
 * @return Puntero al control de grabación o NULL si hay error
 */
ControlGrabacion* crear_control_grabacion(Drone* drone) {
    if (!drone) return NULL;
    
    ControlGrabacion* control = malloc(sizeof(ControlGrabacion));
    if (!control) {
        log_mensaje("Error: No se pudo asignar memoria para el control de grabación");
        return NULL;
    }
    
    // Inicializar estructura base
    control->base.id_componente = 4;
    control->base.activo = 1;
    control->base.drone = drone;
    
    if (pthread_mutex_init(&control->base.mutex, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del control de grabación");
        free(control);
        return NULL;
    }
    
    // Inicializar parámetros específicos
    control->camara_activada = 0;
    control->calidad_grabacion = 100;
    control->almacenamiento_disponible = 1000;
    
    return control;
}

/**
 * Hace despegar un drone
 * @param drone Drone a hacer despegar
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int despegar_drone(Drone* drone) {
    if (!drone) return -1;
    
    pthread_mutex_lock(&drone->mutex_drone);
    
    if (drone->estado != DRONE_ESPERANDO) {
        log_mensaje("Error: El drone no puede despegar en su estado actual");
        pthread_mutex_unlock(&drone->mutex_drone);
        return -1;
    }
    
    if (drone->combustible <= 0) {
        log_mensaje("Error: El drone no tiene combustible para despegar");
        pthread_mutex_unlock(&drone->mutex_drone);
        return -1;
    }
    
    drone->estado = DRONE_VOLANDO_ENSAMBLAJE;
    
    pthread_mutex_unlock(&drone->mutex_drone);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d despegando hacia zona de ensamblaje", drone->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Hace aterrizar un drone
 * @param drone Drone a hacer aterrizar
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int aterrizar_drone(Drone* drone) {
    if (!drone) return -1;
    
    pthread_mutex_lock(&drone->mutex_drone);
    
    drone->estado = DRONE_ESPERANDO;
    
    pthread_mutex_unlock(&drone->mutex_drone);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d aterrizando", drone->id);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Mueve un drone a una nueva posición
 * @param drone Drone a mover
 * @param nueva_posicion Nueva posición objetivo
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int mover_drone(Drone* drone, Coordenada nueva_posicion) {
    if (!drone) return -1;
    
    pthread_mutex_lock(&drone->mutex_drone);
    
    // Verificar que el drone esté volando
    if (drone->estado != DRONE_VOLANDO_ENSAMBLAJE && 
        drone->estado != DRONE_VOLANDO_BLANCO) {
        log_mensaje("Error: El drone no está volando");
        pthread_mutex_unlock(&drone->mutex_drone);
        return -1;
    }
    
    // Calcular distancia a recorrer
    int distancia = calcular_distancia(drone->posicion, nueva_posicion);
    
    // Verificar combustible
    if (drone->combustible < distancia) {
        log_mensaje("Error: El drone no tiene suficiente combustible");
        pthread_mutex_unlock(&drone->mutex_drone);
        return -1;
    }
    
    // Actualizar posición y combustible
    drone->posicion = nueva_posicion;
    drone->combustible -= distancia;
    drone->distancia_recorrida += distancia;
    
    pthread_mutex_unlock(&drone->mutex_drone);
    
    return 0;
}

/**
 * Hilo principal del control de navegación
 * @param arg Puntero al control de navegación
 * @return NULL
 */
void* hilo_control_navegacion(void* arg) {
    ControlNavegacion* control = (ControlNavegacion*)arg;
    if (!control) return NULL;
    
    Drone* drone = control->base.drone;
    
    while (control->base.activo && drone->estado != DRONE_DESTRUIDO) {
        pthread_mutex_lock(&control->base.mutex);
        
        // Verificar si hay un objetivo de navegación
        if (control->posicion_objetivo.x != control->posicion_actual.x ||
            control->posicion_objetivo.y != control->posicion_actual.y) {
            
            // Calcular siguiente paso hacia el objetivo
            Coordenada siguiente_paso = mover_hacia_objetivo(control->posicion_actual, 
                                                           control->posicion_objetivo, 
                                                           control->velocidad);
            
            // Mover el drone
            if (mover_drone(drone, siguiente_paso) == 0) {
                control->posicion_actual = siguiente_paso;
                
                // Verificar si llegamos al objetivo
                if (calcular_distancia(control->posicion_actual, control->posicion_objetivo) == 0) {
                    char mensaje[MAX_STRING];
                    snprintf(mensaje, sizeof(mensaje), "Drone %d llegó a su objetivo", drone->id);
                    log_mensaje(mensaje);
                }
            }
        }
        
        pthread_mutex_unlock(&control->base.mutex);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(100);
    }
    
    return NULL;
}

/**
 * Hilo principal del control de combustible
 * @param arg Puntero al control de combustible
 * @return NULL
 */
void* hilo_control_combustible(void* arg) {
    ControlCombustible* control = (ControlCombustible*)arg;
    if (!control) return NULL;
    
    Drone* drone = control->base.drone;
    
    while (control->base.activo && drone->estado != DRONE_DESTRUIDO) {
        pthread_mutex_lock(&control->base.mutex);
        
        // Actualizar combustible restante
        control->combustible_restante = drone->combustible;
        
        // Verificar nivel crítico
        if (control->combustible_restante <= control->nivel_critico) {
            char mensaje[MAX_STRING];
            snprintf(mensaje, sizeof(mensaje), "ALERTA: Drone %d con combustible crítico (%d%%)", 
                     drone->id, control->combustible_restante);
            log_mensaje(mensaje);
        }
        
        // Verificar si se quedó sin combustible
        if (control->combustible_restante <= 0) {
            drone->estado = DRONE_SIN_COMBUSTIBLE;
            log_mensaje("Drone se quedó sin combustible - Autodestrucción inminente");
        }
        
        pthread_mutex_unlock(&control->base.mutex);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(500);
    }
    
    return NULL;
}

/**
 * Hilo principal del control de armas
 * @param arg Puntero al control de armas
 * @return NULL
 */
void* hilo_control_armas(void* arg) {
    ControlArmas* control = (ControlArmas*)arg;
    if (!control) return NULL;
    
    Drone* drone = control->base.drone;
    
    while (control->base.activo && drone->estado != DRONE_DESTRUIDO) {
        pthread_mutex_lock(&control->base.mutex);
        
        // Verificar si las armas están activadas
        if (control->armas_activadas && drone->estado == DRONE_ATAQUE) {
            // Ejecutar secuencia de ataque
            if (ejecutar_ataque_drone(drone) == 0) {
                control->municion_disponible--;
                if (control->municion_disponible <= 0) {
                    control->armas_activadas = 0;
                    log_mensaje("Drone ha agotado su munición");
                }
            }
        }
        
        pthread_mutex_unlock(&control->base.mutex);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(1000);
    }
    
    return NULL;
}

/**
 * Hilo principal del control de grabación
 * @param arg Puntero al control de grabación
 * @return NULL
 */
void* hilo_control_grabacion(void* arg) {
    ControlGrabacion* control = (ControlGrabacion*)arg;
    if (!control) return NULL;
    
    Drone* drone = control->base.drone;
    
    while (control->base.activo && drone->estado != DRONE_DESTRUIDO) {
        pthread_mutex_lock(&control->base.mutex);
        
        // Verificar si la cámara está activada
        if (control->camara_activada && drone->estado == DRONE_REPORTE) {
            // Ejecutar secuencia de reporte
            if (ejecutar_reporte_drone(drone) == 0) {
                log_mensaje("Drone cámara completó su reporte - Autodestrucción");
                drone->estado = DRONE_DESTRUIDO;
            }
        }
        
        pthread_mutex_unlock(&control->base.mutex);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(1000);
    }
    
    return NULL;
}

/**
 * Obtiene el estado actual de un drone
 * @param drone Drone del cual obtener el estado
 * @return Estado actual del drone
 */
EstadoDrone obtener_estado_drone(Drone* drone) {
    if (!drone) return DRONE_DESTRUIDO;
    
    EstadoDrone estado;
    pthread_mutex_lock(&drone->mutex_drone);
    estado = drone->estado;
    pthread_mutex_unlock(&drone->mutex_drone);
    
    return estado;
}

/**
 * Actualiza el estado de un drone
 * @param drone Drone al cual actualizar el estado
 * @param nuevo_estado Nuevo estado a asignar
 * @return 0 si se actualizó correctamente, -1 en caso de error
 */
int actualizar_estado_drone(Drone* drone, EstadoDrone nuevo_estado) {
    if (!drone) return -1;
    
    pthread_mutex_lock(&drone->mutex_drone);
    
    EstadoDrone estado_anterior = drone->estado;
    drone->estado = nuevo_estado;
    
    pthread_mutex_unlock(&drone->mutex_drone);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d cambió de estado %d a %d", 
             drone->id, estado_anterior, nuevo_estado);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Verifica el nivel de combustible de un drone
 * @param drone Drone a verificar
 * @return 1 si tiene combustible suficiente, 0 en caso contrario
 */
int verificar_combustible_drone(Drone* drone) {
    if (!drone) return 0;
    
    int tiene_combustible;
    pthread_mutex_lock(&drone->mutex_drone);
    tiene_combustible = (drone->combustible > 0);
    pthread_mutex_unlock(&drone->mutex_drone);
    
    return tiene_combustible;
}

/**
 * Verifica si un drone tiene comunicación activa
 * @param drone Drone a verificar
 * @return 1 si tiene comunicación, 0 en caso contrario
 */
int verificar_comunicacion_drone(Drone* drone) {
    if (!drone) return 0;
    
    int comunicacion_activa;
    pthread_mutex_lock(&drone->mutex_drone);
    comunicacion_activa = drone->comunicacion_activa;
    pthread_mutex_unlock(&drone->mutex_drone);
    
    return comunicacion_activa;
}

/**
 * Ejecuta un ataque con el drone
 * @param drone Drone que ejecuta el ataque
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int ejecutar_ataque_drone(Drone* drone) {
    if (!drone || drone->tipo != DRONE_TIPO_ATAQUE) return -1;
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d ejecutando ataque al blanco", drone->id);
    log_mensaje(mensaje);
    
    // Simular tiempo de ataque
    sleep_ms(500);
    
    // Marcar armas como utilizadas
    pthread_mutex_lock(&drone->mutex_drone);
    drone->armas_activadas = 0;
    drone->estado = DRONE_DESTRUIDO;
    pthread_mutex_unlock(&drone->mutex_drone);
    
    log_mensaje("Drone de ataque completó su misión - Autodestrucción");
    
    return 0;
}

/**
 * Ejecuta un reporte con el drone cámara
 * @param drone Drone que ejecuta el reporte
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int ejecutar_reporte_drone(Drone* drone) {
    if (!drone || drone->tipo != DRONE_TIPO_CAMARA) return -1;
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Drone %d ejecutando reporte del blanco", drone->id);
    log_mensaje(mensaje);
    
    // Simular tiempo de reporte
    sleep_ms(300);
    
    // Marcar cámara como utilizada
    pthread_mutex_lock(&drone->mutex_drone);
    drone->camara_activada = 0;
    pthread_mutex_unlock(&drone->mutex_drone);
    
    log_mensaje("Drone cámara completó su reporte");
    
    return 0;
}
