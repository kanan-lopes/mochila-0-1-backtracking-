#include "mochila.h"

#include <stdlib.h>
/*
 * Programação dinâmica 3D: dp[i][w][v] = lucro máximo usando itens 0..i-1
 * com capacidade de peso w e volume v.
 * Complexidade: O(n * W * V) tempo e espaço.
 */
Resultado *prog_dinamica_resolver(const Instancia *inst) {
  Resultado *res = calloc(1, sizeof(Resultado));
  if (!res) {
    return NULL;
  }

  int n = inst->n;
  int W = inst->W;
  int V = inst->V;

  res->selecionados = calloc((size_t)n, sizeof(int));
  if (!res->selecionados) {
    free(res);
    return NULL;
  }

  if (n == 0) {
    res->valor_maximo = 0;
    res->tempo_segundos = 0.0;
    return res;
  }

  int linhas = W + 1;
  int cols = V + 1;

  int **dp = alocar_matriz_int(linhas, cols);
  char **escolha = alocar_matriz_char(n, linhas * cols);
  if (!dp || !escolha) {
    liberar_matriz_int(dp, linhas);
    liberar_matriz_char(escolha, n);
    free(res->selecionados);
    free(res);
    return NULL;
  }

  double inicio = tempo_agora();

  int **dp_ant = alocar_matriz_int(linhas, cols);
  if (!dp_ant) {
    liberar_matriz_int(dp, linhas);
    liberar_matriz_char(escolha, n);
    free(res->selecionados);
    free(res);
    return NULL;
  }

  for (int i = 0; i < n; i++) {
    int pi = inst->itens[i].peso;
    int vi = inst->itens[i].volume;
    int val = inst->itens[i].valor;

    for (int w = 0; w <= W; w++) {
      memcpy(dp_ant[w], dp[w], (size_t)cols * sizeof(int));
    }

    for (int w = 0; w <= W; w++) {
      for (int v = 0; v <= V; v++) {
        int melhor = dp_ant[w][v];
        char tomou = 0;

        if (w >= pi && v >= vi) {
          int candidato = dp_ant[w - pi][v - vi] + val;
          if (candidato > melhor) {
            melhor = candidato;
            tomou = 1;
          }
        }

        dp[w][v] = melhor;
        escolha[i][w * cols + v] = tomou;
      }
    }
  }

  liberar_matriz_int(dp_ant, linhas);

  res->tempo_segundos = tempo_agora() - inicio;
  res->valor_maximo = dp[W][V];

  int w = W;
  int v = V;
  for (int i = n - 1; i >= 0; i--) {
    int idx = w * cols + v;
    if (escolha[i][idx]) {
      res->selecionados[i] = 1;
      w -= inst->itens[i].peso;
      v -= inst->itens[i].volume;
    }
  }

  liberar_matriz_int(dp, linhas);
  liberar_matriz_char(escolha, n);

  return res;
}
