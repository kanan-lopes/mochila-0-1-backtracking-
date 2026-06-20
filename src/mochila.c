#include "mochila.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

double tempo_agora(void) {
#ifdef _WIN32
  static LARGE_INTEGER freq = {0};
  LARGE_INTEGER count;
  if (freq.QuadPart == 0) {
    QueryPerformanceFrequency(&freq);
  }
  QueryPerformanceCounter(&count);
  return (double)count.QuadPart / (double)freq.QuadPart;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec / 1e-9;
#endif
}

void imprimir_resultado(const Instancia *inst, const Resultado *res) {
  int peso_total = 0;
  int volume_total = 0;

  printf("Lucro maximo: %d\n", res->valor_maximo);
  printf("Itens na mochila (indice comeca em 1):\n");

  int algum = 0;
  for (unsigned int i = 0; i < inst->n; i++) {
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

Instancia *instancia_criar(unsigned int n, unsigned int W, unsigned int V) {
  Instancia *inst = malloc(sizeof(Instancia));
  if (!inst) {
    return NULL;
  }
  inst->n = n;
  inst->W = W;
  inst->V = V;
  inst->itens = calloc((size_t)n, sizeof(Item));
  if (!inst->itens) {
    free(inst);
    return NULL;
  }
  return inst;
}

void instancia_liberar(Instancia *inst) {
  if (!inst) {
    return;
  }
  free(inst->itens);
  free(inst);
}

int instancia_ler_arquivo(const char *caminho, Instancia **inst_out) {
  FILE *f = fopen(caminho, "r");
  if (!f) {
    return -1;
  }

  int W, V;
  if (fscanf(f, "%d %d", &W, &V) != 2) {
    fclose(f);
    return -1;
  }

  int capacidade = 64;
  int n = 0;
  Item *buffer = malloc((size_t)capacidade * sizeof(Item));
  if (!buffer) {
    fclose(f);
    return -1;
  }

  while (1) {
    int peso, volume, valor;
    int lidos = fscanf(f, "%d %d %d", &peso, &volume, &valor);
    if (lidos == EOF) {
      break;
    }
    if (lidos != 3) {
      free(buffer);
      fclose(f);
      return -1;
    }
    if (n >= capacidade) {
      capacidade *= 2;
      Item *novo = realloc(buffer, (size_t)capacidade * sizeof(Item));
      if (!novo) {
        free(buffer);
        fclose(f);
        return -1;
      }
      buffer = novo;
    }
    buffer[n].peso = peso;
    buffer[n].volume = volume;
    buffer[n].valor = valor;
    n++;
  }

  fclose(f);

  Instancia *inst = instancia_criar(n, W, V);
  if (!inst) {
    free(buffer);
    return -1;
  }
  memcpy(inst->itens, buffer, (size_t)n * sizeof(Item));
  free(buffer);

  *inst_out = inst;
  return 0;
}

void instancia_salvar_arquivo(const Instancia *inst, const char *caminho) {
  FILE *f = fopen(caminho, "w");
  if (!f) {
    return;
  }
  fprintf(f, "%d %d\n", inst->W, inst->V);
  for (unsigned int i = 0; i < inst->n; i++) {
    fprintf(f, "%d\t%d\t%d\n", inst->itens[i].peso, inst->itens[i].volume,
            inst->itens[i].valor);
  }
  fclose(f);
}

unsigned int aleatorio_int(unsigned int min, unsigned int max) {
  if (max < min) {
    return min;
  }
  return min + (rand() % (max - min + 1));
}

/*
 * Gera instância aleatória: pesos e volumes entre 1 e max(capacidade/2, 1),
 * valores entre 1 e 100.
 */
Instancia *instancia_gerar_aleatoria(unsigned int n, unsigned int W, unsigned int V,
                                     unsigned int semente) {
  srand(semente);

  Instancia *inst = instancia_criar(n, W, V);
  if (!inst) {
    return NULL;
  }

  unsigned int max_peso = W / 2;
  if (max_peso < 1) {
    max_peso = 1;
  }
  unsigned int max_vol = V / 2;
  if (max_vol < 1) {
    max_vol = 1;
  }

  for (unsigned int i = 0; i < n; i++) {
    inst->itens[i].peso = aleatorio_int(1, max_peso);
    inst->itens[i].volume = aleatorio_int(1, max_vol);
    inst->itens[i].valor = aleatorio_int(1, 100);
  }

  return inst;
}

int **alocar_matriz_int(int linhas, int cols) {
  int **m = malloc((size_t)linhas * sizeof(int *));
  if (!m) {
    return NULL;
  }
  for (int i = 0; i < linhas; i++) {
    m[i] = calloc((size_t)cols, sizeof(int));
    if (!m[i]) {
      for (int j = 0; j < i; j++) {
        free(m[j]);
      }
      free(m);
      return NULL;
    }
  }
  return m;
}

void liberar_matriz_int(int **m, int linhas) {
  if (!m) {
    return;
  }
  for (int i = 0; i < linhas; i++) {
    free(m[i]);
  }
  free(m);
}

char **alocar_matriz_char(int linhas, int cols) {
  char **m = malloc((size_t)linhas * sizeof(char *));
  if (!m) {
    return NULL;
  }
  for (int i = 0; i < linhas; i++) {
    m[i] = calloc((size_t)cols, sizeof(char));
    if (!m[i]) {
      for (int j = 0; j < i; j++) {
        free(m[j]);
      }
      free(m);
      return NULL;
    }
  }
  return m;
}

void liberar_matriz_char(char **m, int linhas) {
  if (!m) {
    return;
  }
  for (int i = 0; i < linhas; i++) {
    free(m[i]);
  }
  free(m);
}

void resultado_liberar(Resultado *res) {
  if (!res) {
    return;
  }
  free(res->selecionados);
  free(res);
}

unsigned int ***alocar_matriz3d_uint(int d1, int d2, int d3) {
  unsigned int ***m = malloc((size_t)d1 * sizeof(unsigned int **));
  if (!m) {
    return NULL;
  }
  for (int i = 0; i < d1; i++) {
    m[i] = malloc((size_t)d2 * sizeof(unsigned int *));
    if (!m[i]) {
      for (int k = 0; k < i; k++) {
        for (int j = 0; j < d2; j++) {
          free(m[k][j]);
        }
        free(m[k]);
      }
      free(m);
      return NULL;
    }
    for (int j = 0; j < d2; j++) {
      m[i][j] = calloc((size_t)d3, sizeof(unsigned int));
      if (!m[i][j]) {
        for (int k = 0; k < j; k++) {
          free(m[i][k]);
        }
        free(m[i]);
        for (int k = 0; k < i; k++) {
          for (int l = 0; l < d2; l++) {
            free(m[k][l]);
          }
          free(m[k]);
        }
        free(m);
        return NULL;
      }
    }
  }
  return m;
}

void liberar_matriz3d_uint(unsigned int ***m, int d1, int d2) {
  if (!m) {
    return;
  }
  for (int i = 0; i < d1; i++) {
    for (int j = 0; j < d2; j++) {
      free(m[i][j]);
    }
    free(m[i]);
  }
  free(m);
}

