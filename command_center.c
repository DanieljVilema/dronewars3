#include "command_center.h"
#include "swarm.h"

// Variables globales para configuración
extern int config_num_blancos;
extern int config_num_enjambres;
extern int config_probabilidad_defensa;
extern int config_probabilidad_comunicacion;
extern int config_timeout_reconexion;

/**
 * Crea un nuevo centro de comando
 * @return Puntero al centro de comando o NULL si hay error
 */
CentroComando* crear_centro_comando(void) {
    CentroComando* centro = malloc(sizeof(CentroComando));
    if (!centro) {
        log_mensaje("Error: No se pudo asignar memoria para el centro de comando");
        return NULL;
    }
    
    // Inicializar estructura
    centro->id = 1;
    centro->activo = 1;
    centro->num_enjambres = 0;
    centro->num_camiones = 0;
    centro->num_blancos = 0;
    centro->sistema_activo = 1;
    
    // Inicializar arrays
    for (int i = 0; i < MAX_ENJAMBRES; i++) {
        centro->enjambres[i] = NULL;
    }
    for (int i = 0; i < MAX_DRONES; i++) {
        centro->camiones[i] = NULL;
    }
    for (int i = 0; i < MAX_BLANCOS; i++) {
        centro->blancos[i] = NULL;
    }
    
    // Inicializar mutex
    if (pthread_mutex_init(&centro->mutex_sistema, NULL) != 0) {
        log_mensaje("Error: No se pudo inicializar mutex del centro de comando");
        free(centro);
        return NULL;
    }
    
    log_mensaje("Centro de comando creado exitosamente");
    return centro;
}

/**
 * Destruye el centro de comando liberando recursos
 * @param centro Centro de comando a destruir
 */
