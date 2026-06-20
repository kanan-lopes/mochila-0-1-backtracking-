@echo off
gcc -Wall -Wextra -O2 -std=c11 -o mochila.exe src/main.c src/mochila.c src/generator.c
gcc -Wall -Wextra -O2 -std=c11 -o experimentos.exe src/experiments.c src/mochila.c src/generator.c
