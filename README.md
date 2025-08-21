# Sistema de Drones - Drone Wars 2

## Descripción

Este proyecto implementa un simulador de drones en configuración de enjambre para atacar blancos enemigos. El sistema está desarrollado en lenguaje C y utiliza programación multiproceso para simular la operación concurrente de múltiples drones, camiones y enjambres.

## Características Principales

- **Arquitectura Multiproceso**: Cada camión y drone es un proceso independiente
- **Sistema de Enjambres**: 5 drones por enjambre (4 de ataque + 1 de cámara)
- **Comunicación en Tiempo Real**: Sistema de comunicación directo entre centro de comando y drones
- **Control de Combustible**: Monitoreo automático y autodestrucción por agotamiento
- **Sistema de Defensa**: Simulación de defensas anti-drone con probabilidades configurables
- **Reconformación Automática**: Reasignación de drones entre enjambres incompletos
- **Interfaz de Usuario**: Modo automático e interactivo

## Arquitectura del Sistema

```
[Centro de Comando] ←→ [Camión 1] ←→ [Drone 1] ←→ [Componentes]
                    ←→ [Camión 2] ←→ [Drone 2] ←→ [Componentes]
                    ←→ [Camión N] ←→ [Drone N] ←→ [Componentes]
```

### Componentes del Drone

1. **Control de Navegación**: Dirección de vuelo y posicionamiento
2. **Control de Combustible**: Monitoreo de consumo y niveles críticos
3. **Control de Armas**: Activación y control de carga explosiva
4. **Control de Grabación**: Reporte de estado de blancos

### Estados del Sistema

- **Enjambres**: Incompleto, Completo, Ataque, Destruido
- **Drones**: Esperando, Volando, Ensamblando, Ataque, Reporte, Destruido
- **Blancos**: Intacto, Parcialmente Destruido, Totalmente Destruido

## Requisitos del Sistema

### Software
- GCC (GNU Compiler Collection) versión 4.8 o superior
- Make
- Sistema operativo Unix/Linux (Ubuntu, CentOS, etc.)

### Hardware
- Procesador: 1 GHz o superior
- Memoria RAM: 512 MB mínimo, 1 GB recomendado
- Espacio en disco: 100 MB para compilación y ejecución

## Instalación

### 1. Clonar el Repositorio
```bash
git clone <url-del-repositorio>
cd dronewars3
```

### 2. Verificar Dependencias
```bash
make check-deps
```

### 3. Compilar el Proyecto
```bash
make all
```

### 4. Instalar (Opcional)
```bash
sudo make install
```

## Uso del Sistema

### Ejecución Básica
```bash
# Ejecutar en modo automático (por defecto)
./bin/dronewars3

# Ejecutar en modo interactivo
./bin/dronewars3 -i

# Ejecutar con archivo de configuración personalizado
./bin/dronewars3 -c mi_config.txt
```

### Modos de Ejecución

#### Modo Automático
- El sistema se ejecuta sin intervención del usuario
- Monitoreo automático del estado del sistema
- Logs automáticos cada 5-10 segundos

#### Modo Interactivo
- Menú de opciones para controlar el sistema
- Visualización en tiempo real del estado
- Control manual de operaciones

### Comandos del Makefile

```bash
# Compilación
make all              # Compilar todo el proyecto
make clean            # Limpiar archivos generados
make clean-obj        # Limpiar solo archivos objeto

# Ejecución
make run              # Ejecutar programa
make run-interactive  # Ejecutar en modo interactivo
make run-config       # Ejecutar con configuración personalizada

# Utilidades
make check-deps       # Verificar dependencias
make info             # Mostrar información del proyecto
make help             # Mostrar ayuda
```

## Configuración

El sistema utiliza un archivo de configuración (`config.txt`) que permite personalizar:

- Número de blancos y enjambres
- Probabilidades de defensa y pérdida de comunicación
- Timeouts y delays del sistema
- Configuración de zonas y combustible
- Parámetros de comunicación

