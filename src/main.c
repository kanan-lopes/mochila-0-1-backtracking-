#include "mochila.h"
#include "prog_dinamica.c"

#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {
  // Verifica os argumentos da linha de comando: deve ser o nome do arquivo de entrada
  if (argc < 2) {
    fprintf(stderr, "Uso: %s <arquivo_entrada>\n", argv[0]);
    return 1;
  }

  // Tenta ler a intancia do arquivo
  Instancia *inst = NULL;
  if (instancia_ler_arquivo(argv[1], &inst) != 0) {
    fprintf(stderr, "Erro ao ler arquivo: %s\n", argv[1]);
    return 1;
  }

  // Resolve a instancia usando programacao dinamica
  Resultado *res = prog_dinamica_resolver(inst);
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
