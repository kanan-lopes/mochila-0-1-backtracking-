#include "mochila.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const Instancia *inst;
  unsigned int *solucao_atual;
  Resultado *resultado;
} ContextoBacktracking;

static void atualizar_melhor_solucao(ContextoBacktracking *ctx,
                                     unsigned int valor_atual) {
  if (valor_atual > ctx->resultado->valor_maximo) {
    ctx->resultado->valor_maximo = valor_atual;
    memcpy(ctx->resultado->selecionados, ctx->solucao_atual,
           (size_t)ctx->inst->n * sizeof(unsigned int));
  }
}

static void buscar_backtracking(ContextoBacktracking *ctx, unsigned int indice,
                                unsigned int peso_atual,
                                unsigned int volume_atual,
                                unsigned int valor_atual) {
  const Instancia *inst = ctx->inst;

  if (peso_atual > inst->W || volume_atual > inst->V) {
    return;
  }

  if (indice == inst->n) {
    atualizar_melhor_solucao(ctx, valor_atual);
    return;
  }

  Item item = inst->itens[indice];

  if (item.peso <= inst->W - peso_atual &&
      item.volume <= inst->V - volume_atual) {
    ctx->solucao_atual[indice] = 1;
    buscar_backtracking(ctx, indice + 1, peso_atual + item.peso,
                        volume_atual + item.volume, valor_atual + item.valor);
  }

  ctx->solucao_atual[indice] = 0;
  buscar_backtracking(ctx, indice + 1, peso_atual, volume_atual, valor_atual);
}

/*
 * Backtracking para mochila 0/1 com duas restricoes.
 *
 * Cada nivel da recursao decide se o item atual entra ou nao na mochila.
 * Ramos que violam peso ou volume sao descartados antes de continuar a busca.
 *
 * Complexidade no pior caso: O(2^n) tempo e O(n) espaco auxiliar.
 */
Resultado *backtracking_resolver(const Instancia *inst) {
  double inicio = tempo_agora();

  printf("Resolvendo instancia com n=%u, W=%u, V=%u usando backtracking...\n",
         inst->n, inst->W, inst->V);

  Resultado *res = calloc(1, sizeof(Resultado));
  if (!res) {
    return NULL;
  }

  if (inst->n == 0) {
    res->tempo_segundos = tempo_agora() - inicio;
    return res;
  }

  res->selecionados = calloc((size_t)inst->n, sizeof(unsigned int));
  if (!res->selecionados) {
    free(res);
    return NULL;
  }

  unsigned int *solucao_atual = calloc((size_t)inst->n, sizeof(unsigned int));
  if (!solucao_atual) {
    resultado_liberar(res);
    return NULL;
  }

  ContextoBacktracking ctx = {inst, solucao_atual, res};
  buscar_backtracking(&ctx, 0, 0, 0, 0);

  free(solucao_atual);
  res->tempo_segundos = tempo_agora() - inicio;
  return res;
}
