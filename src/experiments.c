#include "mochila.h"
#include "prog_dinamica.c"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int n;
  int W;
  int V;
} ConfigExperimento;

static const ConfigExperimento CONFIGS[] = {
    {10, 20, 20},    {10, 50, 50},    {10, 100, 100},    {20, 40, 40},
    {20, 100, 100},  {20, 200, 200},  {30, 60, 60},      {30, 150, 150},
    {30, 300, 300},  {40, 80, 80},    {40, 200, 200},    {50, 100, 100},
    {50, 250, 250},  {50, 500, 500},  {75, 150, 150},    {75, 375, 375},
    {100, 200, 200}, {100, 500, 500}, {100, 1000, 1000},
};

static const int NUM_CONFIGS = sizeof(CONFIGS) / sizeof(CONFIGS[0]);
static const int INSTANCIAS_POR_CONFIG = 10;

int main(void) {
  const char *saida = "resultados_experimentos.csv";
  FILE *f = fopen(saida, "w");
  if (!f) {
    fprintf(stderr, "Erro ao criar %s\n", saida);
    return 1;
  }

  fprintf(f, "n,W,V,instancia,tempo_s,valor_maximo,complexidade_nWV\n");

  printf("Executando experimentos (%d configuracoes x %d instancias)...\n",
         NUM_CONFIGS, INSTANCIAS_POR_CONFIG);

  for (int c = 0; c < NUM_CONFIGS; c++) {
    int n = CONFIGS[c].n;
    int W = CONFIGS[c].W;
    int V = CONFIGS[c].V;
    long long complexidade = (long long)n * (long long)W * (long long)V;

    double tempo_total = 0.0;
    double tempo_min = 1e30;
    double tempo_max = 0.0;

    printf("  n=%d W=%d V=%d (n*W*V=%lld) ... ", n, W, V, complexidade);

    for (int k = 0; k < INSTANCIAS_POR_CONFIG; k++) {
      unsigned int semente = (unsigned int)(c * 100 + k + 1);
      Instancia *inst = instancia_gerar_aleatoria(n, W, V, semente);
      if (!inst) {
        fprintf(stderr, "Erro ao gerar instancia.\n");
        continue;
      }

      Resultado *res = mochila_resolver(inst);
      if (!res) {
        instancia_liberar(inst);
        continue;
      }

      double t = res->tempo_segundos;
      tempo_total += t;
      if (t < tempo_min) {
        tempo_min = t;
      }
      if (t > tempo_max) {
        tempo_max = t;
      }

      fprintf(f, "%d,%d,%d,%d,%.9f,%d,%lld\n", n, W, V, k + 1, t,
              res->valor_maximo, complexidade);

      resultado_liberar(res);
      instancia_liberar(inst);
    }

    double tempo_medio = tempo_total / INSTANCIAS_POR_CONFIG;
    printf("medio=%.6f s (min=%.6f, max=%.6f)\n", tempo_medio, tempo_min,
           tempo_max);
  }

  fclose(f);
  printf("\nResultados salvos em: %s\n", saida);
  printf("\nResumo do comportamento assintotico:\n");
  printf("- O algoritmo tem complexidade O(n * W * V).\n");
  printf("- Aumentar n, W ou V aumenta o tempo proporcionalmente.\n");
  printf("- Compare tempo_medio vs n*W*V no CSV para verificar linearidade.\n");

  return 0;
}
