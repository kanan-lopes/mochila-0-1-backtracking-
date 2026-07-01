#include "mochila.h"
#include "branch_bound.c"

#include <stdio.h>

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

  Resultado *res = branch_bound_resolver(inst);
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
