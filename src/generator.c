#include "mochila.h"

#include <stdlib.h>

int aleatorio_int(int min, int max) {
    if (max < min) {
        return min;
    }
    // Gera um numero aleatorio 
    return min + (rand() % (max - min + 1));
}

/*
 * Gera instância aleatória: pesos e volumes entre 1 e max(capacidade/2, 1),
 * valores entre 1 e 100.
 */
Instancia *instancia_gerar_aleatoria(int n, int W, int V, unsigned int semente) {
    srand(semente);

    Instancia *inst = instancia_criar(n, W, V);
    if (!inst) {
        return NULL;
    }

    int max_peso = W / 2;
    if (max_peso < 1) {
        max_peso = 1;
    }
    int max_vol = V / 2;
    if (max_vol < 1) {
        max_vol = 1;
    }

    for (int i = 0; i < n; i++) {
        inst->itens[i].peso = aleatorio_int(1, max_peso);
        inst->itens[i].volume = aleatorio_int(1, max_vol);
        inst->itens[i].valor = aleatorio_int(1, 100);
    }

    return inst;
}
