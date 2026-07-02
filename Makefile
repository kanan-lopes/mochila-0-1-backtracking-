CC = gcc
CFLAGS = -Wall -Wextra -std=c11
# SRC = src/mochila.c src/generator.c
SRC = src/mochila.c
EXPERIMENTOS_SRC = src/experimentos.c src/mochila.c src/prog_dinamica.c src/backtracking.c src/branch_bound.c

.PHONY: all clean run exemplo experimentos backtracking branch_bound

all: mochila mochila_backtracking mochila_branch_bound experimentos

mochila: src/main.c $(SRC)
	$(CC) $(CFLAGS) -o mochila src/main.c $(SRC)

mochila_backtracking: src/main_backtracking.c $(SRC)
	$(CC) $(CFLAGS) -o mochila_backtracking src/main_backtracking.c $(SRC)

mochila_branch_bound: src/main_branch_bound.c $(SRC)
	$(CC) $(CFLAGS) -o mochila_branch_bound src/main_branch_bound.c $(SRC)

experimentos: $(EXPERIMENTOS_SRC)
	$(CC) $(CFLAGS) -o experimentos $(EXPERIMENTOS_SRC)

run: mochila
	./mochila exemplos/exemplo.txt

exemplo: mochila
	./mochila exemplos/exemplo.txt

clean:
	rm -f mochila mochila_backtracking mochila_branch_bound experimentos resultados_experimentos.csv
