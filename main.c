#include "common.h"
#include "communication.h"
#include "drone.h"
#include "swarm.h"
#include "command_center.h"

// Variables globales del sistema
CentroComando* centro_comando = NULL;
int sistema_activo = 1;

// Función para manejar señales del sistema
void manejar_senal(int senal) {
    char mensaje[MAX_STRING];
    snprintf(mensaje, sizeof(mensaje), "Señal recibida: %d - Cerrando sistema...", senal);
    log_mensaje(mensaje);
    
    sistema_activo = 0;
    
    if (centro_comando) {
        detener_sistema_drones(centro_comando);
    }
}

/**
 * Función principal del programa
 * @return 0 si se ejecutó correctamente, -1 en caso de error
 */
int main(int argc, char* argv[]) {
    log_mensaje("=== INICIANDO SISTEMA DE DRONES - DRONE WARS 2 ===");
    
    // Configurar manejo de señales
    signal(SIGINT, manejar_senal);
    signal(SIGTERM, manejar_senal);
    
    // Inicializar sistema
    inicializar_aleatorios();
    
    // Cargar configuración
    if (cargar_configuracion("config.txt") != 0) {
        log_mensaje("Error: No se pudo cargar la configuración del sistema");
        return -1;
    }
    
    // Crear centro de comando
    centro_comando = crear_centro_comando();
    if (!centro_comando) {
        log_mensaje("Error: No se pudo crear el centro de comando");
        return -1;
    }
    
    // Inicializar centro de comando
    if (inicializar_centro_comando(centro_comando) != 0) {
        log_mensaje("Error: No se pudo inicializar el centro de comando");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Crear estructuras del sistema
    if (inicializar_estructuras_sistema(centro_comando) != 0) {
        log_mensaje("Error: No se pudo inicializar las estructuras del sistema");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Crear enjambres
    if (crear_enjambres_sistema(centro_comando) != 0) {
        log_mensaje("Error: No se pudo crear los enjambres del sistema");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Crear camiones
    if (crear_camiones_sistema(centro_comando) != 0) {
        log_mensaje("Error: No se pudo crear los camiones del sistema");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Crear blancos
    if (crear_blancos_sistema(centro_comando) != 0) {
        log_mensaje("Error: No se pudo crear los blancos del sistema");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Asignar blancos a enjambres
    if (asignar_blancos_enjambres(centro_comando) != 0) {
        log_mensaje("Error: No se pudo asignar blancos a enjambres");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Asignar drones a camiones
    if (asignar_drones_camiones(centro_comando) != 0) {
        log_mensaje("Error: No se pudo asignar drones a camiones");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    // Iniciar sistema de drones
    if (iniciar_sistema_drones(centro_comando) != 0) {
        log_mensaje("Error: No se pudo iniciar el sistema de drones");
        destruir_centro_comando(centro_comando);
        return -1;
    }
    
    log_mensaje("Sistema de drones iniciado exitosamente");
    log_mensaje("Presiona Ctrl+C para detener el sistema");
    
    // Bucle principal del sistema
    while (sistema_activo) {
        // Mostrar estado del sistema cada 5 segundos
        mostrar_estado_sistema(centro_comando);
        
        // Verificar estado general del sistema
        if (verificar_estado_general(centro_comando) != 0) {
            log_mensaje("Advertencia: Problemas detectados en el sistema");
        }
        
        // Esperar antes del siguiente ciclo
        sleep_ms(5000);
    }
    
    log_mensaje("Cerrando sistema de drones...");
    
    // Detener sistema
    if (centro_comando) {
        detener_sistema_drones(centro_comando);
        destruir_centro_comando(centro_comando);
    }
    
    log_mensaje("Sistema de drones cerrado exitosamente");
    log_mensaje("=== FIN DEL SISTEMA ===");
    
    return 0;
}

/**
 * Función para mostrar el menú principal del sistema
 */
void mostrar_menu_principal(void) {
    printf("\n=== MENÚ PRINCIPAL - SISTEMA DE DRONES ===\n");
    printf("1. Mostrar estado del sistema\n");
    printf("2. Mostrar estado de enjambres\n");
    printf("3. Mostrar estado de camiones\n");
    printf("4. Mostrar estado de blancos\n");
    printf("5. Generar reporte del sistema\n");
    printf("6. Salir del sistema\n");
    printf("Seleccione una opción: ");
}

/**
 * Función para procesar la entrada del usuario
 * @param opcion Opción seleccionada por el usuario
 * @return 0 si se procesó correctamente, -1 en caso de error
 */
int procesar_opcion_usuario(int opcion) {
    if (!centro_comando) return -1;
    
    switch (opcion) {
        case 1:
            mostrar_estado_sistema(centro_comando);
            break;
        case 2:
            // Mostrar estado de enjambres
            printf("Estado de enjambres:\n");
            for (int i = 0; i < centro_comando->num_enjambres; i++) {
                if (centro_comando->enjambres[i]) {
                    printf("Enjambre %d: Estado %d, Drones activos: %d\n", 
                           i, centro_comando->enjambres[i]->estado, 
                           centro_comando->enjambres[i]->num_drones_activos);
                }
            }
            break;
        case 3:
            // Mostrar estado de camiones
            printf("Estado de camiones:\n");
            for (int i = 0; i < centro_comando->num_camiones; i++) {
                if (centro_comando->camiones[i]) {
                    printf("Camión %d: Drones: %d\n", 
                           i, centro_comando->camiones[i]->num_drones);
                }
            }
            break;
        case 4:
            // Mostrar estado de blancos
            printf("Estado de blancos:\n");
            for (int i = 0; i < centro_comando->num_blancos; i++) {
                if (centro_comando->blancos[i]) {
                    printf("Blanco %d: Estado %d\n", 
                           i, centro_comando->blancos[i]->estado);
                }
            }
            break;
        case 5:
            generar_reporte_sistema(centro_comando);
            break;
        case 6:
            sistema_activo = 0;
            printf("Cerrando sistema...\n");
            break;
        default:
            printf("Opción inválida. Por favor seleccione una opción válida.\n");
            return -1;
    }
    
    return 0;
}

/**
 * Función para ejecutar el sistema en modo interactivo
 */
void ejecutar_modo_interactivo(void) {
    int opcion;
    
    while (sistema_activo) {
        mostrar_menu_principal();
        
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada inválida. Por favor ingrese un número.\n");
            while (getchar() != '\n'); // Limpiar buffer
            continue;
        }
        
        if (procesar_opcion_usuario(opcion) != 0) {
            printf("Error al procesar la opción seleccionada.\n");
        }
        
        // Pausa para que el usuario pueda ver la salida
        printf("\nPresiona Enter para continuar...");
        while (getchar() != '\n'); // Limpiar buffer
        getchar(); // Esperar Enter
        
        limpiar_pantalla();
    }
}

/**
 * Función para ejecutar el sistema en modo automático
 */
void ejecutar_modo_automatico(void) {
    log_mensaje("Ejecutando sistema en modo automático");
    
    while (sistema_activo) {
        // Mostrar estado del sistema cada 10 segundos
        mostrar_estado_sistema(centro_comando);
        
        // Verificar estado general del sistema
        if (verificar_estado_general(centro_comando) != 0) {
            log_mensaje("Advertencia: Problemas detectados en el sistema");
        }
        
        // Esperar antes del siguiente ciclo
        sleep_ms(10000);
    }
}

/**
 * Función para mostrar información de ayuda
 */
void mostrar_ayuda(void) {
    printf("=== AYUDA - SISTEMA DE DRONES ===\n");
    printf("Uso: %s [OPCIONES]\n", "dronewars3");
    printf("\nOpciones:\n");
    printf("  -h, --help     Mostrar esta ayuda\n");
    printf("  -i, --interactive  Ejecutar en modo interactivo\n");
    printf("  -a, --auto     Ejecutar en modo automático (por defecto)\n");
    printf("  -c, --config   Archivo de configuración (por defecto: config.txt)\n");
    printf("\nEjemplos:\n");
    printf("  %s              Ejecutar en modo automático\n", "dronewars3");
    printf("  %s -i           Ejecutar en modo interactivo\n", "dronewars3");
    printf("  %s -c mi_config.txt  Usar archivo de configuración personalizado\n", "dronewars3");
    printf("\nModos de ejecución:\n");
    printf("  Automático: El sistema se ejecuta sin intervención del usuario\n");
    printf("  Interactivo: El usuario puede controlar el sistema mediante menús\n");
}
