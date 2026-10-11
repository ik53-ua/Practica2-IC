# Documentación Arquitectónica - Práctica 2

## 1. Grafo de Dependencias de Tareas (Apartado 2.1)

Representación formal de dependencias de precedencia (Read-After-Write) y paralelismo funcional (*Fork-Join* de 4 ramas):

```mermaid
graph TD
    INICIO([Inicio]) --> T1["Tarea 1: generar_terreno()"]
    
    T1 -->|"mapa (Lectura RAW)"| T2A["Tarea 2A: simular_iluminacion()"]
    T1 -->|"mapa (Lectura RAW)"| T2B["Tarea 2B: simular_lava() [Stencil 3x3]"]
    T1 -->|"mapa (Lectura RAW)"| T2C["Tarea 2C: simular_minerales()"]
    T1 -->|"mapa (Lectura RAW)"| T2D["Tarea 2D: simular_pajaros()"]
    
    T2A -->|"mapa_luz"| T3["Tarea 3: unificar_y_censar()"]
    T2B -->|"mapa_lava"| T3
    T2C -->|"mapa_minerales"| T3
    T2D -->|"mapa_pajaros"| T3
    
    T3 -->|"censo_bloques"| FIN([Métricas y Cierre])
```

### Justificación Teórica:
- **Paralelismo de Datos:** Cada tarea interna divide su matriz por filas sin dependencias cruzadas entre celdas.
- **Paralelismo Funcional:** Las tareas 2A, 2B, 2C y 2D operan simultáneamente sobre la entrada común `mapa` (solo lectura) y generan buffers independientes en memoria sin requerir bloqueos de exclusión mutua.
- **Sincronización (Join):** La Tarea 3 actúa como barrera de sincronización y resuelve colisiones físicas (Magma + Mineral = Obsidiana).

---

## 2. Diagrama de Flujo de Ejecución Secuencial

Secuencia temporal del proceso en la CPU:

```mermaid
flowchart TD
    A([Inicio: main]) --> B["Lectura de argumentos CLI: filas, columnas"]
    B --> C["malloc() de 6 búferes contiguos"]
    C --> D{"¿Asignación válida?"}
    D -- No --> E[Abortar con código de error]
    D -- Sí --> F["Iniciar cronómetro global"]
    F --> G["Fase 1: generar_terreno()"]
    G --> H["Fase 2A: simular_iluminacion()"]
    H --> I["Fase 2B: simular_lava()"]
    I --> J["Fase 2C: simular_minerales()"]
    J --> K["Fase 2D: simular_pajaros()"]
    K --> L["Fase 3: unificar_y_censar()"]
    L --> M["Detener cronómetro y calcular porcentajes"]
    M --> N["printf(): Informe de tiempos y censo"]
    N --> O["free() de todos los búferes"]
    O --> P([Fin del Programa])
```
