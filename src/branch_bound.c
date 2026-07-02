#include "mochila.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned int indice_original;
  unsigned int valor;
  double chave;
} ItemOrdenado;

typedef struct {
  const Instancia *inst;
  ItemOrdenado *ordem_branch;
  ItemOrdenado *ordem_peso;
  ItemOrdenado *ordem_volume;
  unsigned int *posicao_branch;
  unsigned int *solucao_atual;
  Resultado *resultado;
} ContextoBranchBound;

static double densidade(unsigned int valor, double consumo) {
  if (consumo <= 0.0) {
    return valor > 0 ? DBL_MAX : 0.0;
  }
  return (double)valor / consumo;
}

static int comparar_item_ordenado(const void *a, const void *b) {
  const ItemOrdenado *ia = (const ItemOrdenado *)a;
  const ItemOrdenado *ib = (const ItemOrdenado *)b;

  if (ia->chave < ib->chave) {
    return 1;
  }
  if (ia->chave > ib->chave) {
    return -1;
  }
  if (ia->valor < ib->valor) {
    return 1;
  }
  if (ia->valor > ib->valor) {
    return -1;
  }
  if (ia->indice_original > ib->indice_original) {
    return 1;
  }
  if (ia->indice_original < ib->indice_original) {
    return -1;
  }
  return 0;
}

static void preparar_ordens(ContextoBranchBound *ctx) {
  const Instancia *inst = ctx->inst;

  for (unsigned int i = 0; i < inst->n; i++) {
    Item item = inst->itens[i];

    double peso_relativo = inst->W > 0 ? (double)item.peso / inst->W
                                      : (item.peso == 0 ? 0.0 : 1.0);
    double volume_relativo = inst->V > 0 ? (double)item.volume / inst->V
                                        : (item.volume == 0 ? 0.0 : 1.0);

    ctx->ordem_branch[i].indice_original = i;
    ctx->ordem_branch[i].valor = item.valor;
    ctx->ordem_branch[i].chave =
        densidade(item.valor, peso_relativo + volume_relativo);

    ctx->ordem_peso[i].indice_original = i;
    ctx->ordem_peso[i].valor = item.valor;
    ctx->ordem_peso[i].chave = densidade(item.valor, item.peso);

    ctx->ordem_volume[i].indice_original = i;
    ctx->ordem_volume[i].valor = item.valor;
    ctx->ordem_volume[i].chave = densidade(item.valor, item.volume);
  }

  qsort(ctx->ordem_branch, (size_t)inst->n, sizeof(ItemOrdenado),
        comparar_item_ordenado);
  qsort(ctx->ordem_peso, (size_t)inst->n, sizeof(ItemOrdenado),
        comparar_item_ordenado);
  qsort(ctx->ordem_volume, (size_t)inst->n, sizeof(ItemOrdenado),
        comparar_item_ordenado);

  for (unsigned int i = 0; i < inst->n; i++) {
    ctx->posicao_branch[ctx->ordem_branch[i].indice_original] = i;
  }
}

static void gerar_solucao_inicial_gulosa(ContextoBranchBound *ctx) {
  const Instancia *inst = ctx->inst;
  unsigned int peso_total = 0;
  unsigned int volume_total = 0;
  unsigned int valor_total = 0;

  for (unsigned int i = 0; i < inst->n; i++) {
    unsigned int idx = ctx->ordem_branch[i].indice_original;
    Item item = inst->itens[idx];

    if (item.peso <= inst->W - peso_total &&
        item.volume <= inst->V - volume_total) {
      ctx->resultado->selecionados[idx] = 1;
      peso_total += item.peso;
      volume_total += item.volume;
      valor_total += item.valor;
    }
  }

  ctx->resultado->valor_maximo = valor_total;
}

static double limite_fracionario(const ContextoBranchBound *ctx,
                                 const ItemOrdenado *ordem_limite,
                                 unsigned int indice_branch,
                                 unsigned int capacidade_restante,
                                 int usar_volume,
                                 unsigned int valor_atual) {
  const Instancia *inst = ctx->inst;
  double limite = (double)valor_atual;
  double restante = (double)capacidade_restante;

  for (unsigned int i = 0; i < inst->n; i++) {
    unsigned int idx = ordem_limite[i].indice_original;

    if (ctx->posicao_branch[idx] < indice_branch) {
      continue;
    }

    Item item = inst->itens[idx];
    unsigned int consumo = usar_volume ? item.volume : item.peso;

    if (consumo == 0) {
      limite += item.valor;
      continue;
    }

    if ((double)consumo <= restante) {
      limite += item.valor;
      restante -= consumo;
    } else {
      limite += (double)item.valor * (restante / (double)consumo);
      break;
    }
  }

  return limite;
}

