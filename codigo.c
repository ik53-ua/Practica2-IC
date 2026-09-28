#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define AIRE 0
#define TIERRA 1
#define PIEDRA 2
#define LAVA 3
#define ANDESITA 4
#define DIORITA 5
#define CALCITA 6

#define NUM_MATERIALES 7

static inline int calcular_id_bloque(int i, int j, int filas) {
    float ruido = (sin(i * 0.8) + cos(j * 0.8) + sin((i + j) * 0.3) + 3.0) / 6.0;
    float profundidad = (float)i / filas;
    float densidad = profundidad + (ruido - 0.5) * 1.2;
    
    if (densidad < 0.40) return AIRE;
    
    if (densidad < 0.48) {
        if (profundidad < 0.5) {
            return TIERRA;
        } else {
            return PIEDRA; 
        }
    }
    
    float veta = (sin(i * 0.3) * cos(j * 0.3)); 
    if (veta > 0.7) return CALCITA;
    if (veta > 0.4) return ANDESITA;
    if (veta < -0.6) return DIORITA;
    
    return PIEDRA;
}

void generar_terreno(int *mapa, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            mapa[i * columnas + j] = calcular_id_bloque(i, j, filas);
        }
    }
}

void simular_iluminacion(int *mapa, int *mapa_luz, int filas, int columnas) {
    for (int j = 0; j < columnas; j++) {
        int luz_directa = 1; 
        for (int i = 0; i < filas; i++) {
            int indice = i * columnas + j;
            if (mapa[indice] != AIRE && mapa[indice] != LAVA) {
                luz_directa = 0; 
            }
            mapa_luz[indice] = luz_directa;
        }
    }
}

void simular_fluidos(int *mapa, int *mapa_fluidos, int filas, int columnas) {
    int nivel_magma = filas / 2; 
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int indice = i * columnas + j;
            if (i >= nivel_magma && mapa[indice] == AIRE) {
                mapa_fluidos[indice] = LAVA;
            } else {
                mapa_fluidos[indice] = mapa[indice]; 
            }
        }
    }
}

void contar_bloques(int *mapa, int filas, int columnas, int *totales) {
    for (int i = 0; i < NUM_MATERIALES; i++) totales[i] = 0; 
    for (int i = 0; i < filas * columnas; i++) {
        totales[mapa[i]]++; 
    }
}

void imprimir_debug_visual(int *mapa_fluidos, int *mapa_luz, int filas, int columnas) {
    if (filas > 40 || columnas > 40) return;

    printf("\n--- MODO DEPURACION VISUAL (%dx%d) ---\n", filas, columnas);
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int bloque = mapa_fluidos[i * columnas + j];
            int luz = mapa_luz[i * columnas + j];
            
            if (luz == 1 && bloque == AIRE) printf("\x1b[43m \x1b[0m");
            else if (bloque == AIRE) printf(" ");
            else if (bloque == TIERRA) printf("\x1b[32mT\x1b[0m");
            else if (bloque == LAVA) printf("\x1b[31m~\x1b[0m");
            else printf("\x1b[90m#\x1b[0m");
        }
        printf("\n");
    }
    printf("--------------------------------------\n\n");
}

void validar_invariantes(int *mapa_base, int *mapa_luz, int *mapa_fluidos, int filas, int columnas) {
    int errores = 0;
    int nivel_magma = filas / 2;
    int aire_original = 0;
    int aire_final = 0, lava_final = 0;

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int indice = i * columnas + j;
            int bloque_base = mapa_base[indice];
            int bloque_fluido = mapa_fluidos[indice];

            if (bloque_base == AIRE) aire_original++;
            if (bloque_fluido == AIRE) aire_final++;
            if (bloque_fluido == LAVA) lava_final++;

            if (i < nivel_magma && bloque_fluido == LAVA) {
                printf("  [!] Error de gravedad: Lava en altura %d\n", i);
                errores++;
            }

            if (i < filas - 1) {
                int indice_abajo = (i + 1) * columnas + j;
                if (bloque_base != AIRE && bloque_base != LAVA) { 
                    if (mapa_luz[indice_abajo] == 1) {
                        printf("  [!] Error de luz en (%d, %d)\n", i, j);
                        errores++;
                    }
                }
            }
        }
    }

    if (aire_original != (aire_final + lava_final)) {
        printf("  [!] Error de masa: Original (%d) vs Final (%d)\n", aire_original, aire_final + lava_final);
        errores++;
    }

    if (errores == 0) {
        printf("Auditoria de fisicas -> \x1b[32mSUPERADA (0 errores)\x1b[0m\n");
    } else {
        printf("Auditoria de fisicas -> \x1b[31mFALLIDA (%d anomalías)\x1b[0m\n", errores);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <filas> <columnas>\n", argv[0]);
        return 1;
    }
    int filas = atoi(argv[1]);
    int columnas = atoi(argv[2]);

    int *mapa = (int *)malloc(filas * columnas * sizeof(int));
    int *mapa_luz = (int *)malloc(filas * columnas * sizeof(int));
    int *mapa_fluidos = (int *)malloc(filas * columnas * sizeof(int)); 
    
    if (!mapa || !mapa_luz || !mapa_fluidos) {
        printf("Error de memoria.\n");
        return 1;
    }

    struct timespec t0, t1, t2, t3, t4;

    clock_gettime(CLOCK_MONOTONIC, &t0); 
    generar_terreno(mapa, filas, columnas);
    
    clock_gettime(CLOCK_MONOTONIC, &t1);
    simular_iluminacion(mapa, mapa_luz, filas, columnas);
    
    clock_gettime(CLOCK_MONOTONIC, &t2);
    simular_fluidos(mapa, mapa_fluidos, filas, columnas); 
    
    clock_gettime(CLOCK_MONOTONIC, &t3);
    int censo[NUM_MATERIALES];
    contar_bloques(mapa_fluidos, filas, columnas, censo);
    
    clock_gettime(CLOCK_MONOTONIC, &t4); 

    double t_gen   = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    double t_ilum  = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;
    double t_fluid = (t3.tv_sec - t2.tv_sec) + (t3.tv_nsec - t2.tv_nsec) / 1e9;
    double t_censo = (t4.tv_sec - t3.tv_sec) + (t4.tv_nsec - t3.tv_nsec) / 1e9;
    double t_total = (t4.tv_sec - t0.tv_sec) + (t4.tv_nsec - t0.tv_nsec) / 1e9;

    printf("Generacion completada: %d x %d\n", filas, columnas);
    printf("Censo:\n - Aire: %d\n - Tierra: %d\n - Piedra: %d\n - Lava: %d\n - Andesita: %d\n - Diorita: %d\n - Calcita: %d\n", 
            censo[0], censo[1], censo[2], censo[3], censo[4], censo[5], censo[6]);
            
    printf("\n--- DESGLOSE DE RENDIMIENTO ---\n");
    printf("1. Generacion terreno: %f seg\n", t_gen);
    printf("2. Simular luz:        %f seg\n", t_ilum);
    printf("3. Simular fluidos:    %f seg\n", t_fluid);
    printf("4. Auditoria censo:    %f seg\n", t_censo);
    printf("-------------------------------\n");
    printf("TIEMPO TOTAL CPU:      %f seg\n\n", t_total);

    validar_invariantes(mapa, mapa_luz, mapa_fluidos, filas, columnas);
    imprimir_debug_visual(mapa_fluidos, mapa_luz, filas, columnas);

    free(mapa);
    free(mapa_luz);
    free(mapa_fluidos); 
    return 0;
}