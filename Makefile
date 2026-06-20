CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = src/mochila.c src/generator.c

.PHONY: all clean run exemplo experimentos

all: mochila experimentos

mochila: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o mochila src/main.c $(SRC)

experimentos: src/experiments.c $(SRC)
	$(CC) $(CFLAGS) -o experimentos src/experiments.c $(SRC)

run: mochila
	./mochila exemplos/exemplo.txt

exemplo: mochila
	./mochila exemplos/exemplo.txt

clean:
	rm -f mochila experimentos resultados_experimentos.csv
