#include "mochila.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Programação dinâmica 3D: dp[i][w][v] = lucro máximo usando itens 0..i-1
 * com capacidade de peso w e volume v.
 * Complexidade: O(n * W * V) tempo e espaço.
 */
Resultado *prog_dinamica_resolver(const Instancia *inst) {
  double inicio = tempo_agora();

  printf("Resolvendo instancia com n=%u, W=%u, V=%u usando programacao dinamica...\n",
         inst->n, inst->W, inst->V);

  Resultado *res = calloc(1, sizeof(Resultado));
  if (!res) {
    return NULL;
  }

  int n = inst->n;
  int W = inst->W;
  int V = inst->V;

  // Aloca o vetor de itens selecionados (0 ou 1 para cada item)
  // e inicializa com 0 (nenhum item selecionado)
  res->selecionados = calloc((size_t)n, sizeof(unsigned int));
  if (!res->selecionados) {
    free(res);
    return NULL;
  }

  if (n == 0) {
    res->valor_maximo = 0;
    res->tempo_segundos = 0.0;
    return res;
  }

  // Aloca a matriz 3D para programação dinâmica
  // Sendo as dimensões: (n+1) x (W+1) x (V+1)
  unsigned int ***tabela_dp = alocar_matriz3d_uint(n + 1, W + 1, V + 1);
  if (!tabela_dp) {
    resultado_liberar(res);
    return NULL;
  }

  // Inicializa a tabela com zeros
  for (int i = 0; i <= n; i++) {
    for (int w = 0; w <= W; w++) {
      for (int v = 0; v <= V; v++) {
        tabela_dp[i][w][v] = 0;
      }
    }
  }

  // Preenche a tabela de programação dinâmica
  for (int i = 1; i <=n; i++) {
    // Ajustando os indices para acessar o item correto
    int peso = inst->itens[i - 1].peso;
    int volume = inst->itens[i - 1].volume;
    int valor = inst->itens[i - 1].valor;

    // Itera sobre todas as capacidades de peso e volume
    for (int w = 0; w <= W; w++) {
      for (int v = 0; v <= V; v++) {
        // Verifica se o item atual pode ser incluido na mochila
        if (peso <= w && volume <= v) {
          // Escolhe o maximo entre incluir ou nao incluir o item
          unsigned int incluir = valor + tabela_dp[i - 1][w - peso][v - volume];
          unsigned int nao_incluir = tabela_dp[i - 1][w][v];
          tabela_dp[i][w][v] = (incluir > nao_incluir) ? incluir : nao_incluir;
        } else {
          // Nao pode incluir o item, entao apenas copia o valor da pagina anterior
          tabela_dp[i][w][v] = tabela_dp[i - 1][w][v];
        }
      }
    }
  }

  // Registra o resultado dos selecionados
  int w = W;
  int v = V;

  for (int i = n; i > 0; i--) {
    if (tabela_dp[i][w][v] != tabela_dp[i - 1][w][v]) {
      res->selecionados[i - 1] = 1;
      w -= inst->itens[i - 1].peso;
      v -= inst->itens[i - 1].volume;
    } else {
      res->selecionados[i - 1] = 0;
    }
  }

  res->valor_maximo = tabela_dp[n][W][V];
  res->tempo_segundos = tempo_agora() - inicio;

  liberar_matriz3d_uint(tabela_dp, n + 1, W + 1);

  return res;
}
