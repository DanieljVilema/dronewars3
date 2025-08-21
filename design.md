# Diseño del Sistema de Simulación de Drones - Drone Wars 2

## Arquitectura General

El sistema está diseñado como una aplicación multiproceso en C que simula un enjambre de drones atacando blancos enemigos. La arquitectura se basa en procesos independientes que se comunican a través de mecanismos de IPC del sistema operativo.

## Componentes Principales

### 1. Procesos del Sistema
- **Centro de Comando**: Proceso principal que coordina todos los drones
- **Camiones**: Procesos independientes que contienen los drones
- **Drones**: Procesos que representan cada vehículo aéreo
- **Componentes del Drone**: Hilos dentro de cada proceso drone

### 2. Estructura de Comunicación
- **Comunicación Directa**: Entre centro de comando y cada drone
- **Eventos Reportados**: Solo cambios de estado críticos (despegue, llegada a zona, pérdida de comunicación)
- **Control de Conexión**: Verificación periódica de enlace con timeout configurable

### 3. Algoritmos de Sincronización

#### Gestión de Enjambres
- **Ensamblaje**: Los drones se reúnen en zonas específicas antes del ataque
- **Reconformación**: Algoritmo de búsqueda alternada (izquierda-derecha) para reasignar drones
- **Exclusividad**: Un drone solo puede ser reasignado a un enjambre a la vez

#### Control de Combustible
- Decremento automático por unidad de desplazamiento
- Autodestrucción cuando llega a 0%
- Monitoreo continuo en hilo independiente

#### Sistema de Defensa
- Activación en zona de defensa con probabilidad W%
- Pérdida de comunicación con probabilidad Q%
- Timeout de reconexión configurable (Z segundos)

## Diagrama de Despliegue

```
[Centro de Comando] ←→ [Camión 1] ←→ [Drone 1] ←→ [Componentes]
                    ←→ [Camión 2] ←→ [Drone 2] ←→ [Componentes]
                    ←→ [Camión N] ←→ [Drone N] ←→ [Componentes]
```

## Estructuras de Datos

- **Enjambre**: Lista enlazada de drones con estado de completitud
- **Zonas**: Matriz de coordenadas para ensamblaje y blancos
- **Comunicación**: Cola de mensajes con prioridad para eventos críticos
- **Estado del Sistema**: Estructura compartida con mutex para sincronización

## Mecanismos de Sincronización

- **Mutex**: Para acceso exclusivo a estructuras compartidas
- **Semáforos**: Para control de concurrencia en zonas críticas
- **Condiciones**: Para señalización entre hilos de comunicación
- **Timeouts**: Para detección de pérdida de enlace y reconexión
