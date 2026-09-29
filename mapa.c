#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define AIRE       0
#define TIERRA     1
#define PIEDRA     2
#define LAVA       3
#define MINERAL    4
#define OBSIDIANA  5
#define PAJARO     6

// Calcula la diferencia de tiempo en segundos con precisión de nanosegundos
double obtener_tiempo(struct timespec t_inicio, struct timespec t_fin) {
    return (t_fin.tv_sec - t_inicio.tv_sec) + (t_fin.tv_nsec - t_inicio.tv_nsec) / 1e9;
}

// -----------------------------------------------------------------------------
// TAREA 1: Generación base del terreno (Paralelismo de Datos)
// -----------------------------------------------------------------------------
void generar_terreno(int *mapa, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            // Relieve armónico dependiente de columnas
            float relieve = sinf(j * 0.004f) * (filas * 0.12f) + 
                            cosf(j * 0.015f) * (filas * 0.04f) +
                            sinf(j * 0.050f) * (filas * 0.01f);
            
            // Densidad 2D para cavidades subterráneas
            float densidad = sinf(i * 0.04f) * cosf(j * 0.04f) + 
                             sinf((i + j) * 0.02f) * cosf((i - j) * 0.01f);

            int cota_superficie = (int)(filas * 0.35f + relieve);
            int espesor_tierra  = (int)(filas * 0.08f);

            int id_bloque;
            if (i < cota_superficie) {
                id_bloque = AIRE;
            } else if (i < cota_superficie + espesor_tierra) {
                id_bloque = TIERRA;
            } else {
                id_bloque = (densidad < -1.25f) ? AIRE : PIEDRA;
            }

            mapa[i * columnas + j] = id_bloque;
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 2A: Simulación de Iluminación (Rama Funcional A)
// Marca los bloques de TIERRA en contacto directo con AIRE
// -----------------------------------------------------------------------------
void simular_iluminacion(const int *mapa, int *mapa_luz, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int idx = i * columnas + j;
            mapa_luz[idx] = 0;

            if (mapa[idx] == TIERRA) {
                if ((i > 0 && mapa[(i - 1) * columnas + j] == AIRE) ||
                    (i < filas - 1 && mapa[(i + 1) * columnas + j] == AIRE) ||
                    (j > 0 && mapa[i * columnas + (j - 1)] == AIRE) ||
                    (j < columnas - 1 && mapa[i * columnas + (j + 1)] == AIRE)) {
                    mapa_luz[idx] = 1;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 2B: Simulación de Magma / Lava (Rama Funcional B)
// Stencil 3x3: Núcleos en celdas rodeadas íntegramente por PIEDRA
// -----------------------------------------------------------------------------
void simular_lava(const int *mapa, int *mapa_lava, int filas, int columnas) {
    for (int idx = 0; idx < filas * columnas; idx++) {
        mapa_lava[idx] = AIRE;
    }

    for (int i = 1; i < filas - 1; i++) {
        for (int j = 1; j < columnas - 1; j++) {
            int idx = i * columnas + j;

            if (mapa[idx] == PIEDRA && i > (filas * 0.45f)) {
                int vecindario_es_piedra = 1;
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        if (mapa[(i + di) * columnas + (j + dj)] != PIEDRA) {
                            vecindario_es_piedra = 0;
                            break;
                        }
                    }
                    if (!vecindario_es_piedra) break;
                }

                if (vecindario_es_piedra) {
                    mapa_lava[idx] = LAVA;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 2C: Simulación de Vetas de Mineral (Rama Funcional C)
// -----------------------------------------------------------------------------
void simular_minerales(const int *mapa, int *mapa_minerales, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int idx = i * columnas + j;
            mapa_minerales[idx] = AIRE;

            if (mapa[idx] == PIEDRA && i > (filas * 0.40f)) {
                float veta = sinf(i * 0.12f) * cosf(j * 0.12f) + sinf((i + j) * 0.04f);
                if (veta > 1.30f) {
                    mapa_minerales[idx] = MINERAL;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 2D: Simulación de Fauna / Pájaros (Rama Funcional D)
// Exige 3 bloques de margen a bordes y al menos 3 bloques de aire hacia abajo
// -----------------------------------------------------------------------------
void simular_pajaros(const int *mapa, int *mapa_pajaros, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int idx = i * columnas + j;
            mapa_pajaros[idx] = AIRE;

            if (i >= 3 && (i + 3) < filas && j >= 3 && j < (columnas - 3)) {
                if (mapa[idx] == AIRE) {
                    int despejado_inferior = (mapa[(i + 1) * columnas + j] == AIRE) &&
                                             (mapa[(i + 2) * columnas + j] == AIRE) &&
                                             (mapa[(i + 3) * columnas + j] == AIRE);

                    if (despejado_inferior) {
                        unsigned int hash = (i * 73856093) ^ (j * 19349663);
                        if ((hash % 1000) < 4) { // 0.4% de densidad de vuelo
                            mapa_pajaros[idx] = PAJARO;
                        }
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 3: Fusión, Reacciones Químicas y Censo Final (Barrera Join)
// -----------------------------------------------------------------------------
void unificar_y_censar(const int *mapa, const int *mapa_lava, const int *mapa_minerales,
                       const int *mapa_pajaros, int *mapa_final, size_t total_celdas,
                       int *censo_bloques) {
    for (int k = 0; k < 7; k++) censo_bloques[k] = 0;

    for (size_t idx = 0; idx < total_celdas; idx++) {
        int bloque = mapa[idx];

        if (mapa_pajaros[idx] == PAJARO) {
            bloque = PAJARO;
        } else if (mapa_lava[idx] == LAVA && mapa_minerales[idx] == MINERAL) {
            bloque = OBSIDIANA; // Reacción térmica entre magma y mineral
        } else if (mapa_lava[idx] == LAVA) {
            bloque = LAVA;
        } else if (mapa_minerales[idx] == MINERAL) {
            bloque = MINERAL;
        }

        mapa_final[idx] = bloque;
        censo_bloques[bloque]++;
    }
}

// -----------------------------------------------------------------------------
// MAIN: Orquestador Secuencial e Instrumentación Temporal
// -----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <filas> <columnas>\n", argv[0]);
        return 1;
    }

    int filas = atoi(argv[1]);
    int columnas = atoi(argv[2]);
    size_t total_celdas = (size_t)filas * columnas;

    int *mapa           = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_luz       = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_lava      = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_minerales = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_pajaros   = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_final     = (int *)malloc(total_celdas * sizeof(int));

    if (!mapa || !mapa_luz || !mapa_lava || !mapa_minerales || !mapa_pajaros || !mapa_final) {
        fprintf(stderr, "Error: Memoria RAM insuficiente para asignar matrices.\n");
        free(mapa); free(mapa_luz); free(mapa_lava);
        free(mapa_minerales); free(mapa_pajaros); free(mapa_final);
        return 1;
    }

    struct timespec t_inicio_global, t_fin_global;
    struct timespec t0, t1;
    double t_terreno, t_luz, t_lava, t_mineral, t_pajaro, t_unif, t_total;

    clock_gettime(CLOCK_MONOTONIC, &t_inicio_global);

    // Tarea 1: Terreno
    clock_gettime(CLOCK_MONOTONIC, &t0);
    generar_terreno(mapa, filas, columnas);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_terreno = obtener_tiempo(t0, t1);

    // Tarea 2A: Iluminación
    clock_gettime(CLOCK_MONOTONIC, &t0);
    simular_iluminacion(mapa, mapa_luz, filas, columnas);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_luz = obtener_tiempo(t0, t1);

    // Tarea 2B: Magma (3x3)
    clock_gettime(CLOCK_MONOTONIC, &t0);
    simular_lava(mapa, mapa_lava, filas, columnas);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_lava = obtener_tiempo(t0, t1);

    // Tarea 2C: Minerales
    clock_gettime(CLOCK_MONOTONIC, &t0);
    simular_minerales(mapa, mapa_minerales, filas, columnas);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_mineral = obtener_tiempo(t0, t1);

    // Tarea 2D: Pájaros
    clock_gettime(CLOCK_MONOTONIC, &t0);
    simular_pajaros(mapa, mapa_pajaros, filas, columnas);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_pajaro = obtener_tiempo(t0, t1);

    // Tarea 3: Unificación y Reducción
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int censo[7];
    int celdas_luz = 0;
    unificar_y_censar(mapa, mapa_lava, mapa_minerales, mapa_pajaros, mapa_final, total_celdas, censo);
    for (size_t i = 0; i < total_celdas; i++) {
        if (mapa_luz[i] == 1) celdas_luz++;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    t_unif = obtener_tiempo(t0, t1);

    clock_gettime(CLOCK_MONOTONIC, &t_fin_global);
    t_total = obtener_tiempo(t_inicio_global, t_fin_global);

    printf("\n================ RESUMEN DE EJECUCION SECUENCIAL ================\n");
    printf("Dimensiones: %d x %d  |  Total Celdas: %zu\n", filas, columnas, total_celdas);
    printf("----------------------------------------------------------------\n");
    printf("Tarea 1  (Generar Terreno):          %8.4f s  (%5.1f%%)\n", t_terreno, (t_terreno / t_total) * 100.0);
    printf("Tarea 2A (Iluminacion Superficie):   %8.4f s  (%5.1f%%)\n", t_luz,     (t_luz / t_total) * 100.0);
    printf("Tarea 2B (Stencil Magma 3x3):        %8.4f s  (%5.1f%%)\n", t_lava,    (t_lava / t_total) * 100.0);
    printf("Tarea 2C (Vetas de Mineral):         %8.4f s  (%5.1f%%)\n", t_mineral, (t_mineral / t_total) * 100.0);
    printf("Tarea 2D (Fauna / Pajaros):          %8.4f s  (%5.1f%%)\n", t_pajaro,  (t_pajaro / t_total) * 100.0);
    printf("Tarea 3  (Fusion, Obsidiana, Censo): %8.4f s  (%5.1f%%)\n", t_unif,    (t_unif / t_total) * 100.0);
    printf("----------------------------------------------------------------\n");
    printf("TIEMPO TOTAL SECUENCIAL:             %8.4f s  (100.0%%)\n", t_total);
    printf("================================================================\n");
    printf("Censo de Entidades en el Mapa:\n");
    printf("  [0] Aire:      %10d  |  [1] Tierra:    %10d\n", censo[0], censo[1]);
    printf("  [2] Piedra:    %10d  |  [3] Lava:      %10d\n", censo[2], censo[3]);
    printf("  [4] Mineral:   %10d  |  [5] OBSIDIANA: %10d\n", censo[4], censo[5]);
    printf("  [6] Pajaros:   %10d  |  [Luz] Bloques Iluminados: %d\n\n", censo[6], celdas_luz);

    free(mapa); free(mapa_luz); free(mapa_lava);
    free(mapa_minerales); free(mapa_pajaros); free(mapa_final);
    return 0;
}
