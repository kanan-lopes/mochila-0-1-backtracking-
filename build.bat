@echo off
gcc -Wall -Wextra -O2 -std=c11 -o mochila.exe src/main.c src/mochila.c
gcc -Wall -Wextra -O2 -std=c11 -o mochila_backtracking.exe src/main_backtracking.c src/mochila.c
gcc -Wall -Wextra -O2 -std=c11 -o mochila_branch_bound.exe src/main_branch_bound.c src/mochila.c
gcc -Wall -Wextra -O2 -std=c11 -o experimentos.exe src/experimentos.c src/mochila.c src/prog_dinamica.c src/backtracking.c src/branch_bound.c
