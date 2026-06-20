#include "mochila.h"
#include "prog_dinamica.c"

#include <stdio.h>
#include <stdlib.h>

static void imprimir_resultado(const Instancia *inst, const Resultado *res) {
  int peso_total = 0;
  int volume_total = 0;

  printf("Lucro maximo: %d\n", res->valor_maximo);
  printf("Itens na mochila (indice comeca em 1):\n");

  int algum = 0;
  for (int i = 0; i < inst->n; i++) {
    if (res->selecionados[i]) {
      printf("  Item %d: peso=%d, volume=%d, valor=%d\n", i + 1,
             inst->itens[i].peso, inst->itens[i].volume, inst->itens[i].valor);
      peso_total += inst->itens[i].peso;
      volume_total += inst->itens[i].volume;
      algum = 1;
    }
  }

  if (!algum) {
    printf("  (nenhum item selecionado)\n");
  }

  printf("Peso total: %d / %d\n", peso_total, inst->W);
  printf("Volume total: %d / %d\n", volume_total, inst->V);
  printf("Tempo de execucao: %.6f s\n", res->tempo_segundos);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Uso: %s <arquivo_entrada>\n", argv[0]);
    return 1;
  }

  Instancia *inst = NULL;
  if (instancia_ler_arquivo(argv[1], &inst) != 0) {
    fprintf(stderr, "Erro ao ler arquivo: %s\n", argv[1]);
    return 1;
  }

  Resultado *res = prog_mochila_resolver(inst);
  if (!res) {
    fprintf(stderr, "Erro ao resolver instancia.\n");
    instancia_liberar(inst);
    return 1;
  }

  imprimir_resultado(inst, res);

  resultado_liberar(res);
  instancia_liberar(inst);
  return 0;
}
