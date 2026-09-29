# Documentación Arquitectónica - Práctica 2 (Versión Base)

## 1. Grafo de Dependencias de Tareas (Apartado 2.1)

Representación formal de dependencias de precedencia (Read-After-Write) y paralelismo funcional (*Fork-Join* de 2 ramas independientes):

```mermaid
graph TD
    INICIO([Inicio]) --> T1["Tarea 1: generar_terreno()"]
    
    T1 -->|"mapa (Lectura RAW)"| T2A["Tarea 2A: simular_iluminacion()"]
    T1 -->|"mapa (Lectura RAW)"| T2B["Tarea 2B: simular_lava() [Stencil 3x3]"]
    
    T2A -->|"mapa_luz"| T3["Tarea 3: Auditoría y Censo"]
    T2B -->|"mapa_lava"| T3
    
    T3 -->|"censo_bloques"| FIN([Métricas y Cierre])
```

### Justificación Teórica:
- **Paralelismo de Datos:** Las tareas internas procesan las matrices fila a fila sin dependencias entre celdas dentro del mismo paso.
- **Paralelismo Funcional:** Las tareas 2A y 2B leen de forma no destructiva la matriz `mapa` (solo lectura con `const int*`) y escriben en matrices disjuntas (`mapa_luz` y `mapa_lava`), permitiendo su ejecución concurrente sin regiones críticas ni bloqueos.
- **Barrera de Sincronización (Join):** La Tarea 3 actúa como punto de encuentro para procesar el censo de bloques y los puntos iluminados antes de liberar la memoria.

---

## 2. Diagrama de Flujo de Ejecución Secuencial (`main.c`)

Secuencia temporal del proceso en la CPU:

```mermaid
flowchart TD
    A([Inicio: main]) --> B["Lectura de argumentos CLI: filas y columnas"]
    B --> C["malloc() de búferes: mapa, mapa_luz, mapa_lava"]
    C --> D{"¿Memoria asignada con éxito?"}
    
    D -- No --> E[Mostrar Error de Memoria y Salir]
    D -- Sí --> F["Iniciar cronómetro global (clock_gettime)"]
    
    F --> G["1. generar_terreno() [Medir t_terreno]"]
    G --> H["2. simular_iluminacion() [Medir t_luz]"]
    H --> I["3. simular_lava() [Medir t_lava]"]
    I --> J["4. Auditoría y Censo [Medir t_censo]"]
    
    J --> K["Detener cronómetro global"]
    K --> L["printf(): Informe de tiempos, porcentajes y censo"]
    L --> M["free() de todos los búferes"]
    M --> N([Fin del Programa])
```