### Ejemplo de Configuración
```ini
# Parámetros del Sistema
NUM_BLANCOS=10
NUM_ENJAMBRES=10
DRONES_POR_ENJAMBRE=5

# Probabilidades del Sistema
PROBABILIDAD_DEFENSA=15
PROBABILIDAD_PERDIDA_COMUNICACION=5

# Timeouts y Delays
TIMEOUT_RECONEXION=30
DELAY_MOVIMIENTO=100
```

## Estructura del Proyecto

```
dronewars3/
├── common.h              # Estructuras y definiciones comunes
├── utils.c               # Funciones de utilidad del sistema
├── communication.h       # Sistema de comunicación entre procesos
├── communication.c       # Implementación del sistema de comunicación
├── drone.h               # Sistema de drones y componentes
├── drone.c               # Implementación del sistema de drones
├── swarm.h               # Gestión de enjambres
├── swarm.c               # Implementación de gestión de enjambres
├── command_center.h      # Centro de comando principal
├── command_center.c      # Implementación del centro de comando
├── main.c                # Programa principal
├── config.txt            # Archivo de configuración
├── design.md             # Documento de diseño del sistema
├── Makefile              # Script de compilación
└── README.md             # Este archivo
```

## Algoritmos Implementados

### Gestión de Enjambres
- **Ensamblaje**: Los drones se reúnen en zonas específicas antes del ataque
- **Reconformación**: Algoritmo de búsqueda alternada (izquierda-derecha) para reasignar drones
- **Exclusividad**: Un drone solo puede ser reasignado a un enjambre a la vez

### Sistema de Defensa
- Activación en zona de defensa con probabilidad configurable
- Pérdida de comunicación con probabilidad configurable
- Timeout de reconexión configurable

### Control de Combustible
- Decremento automático por unidad de desplazamiento
- Autodestrucción cuando llega a 0%
- Monitoreo continuo en hilo independiente

## Logs y Monitoreo

El sistema genera logs detallados que incluyen:

- Timestamps de todas las operaciones
- Cambios de estado de drones y enjambres
- Eventos de comunicación y reconexión
- Alertas de combustible crítico
- Reportes de ataques y destrucción de blancos

### Ejemplo de Log
```
[14:30:15] Drone 5 creado - Tipo: Ataque, Enjambre: 2
[14:30:16] Enjambre 2 ensamblaje completado
[14:30:17] Drone 5 ejecutando ataque al blanco
[14:30:18] Drone de ataque completó su misión - Autodestrucción
```

## Solución de Problemas

### Errores Comunes

1. **Error de compilación**: Verificar que GCC esté instalado
2. **Error de permisos**: Ejecutar con permisos de administrador si es necesario
3. **Error de comunicación**: Verificar que el puerto configurado esté disponible

### Debugging

```bash
# Compilar con información de debug
make clean
make CFLAGS="-Wall -Wextra -std=c99 -g -O0 -DDEBUG" all

# Ejecutar con valgrind (si está disponible)
valgrind --leak-check=full ./bin/dronewars3
```

## Contribución

Para contribuir al proyecto:

1. Fork del repositorio
2. Crear rama para nueva funcionalidad
3. Implementar cambios con comentarios detallados
4. Probar exhaustivamente
5. Crear Pull Request

## Licencia

Este proyecto está bajo licencia [especificar licencia].

## Contacto

Para preguntas o soporte técnico:
- Email: [email]
- Repositorio: [url-del-repositorio]
- Documentación: [url-de-documentacion]

## Changelog

### Versión 1.0.0
- Implementación inicial del sistema de drones
- Sistema de enjambres funcional
- Comunicación en tiempo real
- Interfaz de usuario básica

## Agradecimientos

- Profesores del curso de Sistemas Operativos
- Comunidad de desarrolladores de código abierto
- Contribuidores del proyecto
