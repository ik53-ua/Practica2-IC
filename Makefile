CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm

# Caso de referencia: 8000x8000 para que tarde varios segundos
FILAS = 8000
COLUMNAS = 8000

TARGET = generador_mapa

all: $(TARGET)

$(TARGET): mapa.c
	$(CC) $(CFLAGS) mapa.c -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) $(FILAS) $(COLUMNAS)

clean:
	rm -f $(TARGET)

