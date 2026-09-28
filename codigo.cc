#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define AIRE 0
#define TIERRA 1
#define PIEDRA 2
#define LAVA 3

// ---------------------------------------------------------
// TAREA 1: Generacion base (Paralelismo de Datos espacial)
// ---------------------------------------------------------
void generar_terreno(int *mapa, int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            // Ruido matematico simple para simular el relieve y cuevas
            float ruido = (sin(i * 0.1) + cos(j * 0.1) + sin((i + j) * 0.05) + 3.0) / 6.0;
            
            int id_bloque;
            if (ruido < 0.4) id_bloque = AIRE;          // 40% de probabilidad base de aire
            else if (ruido < 0.7) id_bloque = TIERRA;   
            else id_bloque = PIEDRA;                    

            mapa[i * columnas + j] = id_bloque;
        }
    }
}

// ---------------------------------------------------------
// TAREA 2: Iluminacion (Paralelismo Funcional - Rama A)
// Trazado de rayos vertical: la luz cae hasta chocar con algo solido
// ---------------------------------------------------------
void simular_iluminacion(int *mapa, int *mapa_luz, int filas, int columnas) {
    for (int j = 0; j < columnas; j++) {
        int luz_directa = 1; // 1 = Sol directo, 0 = Sombra
        for (int i = 0; i < filas; i++) {
            int indice = i * columnas + j;
            // Si chocamos con tierra o piedra, bloqueamos la luz para las celdas inferiores
            if (mapa[indice] == TIERRA || mapa[indice] == PIEDRA) {
                luz_directa = 0; 
            }
            mapa_luz[indice] = luz_directa;
        }
    }
}

// ---------------------------------------------------------
// TAREA 3: Fluidos (Paralelismo Funcional - Rama B)
// Inunda con lava las zonas de aire que esten en la mitad inferior del mapa
// ---------------------------------------------------------
void simular_fluidos(int *mapa, int *mapa_fluidos, int filas, int columnas) {
    int nivel_magma = filas / 2; 
    for (int i = nivel_magma; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int indice = i * columnas + j;
            if (mapa[indice] == AIRE) {
                mapa_fluidos[indice] = LAVA;
            } else {
                mapa_fluidos[indice] = mapa[indice]; // Copia el bloque original si no es aire
            }
        }
    }
}

// ---------------------------------------------------------
// TAREA 4: Auditoria (Validacion y cierre)
// ---------------------------------------------------------
void contar_bloques(int *mapa, int filas, int columnas, int *totales) {
    for (int i = 0; i < 4; i++) totales[i] = 0; // Inicializar contadores

    for (int i = 0; i < filas * columnas; i++) {
        totales[mapa[i]]++; // El valor del bloque (0, 1, 2, 3) coincide con el indice del array
    }
}

int main(int argc, char *argv[]) {
    // 1. Configuracion y carga de trabajo segun parametros
    if (argc != 3) {
        printf("Uso: %s <filas> <columnas>\n", argv[0]);
        return 1;
    }
    int filas = atoi(argv[1]);
    int columnas = atoi(argv[2]);

    int *mapa = (int *)malloc(filas * columnas * sizeof(int));
    int *mapa_luz = (int *)malloc(filas * columnas * sizeof(int));
    int *mapa_fluidos = (int *)malloc(filas * columnas * sizeof(int)); // NUEVO ARREGLO
    
    if (!mapa || !mapa_luz || !mapa_fluidos) {
        printf("Error de memoria.\n");
        return 1;
    }

    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio); // Inicio de medicion

    // 2. Ejecucion del Pipeline Secuencial
    generar_terreno(mapa, filas, columnas);
    
    // Estas dos tareas actuan como ramas independientes en vuestro analisis teorico
    simular_iluminacion(mapa, mapa_luz, filas, columnas);
    simular_fluidos(mapa, mapa_fluidos, filas, columnas); // LLAMADA CORREGIDA
    
    int censo[4];
    // Contamos sobre el mapa_fluidos para poder registrar la lava generada
    contar_bloques(mapa_fluidos, filas, columnas, censo);

    clock_gettime(CLOCK_MONOTONIC, &fin); // Fin de medicion
    double tiempo = (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;

    // 3. Resultados
    printf("Generacion completada: %d x %d\n", filas, columnas);
    printf("Censo -> Aire: %d | Tierra: %d | Piedra: %d | Lava: %d\n", censo[0], censo[1], censo[2], censo[3]);
    printf("Tiempo total de ejecucion secuencial: %f segundos\n", tiempo);

    free(mapa);
    free(mapa_luz);
    free(mapa_fluidos); // LIBERAR MEMORIA
    return 0;
}