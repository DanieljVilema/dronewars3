#include "common.h"

// Variables globales de configuración
int config_num_blancos = 10;
int config_num_enjambres = 10;
int config_probabilidad_defensa = 15;
int config_probabilidad_comunicacion = 5;
int config_timeout_reconexion = 30;

/**
 * Calcula la distancia euclidiana entre dos coordenadas
 * @param c1 Primera coordenada
 * @param c2 Segunda coordenada
 * @return Distancia calculada
 */
int calcular_distancia(Coordenada c1, Coordenada c2) {
    int dx = c1.x - c2.x;
    int dy = c1.y - c2.y;
    return (int)sqrt(dx * dx + dy * dy);
}

/**
 * Genera un número aleatorio y verifica si está dentro del porcentaje especificado
 * @param porcentaje Porcentaje de probabilidad (0-100)
 * @return 1 si se cumple la probabilidad, 0 en caso contrario
 */
int generar_probabilidad(int porcentaje) {
    if (porcentaje <= 0) return 0;
    if (porcentaje >= 100) return 1;
    
    int numero_aleatorio = rand() % 100;
    return (numero_aleatorio < porcentaje) ? 1 : 0;
}

/**
 * Suspende la ejecución por un número específico de milisegundos
 * @param milliseconds Tiempo de espera en milisegundos
 */
void sleep_ms(int milliseconds) {
    usleep(milliseconds * 1000);
}

/**
 * Registra un mensaje en el log del sistema con timestamp
 * @param mensaje Mensaje a registrar
 */
void log_mensaje(const char* mensaje) {
    time_t ahora = time(NULL);
    struct tm* tiempo_info = localtime(&ahora);
    
    printf("[%02d:%02d:%02d] %s\n", 
           tiempo_info->tm_hour, 
           tiempo_info->tm_min, 
           tiempo_info->tm_sec, 
           mensaje);
}

/**
 * Limpia la pantalla del terminal (compatible con Unix/Linux)
 */
void limpiar_pantalla(void) {
    printf("\033[2J\033[H"); // Códigos ANSI para limpiar pantalla
}

/**
 * Carga la configuración desde un archivo de texto
 * @param archivo Nombre del archivo de configuración
 * @return 0 si se cargó correctamente, -1 en caso de error
 */
int cargar_configuracion(const char* archivo) {
    FILE* file = fopen(archivo, "r");
    if (!file) {
        log_mensaje("Error: No se pudo abrir el archivo de configuración");
        return -1;
    }
    
    char linea[MAX_STRING];
    char clave[MAX_STRING];
    char valor[MAX_STRING];
    
    while (fgets(linea, sizeof(linea), file)) {
        // Ignorar comentarios y líneas vacías
        if (linea[0] == '#' || linea[0] == '\n') continue;
        
        if (sscanf(linea, "%[^=]=%s", clave, valor) == 2) {
            // Eliminar espacios en blanco
            char* k = clave;
            char* v = valor;
            while (*k == ' ') k++;
            while (*v == ' ') v++;
            
            // Asignar valores según la clave
            if (strcmp(k, "NUM_BLANCOS") == 0) {
                config_num_blancos = atoi(v);
            } else if (strcmp(k, "NUM_ENJAMBRES") == 0) {
                config_num_enjambres = atoi(v);
            } else if (strcmp(k, "PROBABILIDAD_DEFENSA") == 0) {
                config_probabilidad_defensa = atoi(v);
            } else if (strcmp(k, "PROBABILIDAD_PERDIDA_COMUNICACION") == 0) {
                config_probabilidad_comunicacion = atoi(v);
            } else if (strcmp(k, "TIMEOUT_RECONEXION") == 0) {
                config_timeout_reconexion = atoi(v);
            }
        }
    }
    
    fclose(file);
    log_mensaje("Configuración cargada exitosamente");
    return 0;
}

/**
 * Inicializa el generador de números aleatorios
 */
void inicializar_aleatorios(void) {
    srand(time(NULL));
}

/**
 * Verifica si una coordenada está dentro de una zona circular
 * @param centro Centro de la zona
 * @param radio Radio de la zona
 * @param posicion Posición a verificar
 * @return 1 si está dentro, 0 en caso contrario
 */
int esta_en_zona(Coordenada centro, int radio, Coordenada posicion) {
    return calcular_distancia(centro, posicion) <= radio;
}

/**
 * Mueve una coordenada hacia un objetivo con un paso específico
 * @param actual Posición actual
 * @param objetivo Posición objetivo
 * @param paso Tamaño del paso de movimiento
 * @return Nueva posición calculada
 */
Coordenada mover_hacia_objetivo(Coordenada actual, Coordenada objetivo, int paso) {
    Coordenada nueva = actual;
    int distancia = calcular_distancia(actual, objetivo);
    
    if (distancia <= paso) {
        // Llegamos al objetivo
        nueva = objetivo;
    } else {
        // Calculamos la dirección y movemos un paso
        double dx = (double)(objetivo.x - actual.x) / distancia;
        double dy = (double)(objetivo.y - actual.y) / distancia;
        
        nueva.x = actual.x + (int)(dx * paso);
        nueva.y = actual.y + (int)(dy * paso);
    }
    
    return nueva;
}
