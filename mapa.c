#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define AIRE   0
#define TIERRA 1
#define PIEDRA 2
#define LAVA   3

// -----------------------------------------------------------------------------
// TAREA 1: Generación base (Paralelismo de Datos)
// Usa una función de altura armónica para garantizar que no existan islas
// flotantes: arriba siempre hay aire, luego un estrato de tierra y abajo roca.
// -----------------------------------------------------------------------------
void generar_terreno(int *mapa, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            // Relieve coherente dependiente de la coordenada horizontal
            float relieve = sinf(j * 0.005f) * (filas * 0.10f) + 
                            cosf(j * 0.020f) * (filas * 0.04f);
            
            int cota_superficie = (int)(filas * 0.35f + relieve);
            int espesor_tierra  = (int)(filas * 0.08f);

            int id_bloque;
            if (i < cota_superficie) {
                id_bloque = AIRE;
            } else if (i < cota_superficie + espesor_tierra) {
                id_bloque = TIERRA;
            } else {
                id_bloque = PIEDRA;
            }

            mapa[i * columnas + j] = id_bloque;
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 2: Iluminación de superficie (Rama Funcional A)
// Identifica bloques de TIERRA en contacto directo (4 vecinos) con AIRE.
// Solo lee 'mapa' y escribe en 'mapa_luz'.
// -----------------------------------------------------------------------------
void simular_iluminacion(const int *mapa, int *mapa_luz, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int idx = i * columnas + j;
            mapa_luz[idx] = 0;

            if (mapa[idx] == TIERRA) {
                // Comprobacion de fronteras con celdas de aire
                if ((i > 0 && mapa[(i - 1) * columnas + j] == AIRE) ||
                    (i < filas - 1 && mapa[(i + 1) * columnas + j] == AIRE) ||
                    (j > 0 && mapa[i * columnas + (j - 1)] == AIRE) ||
                    (j < columnas - 1 && mapa[i * columnas + (j + 1)] == AIRE)) {
                    mapa_luz[idx] = 1; // Bloque iluminado
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// TAREA 3: Generación de magma (Rama Funcional B)
// Stencil 3x3: si una celda y sus 8 vecinos son PIEDRA, el centro transmuta a LAVA.
// Solo lee 'mapa' y escribe en 'mapa_lava'.
// -----------------------------------------------------------------------------
void simular_lava(const int *mapa, int *mapa_lava, int filas, int columnas) {
    // Inicializacion con el mapa base
    for (int idx = 0; idx < filas * columnas; idx++) {
        mapa_lava[idx] = mapa[idx];
    }

    // Evaluacion del vecindario 3x3 en celdas internas
    for (int i = 1; i < filas - 1; i++) {
        for (int j = 1; j < columnas - 1; j++) {
            int idx = i * columnas + j;

            if (mapa[idx] == PIEDRA) {
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
// TAREA 4: Métricas y Reducción final
// -----------------------------------------------------------------------------
void contar_bloques(const int *mapa, int total_celdas, int *totales) {
    for (int i = 0; i < 4; i++) totales[i] = 0;
    for (int i = 0; i < total_celdas; i++) {
        totales[mapa[i]]++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <filas> <columnas>\n", argv[0]);
        return 1;
    }

    int filas = atoi(argv[1]);
    int columnas = atoi(argv[2]);
    size_t total_celdas = (size_t)filas * columnas;

    int *mapa       = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_luz   = (int *)malloc(total_celdas * sizeof(int));
    int *mapa_lava  = (int *)malloc(total_celdas * sizeof(int));

    if (!mapa || !mapa_luz || !mapa_lava) {
        fprintf(stderr, "Error: Memoria insuficiente.\n");
        free(mapa);
        free(mapa_luz);
        free(mapa_lava);
        return 1;
    }

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    // 1. Fase de generación de terreno
    generar_terreno(mapa, filas, columnas);

    // 2. Ramas funcionales independientes (leen 'mapa' en modo solo lectura)
    simular_iluminacion(mapa, mapa_luz, filas, columnas);
    simular_lava(mapa, mapa_lava, filas, columnas);

    // 3. Mediciones y reduccion
    int censo_materiales[4];
    int censo_luz = 0;

    contar_bloques(mapa_lava, total_celdas, censo_materiales);
    for (size_t i = 0; i < total_celdas; i++) {
        if (mapa_luz[i] == 1) censo_luz++;
    }

    clock_gettime(CLOCK_MONOTONIC, &fin);
    double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("Dimensiones: %d x %d (%zu celdas)\n", filas, columnas, total_celdas);
    printf("Distribucion -> Aire: %d | Tierra: %d | Piedra: %d | Lava: %d\n", 
           censo_materiales[0], censo_materiales[1], censo_materiales[2], censo_materiales[3]);
    printf("Celdas con iluminacion: %d\n", censo_luz);
    printf("Tiempo ejecucion secuencial: %.6f segundos\n", tiempo);

    free(mapa);
    free(mapa_luz);
    free(mapa_lava);
    return 0;
}