static int e_promissora(const ContextoBranchBound *ctx,
                        unsigned int indice_branch,
                        unsigned int peso_atual,
                        unsigned int volume_atual,
                        unsigned int valor_atual) {
  const Instancia *inst = ctx->inst;
  unsigned int peso_restante = inst->W - peso_atual;
  unsigned int volume_restante = inst->V - volume_atual;

  double limite_por_peso =
      limite_fracionario(ctx, ctx->ordem_peso, indice_branch, peso_restante, 0,
                         valor_atual);
  double limite_por_volume =
      limite_fracionario(ctx, ctx->ordem_volume, indice_branch,
                         volume_restante, 1, valor_atual);

  double limite_otimista =
      limite_por_peso < limite_por_volume ? limite_por_peso : limite_por_volume;

  return limite_otimista + 1e-9 > (double)ctx->resultado->valor_maximo;
}

static void atualizar_melhor_solucao(ContextoBranchBound *ctx,
                                     unsigned int valor_atual) {
  if (valor_atual > ctx->resultado->valor_maximo) {
    ctx->resultado->valor_maximo = valor_atual;
    memcpy(ctx->resultado->selecionados, ctx->solucao_atual,
           (size_t)ctx->inst->n * sizeof(unsigned int));
  }
}

static void buscar_branch_bound(ContextoBranchBound *ctx,
                                unsigned int indice_branch,
                                unsigned int peso_atual,
                                unsigned int volume_atual,
                                unsigned int valor_atual) {
  const Instancia *inst = ctx->inst;

  if (peso_atual > inst->W || volume_atual > inst->V) {
    return;
  }

  if (!e_promissora(ctx, indice_branch, peso_atual, volume_atual,
                    valor_atual)) {
    return;
  }

  if (indice_branch == inst->n) {
    atualizar_melhor_solucao(ctx, valor_atual);
    return;
  }

  unsigned int idx = ctx->ordem_branch[indice_branch].indice_original;
  Item item = inst->itens[idx];

  if (item.peso <= inst->W - peso_atual &&
      item.volume <= inst->V - volume_atual) {
    ctx->solucao_atual[idx] = 1;
    buscar_branch_bound(ctx, indice_branch + 1, peso_atual + item.peso,
                        volume_atual + item.volume, valor_atual + item.valor);
  }

  ctx->solucao_atual[idx] = 0;
  buscar_branch_bound(ctx, indice_branch + 1, peso_atual, volume_atual,
                      valor_atual);
}

static void liberar_contexto(ContextoBranchBound *ctx) {
  free(ctx->ordem_branch);
  free(ctx->ordem_peso);
  free(ctx->ordem_volume);
  free(ctx->posicao_branch);
  free(ctx->solucao_atual);
}

/*
 * Branch-and-bound para mochila 0/1 com duas restricoes.
 *
 * Branch: explora a arvore de decisoes em profundidade.
 * Bound: poda ramos cujo limite superior otimista nao supera a melhor solucao.
 *
 * A solucao inicial e gulosa. O limite superior relaxa uma restricao por vez
 * com mochila fracionaria, e usa o menor dos dois limites otimistas.
 *
 * Pior caso: O(n * 2^n), pois a arvore pode ter O(2^n) nos e o limite
 * percorre os itens restantes. Na pratica, as podas reduzem a quantidade
 * de nos visitados. Espaco auxiliar: O(n).
 */
static Resultado *branch_bound_resolver_config(const Instancia *inst,
                                               int usar_solucao_inicial,
                                               const char *descricao) {
  double inicio = tempo_agora();

  printf("Resolvendo instancia com n=%u, W=%u, V=%u usando %s...\n", inst->n,
         inst->W, inst->V, descricao);

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

  ContextoBranchBound ctx;
  ctx.inst = inst;
  ctx.resultado = res;
  ctx.ordem_branch = calloc((size_t)inst->n, sizeof(ItemOrdenado));
  ctx.ordem_peso = calloc((size_t)inst->n, sizeof(ItemOrdenado));
  ctx.ordem_volume = calloc((size_t)inst->n, sizeof(ItemOrdenado));
  ctx.posicao_branch = calloc((size_t)inst->n, sizeof(unsigned int));
  ctx.solucao_atual = calloc((size_t)inst->n, sizeof(unsigned int));

  if (!ctx.ordem_branch || !ctx.ordem_peso || !ctx.ordem_volume ||
      !ctx.posicao_branch || !ctx.solucao_atual) {
    liberar_contexto(&ctx);
    resultado_liberar(res);
    return NULL;
  }

  preparar_ordens(&ctx);
  if (usar_solucao_inicial) {
    gerar_solucao_inicial_gulosa(&ctx);
  }
  buscar_branch_bound(&ctx, 0, 0, 0, 0);

  liberar_contexto(&ctx);
  res->tempo_segundos = tempo_agora() - inicio;
  return res;
}

Resultado *branch_bound_resolver(const Instancia *inst) {
  return branch_bound_resolver_config(inst, 1, "branch-and-bound");
}

Resultado *branch_bound_sem_inicial_resolver(const Instancia *inst) {
  return branch_bound_resolver_config(inst, 0,
                                      "branch-and-bound sem solucao inicial");
}