void destruir_centro_comando(CentroComando* centro) {
    if (!centro) return;
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    // Destruir enjambres
    for (int i = 0; i < MAX_ENJAMBRES; i++) {
        if (centro->enjambres[i]) {
            destruir_enjambre(centro->enjambres[i]);
            centro->enjambres[i] = NULL;
        }
    }
    
    // Destruir camiones
    for (int i = 0; i < MAX_DRONES; i++) {
        if (centro->camiones[i]) {
            free(centro->camiones[i]);
            centro->camiones[i] = NULL;
        }
    }
    
    // Destruir blancos
    for (int i = 0; i < MAX_BLANCOS; i++) {
        if (centro->blancos[i]) {
            free(centro->blancos[i]);
            centro->blancos[i] = NULL;
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    pthread_mutex_destroy(&centro->mutex_sistema);
    
    log_mensaje("Centro de comando destruido");
    free(centro);
}

/**
 * Inicializa el centro de comando
 * @param centro Centro de comando a inicializar
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int inicializar_centro_comando(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Inicializando centro de comando...");
    
    // Cargar configuración
    if (cargar_configuracion("config.txt") != 0) {
        log_mensaje("Advertencia: No se pudo cargar configuración, usando valores por defecto");
        config_num_blancos = 10;
        config_num_enjambres = 10;
        config_probabilidad_defensa = 15;
        config_probabilidad_comunicacion = 5;
        config_timeout_reconexion = 30;
    }
    
    pthread_mutex_lock(&centro->mutex_sistema);
    centro->sistema_activo = 1;
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    log_mensaje("Centro de comando inicializado exitosamente");
    return 0;
}

/**
 * Crea los enjambres del sistema
 * @param centro Centro de comando
 * @return 0 si se crearon correctamente, -1 en caso de error
 */
int crear_enjambres_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Creando enjambres del sistema...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    for (int i = 0; i < config_num_enjambres && i < MAX_ENJAMBRES; i++) {
        Coordenada zona_ensamblaje = {100 + i * 50, 100 + i * 30};
        Coordenada posicion_blanco = {500 + i * 100, 300 + i * 50};
        
        Enjambre* enjambre = crear_enjambre(i, i, zona_ensamblaje, posicion_blanco);
        if (enjambre) {
            centro->enjambres[i] = enjambre;
            centro->num_enjambres++;
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "%d enjambres creados exitosamente", centro->num_enjambres);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Crea los blancos del sistema
 * @param centro Centro de comando
 * @return 0 si se crearon correctamente, -1 en caso de error
 */
int crear_blancos_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Creando blancos del sistema...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    for (int i = 0; i < config_num_blancos && i < MAX_BLANCOS; i++) {
        Blanco* blanco = malloc(sizeof(Blanco));
        if (blanco) {
            blanco->id = i;
            blanco->posicion.x = 500 + i * 100;
            blanco->posicion.y = 300 + i * 50;
            blanco->estado = BLANCO_INTACTO;
            blanco->enjambres_asignados = 0;
            
            if (pthread_mutex_init(&blanco->mutex_blanco, NULL) == 0) {
                centro->blancos[i] = blanco;
                centro->num_blancos++;
            } else {
                free(blanco);
            }
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "%d blancos creados exitosamente", centro->num_blancos);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Crea los camiones del sistema
 * @param centro Centro de comando
 * @return 0 si se crearon correctamente, -1 en caso de error
 */
int crear_camiones_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Creando camiones del sistema...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    int num_camiones = config_num_enjambres; // Un camión por enjambre
    
    for (int i = 0; i < num_camiones && i < MAX_DRONES; i++) {
        Camion* camion = malloc(sizeof(Camion));
        if (camion) {
            camion->id = i;
            camion->posicion.x = 50 + i * 25;
            camion->posicion.y = 50 + i * 15;
            camion->num_drones = 0;
            
            // Inicializar array de drones
            for (int j = 0; j < MAX_DRONES; j++) {
                camion->drones[j] = NULL;
            }
            
            if (pthread_mutex_init(&camion->mutex_camion, NULL) == 0) {
                centro->camiones[i] = camion;
                centro->num_camiones++;
            } else {
                free(camion);
            }
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "%d camiones creados exitosamente", centro->num_camiones);
    log_mensaje(mensaje);
    
    return 0;
}

/**
 * Asigna blancos a enjambres de forma aleatoria
 * @param centro Centro de comando
 * @return 0 si se asignaron correctamente, -1 en caso de error
 */
int asignar_blancos_enjambres(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Asignando blancos a enjambres...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    for (int i = 0; i < centro->num_enjambres; i++) {
        if (centro->enjambres[i] && i < centro->num_blancos) {
            centro->enjambres[i]->id_blanco = i;
            if (centro->blancos[i]) {
                centro->blancos[i]->enjambres_asignados++;
                centro->enjambres[i]->posicion_blanco = centro->blancos[i]->posicion;
            }
            
            char mensaje[MAX_STRING];
            snprintf(mensaje, sizeof(mensaje), "Enjambre %d asignado al blanco %d", i, i);
            log_mensaje(mensaje);
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    log_mensaje("Asignación de blancos completada");
    return 0;
}

/**
 * Asigna drones a camiones
 * @param centro Centro de comando
 * @return 0 si se asignaron correctamente, -1 en caso de error
 */
int asignar_drones_camiones(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Asignando drones a camiones...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    for (int i = 0; i < centro->num_camiones && i < centro->num_enjambres; i++) {
        Camion* camion = centro->camiones[i];
        Enjambre* enjambre = centro->enjambres[i];
        
        if (camion && enjambre) {
            // Crear 4 drones de ataque
            for (int j = 0; j < 4; j++) {
                Drone* drone = crear_drone(i * 5 + j, i, DRONE_TIPO_ATAQUE, camion->posicion);
                if (drone) {
                    camion->drones[j] = drone;
                    camion->num_drones++;
                    agregar_drone_enjambre(enjambre, drone);
                }
            }
            
            // Crear 1 drone de cámara
            Drone* drone_camara = crear_drone(i * 5 + 4, i, DRONE_TIPO_CAMARA, camion->posicion);
            if (drone_camara) {
                camion->drones[4] = drone_camara;
                camion->num_drones++;
                agregar_drone_enjambre(enjambre, drone_camara);
            }
            
            char mensaje[MAX_STRING];
            snprintf(mensaje, sizeof(mensaje), "Camión %d: %d drones asignados", i, camion->num_drones);
            log_mensaje(mensaje);
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    log_mensaje("Asignación de drones completada");
    return 0;
}

/**
 * Inicia el sistema de drones
 * @param centro Centro de comando
 * @return 0 si se inició correctamente, -1 en caso de error
 */
int iniciar_sistema_drones(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Iniciando sistema de drones...");
    
    // Inicializar todos los enjambres
    for (int i = 0; i < centro->num_enjambres; i++) {
        if (centro->enjambres[i]) {
            inicializar_enjambre(centro->enjambres[i]);
            verificar_ensamblaje_completo_enjambre(centro->enjambres[i]);
        }
    }
    
    // Inicializar todos los drones
    for (int i = 0; i < centro->num_camiones; i++) {
        if (centro->camiones[i]) {
            for (int j = 0; j < centro->camiones[i]->num_drones; j++) {
                if (centro->camiones[i]->drones[j]) {
                    inicializar_drone(centro->camiones[i]->drones[j]);
                }
            }
        }
    }
    
    log_mensaje("Sistema de drones iniciado exitosamente");
    return 0;
}

/**
 * Detiene el sistema de drones
 * @param centro Centro de comando
 * @return 0 si se detuvo correctamente, -1 en caso de error
 */
int detener_sistema_drones(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Deteniendo sistema de drones...");
    
    pthread_mutex_lock(&centro->mutex_sistema);
    centro->sistema_activo = 0;
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    log_mensaje("Sistema de drones detenido");
    return 0;
}

/**
 * Muestra el estado actual del sistema
 * @param centro Centro de comando
 * @return 0 si se mostró correctamente, -1 en caso de error
 */
int mostrar_estado_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    printf("\n=== ESTADO DEL SISTEMA DE DRONES - DRONE WARS 2 ===\n");
    printf("Tiempo: %ld\n", time(NULL));
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    // Estado de enjambres
    printf("\n--- ESTADO DE ENJAMBRES ---\n");
    for (int i = 0; i < centro->num_enjambres; i++) {
        if (centro->enjambres[i]) {
            const char* estado_str;
            switch (centro->enjambres[i]->estado) {
                case ENJAMBRE_INCOMPLETO: estado_str = "INCOMPLETO"; break;
                case ENJAMBRE_COMPLETO: estado_str = "COMPLETO"; break;
                case ENJAMBRE_ATAQUE: estado_str = "ATAQUE"; break;
                case ENJAMBRE_DESTRUIDO: estado_str = "DESTRUIDO"; break;
                default: estado_str = "DESCONOCIDO"; break;
            }
            
            printf("Enjambre %d: Estado=%s, Drones activos=%d/%d, Blanco=%d\n",
                   i, estado_str, 
                   centro->enjambres[i]->num_drones_activos,
                   centro->enjambres[i]->num_drones,
                   centro->enjambres[i]->id_blanco);
        }
    }
    
    // Estado de drones
    printf("\n--- ESTADO DE DRONES ---\n");
    for (int i = 0; i < centro->num_camiones; i++) {
        if (centro->camiones[i]) {
            printf("Camión %d: %d drones\n", i, centro->camiones[i]->num_drones);
            for (int j = 0; j < centro->camiones[i]->num_drones; j++) {
                if (centro->camiones[i]->drones[j]) {
                    Drone* drone = centro->camiones[i]->drones[j];
                    const char* tipo_str = (drone->tipo == DRONE_TIPO_ATAQUE) ? "ATAQUE" : "CAMARA";
                    const char* estado_str;
                    
                    switch (drone->estado) {
                        case DRONE_ESPERANDO: estado_str = "ESPERANDO"; break;
                        case DRONE_VOLANDO_ENSAMBLAJE: estado_str = "VOLANDO_ENSAMBLAJE"; break;
                        case DRONE_ENSAMBLANDO: estado_str = "ENSAMBLANDO"; break;
                        case DRONE_VOLANDO_BLANCO: estado_str = "VOLANDO_BLANCO"; break;
                        case DRONE_ATAQUE: estado_str = "ATAQUE"; break;
                        case DRONE_REPORTE: estado_str = "REPORTE"; break;
                        case DRONE_DESTRUIDO: estado_str = "DESTRUIDO"; break;
                        case DRONE_SIN_COMBUSTIBLE: estado_str = "SIN_COMBUSTIBLE"; break;
                        default: estado_str = "DESCONOCIDO"; break;
                    }
                    
                    printf("  Drone %d: Tipo=%s, Estado=%s, Combustible=%d%%\n",
                           drone->id, tipo_str, estado_str, drone->combustible);
                }
            }
        }
    }
    
    // Estado de blancos
    printf("\n--- ESTADO DE BLANCOS ---\n");
    for (int i = 0; i < centro->num_blancos; i++) {
        if (centro->blancos[i]) {
            const char* estado_str;
            switch (centro->blancos[i]->estado) {
                case BLANCO_INTACTO: estado_str = "INTACTO"; break;
                case BLANCO_PARCIALMENTE_DESTRUIDO: estado_str = "PARCIALMENTE_DESTRUIDO"; break;
                case BLANCO_TOTALMENTE_DESTRUIDO: estado_str = "TOTALMENTE_DESTRUIDO"; break;
                default: estado_str = "DESCONOCIDO"; break;
            }
            
            printf("Blanco %d: Estado=%s, Posición=(%d,%d)\n",
                   i, estado_str, 
                   centro->blancos[i]->posicion.x,
                   centro->blancos[i]->posicion.y);
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    printf("\n=== FIN DEL ESTADO ===\n\n");
    return 0;
}

/**
 * Verifica el estado general del sistema
 * @param centro Centro de comando
 * @return 0 si el sistema está funcionando correctamente, -1 si hay problemas
 */
int verificar_estado_general(CentroComando* centro) {
    if (!centro) return -1;
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    int problemas = 0;
    
    // Verificar enjambres
    for (int i = 0; i < centro->num_enjambres; i++) {
        if (centro->enjambres[i]) {
            if (centro->enjambres[i]->num_drones_activos == 0 && 
                centro->enjambres[i]->estado != ENJAMBRE_DESTRUIDO) {
                problemas++;
            }
        }
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    return problemas > 0 ? -1 : 0;
}

/**
 * Hilo principal del centro de comando
 * @param arg Puntero al centro de comando
 * @return NULL
 */
void* hilo_principal_centro_comando(void* arg) {
    CentroComando* centro = (CentroComando*)arg;
    if (!centro) return NULL;
    
    log_mensaje("Hilo principal del centro de comando iniciado");
    
    while (centro->sistema_activo) {
        // Verificar estado de enjambres
        for (int i = 0; i < centro->num_enjambres; i++) {
            if (centro->enjambres[i]) {
                verificar_ensamblaje_completo_enjambre(centro->enjambres[i]);
                
                // Si el enjambre está completo, activarlo
                if (centro->enjambres[i]->estado == ENJAMBRE_COMPLETO) {
                    activar_enjambre(centro->enjambres[i]);
                }
            }
        }
        
        // Esperar antes del siguiente ciclo
        sleep_ms(2000);
    }
    
    log_mensaje("Hilo principal del centro de comando terminado");
    return NULL;
}

/**
 * Hilo de monitoreo del sistema
 * @param arg Puntero al centro de comando
 * @return NULL
 */
void* hilo_monitoreo_sistema(void* arg) {
    CentroComando* centro = (CentroComando*)arg;
    if (!centro) return NULL;
    
    log_mensaje("Hilo de monitoreo del sistema iniciado");
    
    while (centro->sistema_activo) {
        // Verificar estado general
        verificar_estado_general(centro);
        
        // Esperar antes del siguiente ciclo
        sleep_ms(5000);
    }
    
    log_mensaje("Hilo de monitoreo del sistema terminado");
    return NULL;
}

/**
 * Inicializa las estructuras del sistema
 * @param centro Centro de comando
 * @return 0 si se inicializó correctamente, -1 en caso de error
 */
int inicializar_estructuras_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    log_mensaje("Inicializando estructuras del sistema...");
    
    // Crear blancos del sistema
    if (crear_blancos_sistema(centro) != 0) {
        log_mensaje("Error: No se pudieron crear los blancos");
        return -1;
    }
    
    // Crear camiones del sistema
    if (crear_camiones_sistema(centro) != 0) {
        log_mensaje("Error: No se pudieron crear los camiones");
        return -1;
    }
    
    log_mensaje("Estructuras del sistema inicializadas exitosamente");
    return 0;
}

/**
 * Genera un reporte completo del sistema
 * @param centro Centro de comando
 * @return 0 si se generó correctamente, -1 en caso de error
 */
int generar_reporte_sistema(CentroComando* centro) {
    if (!centro) return -1;
    
    printf("\n");
    printf("====================================================\n");
    printf("          REPORTE COMPLETO DEL SISTEMA\n");
    printf("             DRONE WARS 2\n");
    printf("====================================================\n");
    
    time_t ahora = time(NULL);
    struct tm* tiempo_info = localtime(&ahora);
    printf("Fecha y hora: %02d/%02d/%04d %02d:%02d:%02d\n",
           tiempo_info->tm_mday, tiempo_info->tm_mon + 1, tiempo_info->tm_year + 1900,
           tiempo_info->tm_hour, tiempo_info->tm_min, tiempo_info->tm_sec);
    
    pthread_mutex_lock(&centro->mutex_sistema);
    
    printf("\n--- RESUMEN EJECUTIVO ---\n");
    printf("Total de enjambres: %d\n", centro->num_enjambres);
    printf("Total de camiones: %d\n", centro->num_camiones);
    printf("Total de blancos: %d\n", centro->num_blancos);
    
    // Contadores de estado
    int enjambres_completos = 0, enjambres_activos = 0, enjambres_destruidos = 0;
    int drones_activos = 0, drones_ataque = 0, drones_camara = 0, drones_destruidos = 0;
    int blancos_intactos = 0, blancos_parciales = 0, blancos_destruidos = 0;
    
    // Análisis de enjambres
    for (int i = 0; i < centro->num_enjambres; i++) {
        if (centro->enjambres[i]) {
            switch (centro->enjambres[i]->estado) {
                case ENJAMBRE_COMPLETO: enjambres_completos++; break;
                case ENJAMBRE_ATAQUE: enjambres_activos++; break;
                case ENJAMBRE_DESTRUIDO: enjambres_destruidos++; break;
                default: break;
            }
        }
    }
    
    // Análisis de drones
    for (int i = 0; i < centro->num_camiones; i++) {
        if (centro->camiones[i]) {
            for (int j = 0; j < centro->camiones[i]->num_drones; j++) {
                if (centro->camiones[i]->drones[j]) {
                    Drone* drone = centro->camiones[i]->drones[j];
                    if (drone->estado != DRONE_DESTRUIDO) {
                        drones_activos++;
                        if (drone->tipo == DRONE_TIPO_ATAQUE) {
                            drones_ataque++;
                        } else {
                            drones_camara++;
                        }
                    } else {
                        drones_destruidos++;
                    }
                }
            }
        }
    }
    
    // Análisis de blancos
    for (int i = 0; i < centro->num_blancos; i++) {
        if (centro->blancos[i]) {
            switch (centro->blancos[i]->estado) {
                case BLANCO_INTACTO: blancos_intactos++; break;
                case BLANCO_PARCIALMENTE_DESTRUIDO: blancos_parciales++; break;
                case BLANCO_TOTALMENTE_DESTRUIDO: blancos_destruidos++; break;
            }
        }
    }
    
    printf("\n--- ESTADO DE LA OPERACIÓN ---\n");
    printf("Enjambres completos: %d\n", enjambres_completos);
    printf("Enjambres en ataque: %d\n", enjambres_activos);
    printf("Enjambres destruidos: %d\n", enjambres_destruidos);
    
    printf("\nDrones activos: %d\n", drones_activos);
    printf("  - Drones de ataque: %d\n", drones_ataque);
    printf("  - Drones de cámara: %d\n", drones_camara);
    printf("Drones destruidos: %d\n", drones_destruidos);
    
    printf("\nBlancos intactos: %d\n", blancos_intactos);
    printf("Blancos parcialmente destruidos: %d\n", blancos_parciales);
    printf("Blancos totalmente destruidos: %d\n", blancos_destruidos);
    
    // Eficiencia de la operación
    int total_drones = drones_activos + drones_destruidos;
    int eficiencia = total_drones > 0 ? (drones_activos * 100) / total_drones : 0;
    
    printf("\n--- ANÁLISIS DE RENDIMIENTO ---\n");
    printf("Eficiencia operacional: %d%%\n", eficiencia);
    printf("Blancos neutralizados: %d/%d\n", blancos_parciales + blancos_destruidos, centro->num_blancos);
    
    if (total_drones > 0) {
        printf("Tasa de supervivencia de drones: %d%%\n", (drones_activos * 100) / total_drones);
    }
    
    pthread_mutex_unlock(&centro->mutex_sistema);
    
    printf("\n====================================================\n");
    printf("                  FIN DEL REPORTE\n");
    printf("====================================================\n\n");
    
    return 0;
}