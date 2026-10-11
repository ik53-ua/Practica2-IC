CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

# Caso de prueba de referencia
FILAS = 30000
COLUMNAS = 30000

TARGET = generador_mapa

all: $(TARGET)

$(TARGET): mapa.c
	$(CC) $(CFLAGS) mapa.c -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(FILAS) $(COLUMNAS)

clean:
	rm -f $(TARGET)
