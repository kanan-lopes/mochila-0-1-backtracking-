#include "mochila.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_INSTANCIAS_POR_CONFIG 10
#define DEFAULT_LIMITE_BACKTRACKING 25
#define DEFAULT_LIMITE_BRANCH_BOUND 35
#define MAX_ALGORITMOS 4

typedef struct {
  unsigned int n;
  unsigned int W;
  unsigned int V;
} ConfigExperimento;

typedef struct {
  const char *nome;
  Resultado *(*resolver)(const Instancia *inst);
  unsigned int limite_n;
  const char *motivo_limite;
} AlgoritmoExperimento;

typedef struct {
  double tempo_total;
  double tempo_min;
  double tempo_max;
  unsigned int executados;
  unsigned int ignorados;
} ResumoAlgoritmo;

static const ConfigExperimento CONFIGS[] = {
    {10, 20, 20},     {10, 20, 50},     {10, 50, 20},
    {10, 100, 100},   {20, 40, 40},     {20, 40, 100},
    {20, 100, 40},    {20, 200, 200},   {30, 60, 60},
    {30, 60, 150},    {30, 150, 60},    {30, 300, 300},
    {40, 80, 80},     {40, 80, 200},    {40, 200, 80},
    {40, 200, 200},   {50, 100, 100},   {50, 100, 250},
    {50, 250, 100},   {50, 500, 500},   {75, 150, 150},
    {75, 150, 375},   {75, 375, 150},   {75, 375, 375},
    {100, 200, 200},  {100, 200, 500},  {100, 500, 200},
    {100, 1000, 1000},
};

static const unsigned int NUM_CONFIGS = sizeof(CONFIGS) / sizeof(CONFIGS[0]);

static int ler_env_int(const char *nome, int padrao, int minimo, int maximo) {
  const char *valor = getenv(nome);
  if (!valor || valor[0] == '\0') {
    return padrao;
  }

  int convertido = atoi(valor);
  if (convertido < minimo) {
    return minimo;
  }
  if (convertido > maximo) {
    return maximo;
  }
  return convertido;
}

static void resumo_inicializar(ResumoAlgoritmo *resumo) {
  resumo->tempo_total = 0.0;
  resumo->tempo_min = DBL_MAX;
  resumo->tempo_max = 0.0;
  resumo->executados = 0;
  resumo->ignorados = 0;
}

static void resumo_registrar_execucao(ResumoAlgoritmo *resumo, double tempo) {
  resumo->tempo_total += tempo;
  if (tempo < resumo->tempo_min) {
    resumo->tempo_min = tempo;
  }
  if (tempo > resumo->tempo_max) {
    resumo->tempo_max = tempo;
  }
  resumo->executados++;
}

static void escrever_linha_ignorada(FILE *f, const char *algoritmo,
                                    const ConfigExperimento *cfg,
                                    unsigned int instancia,
                                    unsigned int semente,
                                    long long complexidade,
                                    const char *observacao) {
  fprintf(f, "%s,%u,%u,%u,%u,%u,0,-1.000000000,0,-1,%lld,%s\n",
          algoritmo, cfg->n, cfg->W, cfg->V, instancia, semente, complexidade,
          observacao);
}

static int executar_algoritmo(FILE *f, const AlgoritmoExperimento *alg,
                              const ConfigExperimento *cfg,
                              const Instancia *inst, unsigned int instancia,
                              unsigned int semente, long long complexidade,
                              int tem_referencia, unsigned int referencia,
                              unsigned int *valor_obtido,
                              ResumoAlgoritmo *resumo) {
  if (alg->limite_n > 0 && cfg->n > alg->limite_n) {
    escrever_linha_ignorada(f, alg->nome, cfg, instancia, semente, complexidade,
                            alg->motivo_limite);
    resumo->ignorados++;
    return 0;
  }

  Resultado *res = alg->resolver(inst);
  if (!res) {
    escrever_linha_ignorada(f, alg->nome, cfg, instancia, semente, complexidade,
                            "erro_execucao");
    resumo->ignorados++;
    return 0;
  }

  int coerente = -1;
  if (tem_referencia) {
    coerente = res->valor_maximo == referencia ? 1 : 0;
  }

  fprintf(f, "%s,%u,%u,%u,%u,%u,1,%.9f,%u,%d,%lld,ok\n", alg->nome, cfg->n,
          cfg->W, cfg->V, instancia, semente, res->tempo_segundos,
          res->valor_maximo, coerente, complexidade);

  if (valor_obtido) {
    *valor_obtido = res->valor_maximo;
  }

  if (coerente == 0) {
    fprintf(stderr,
            "Aviso: %s divergente em n=%u W=%u V=%u instancia=%u "
            "(valor=%u referencia=%u)\n",
            alg->nome, cfg->n, cfg->W, cfg->V, instancia, res->valor_maximo,
            referencia);
  }

  resumo_registrar_execucao(resumo, res->tempo_segundos);
  resultado_liberar(res);
  return 1;
}

static void imprimir_resumo_config(const AlgoritmoExperimento *algoritmos,
                                   const ResumoAlgoritmo *resumos,
                                   unsigned int qtd_algoritmos) {
  for (unsigned int i = 0; i < qtd_algoritmos; i++) {
    if (resumos[i].executados == 0) {
      printf("    %-21s executados=0 ignorados=%u\n", algoritmos[i].nome,
             resumos[i].ignorados);
      continue;
    }

    double media = resumos[i].tempo_total / resumos[i].executados;
    printf("    %-21s media=%.6f s min=%.6f max=%.6f executados=%u "
           "ignorados=%u\n",
           algoritmos[i].nome, media, resumos[i].tempo_min,
           resumos[i].tempo_max, resumos[i].executados,
           resumos[i].ignorados);
  }
}

int main(void) {
  const char *saida = getenv("MOCHILA_SAIDA");
  if (!saida || saida[0] == '\0') {
    saida = "resultados_experimentos.csv";
  }

  unsigned int instancias_por_config =
      (unsigned int)ler_env_int("MOCHILA_INSTANCIAS",
                                DEFAULT_INSTANCIAS_POR_CONFIG, 1, 1000);
  unsigned int limite_backtracking =
      (unsigned int)ler_env_int("MOCHILA_LIMITE_BACKTRACKING",
                                DEFAULT_LIMITE_BACKTRACKING, 0, 1000000);
  unsigned int limite_branch_bound =
      (unsigned int)ler_env_int("MOCHILA_LIMITE_BRANCH_BOUND",
                                DEFAULT_LIMITE_BRANCH_BOUND, 0, 1000000);
  unsigned int testar_bb_sem_inicial =
      (unsigned int)ler_env_int("MOCHILA_TESTAR_BB_SEM_INICIAL", 0, 0, 1);
  unsigned int max_configs =
      (unsigned int)ler_env_int("MOCHILA_MAX_CONFIGS", (int)NUM_CONFIGS, 1,
                                (int)NUM_CONFIGS);
  unsigned int config_inicio =
      (unsigned int)ler_env_int("MOCHILA_CONFIG_INICIO", 1, 1,
                                (int)NUM_CONFIGS);
  unsigned int primeiro_indice = config_inicio - 1;
  unsigned int ultimo_indice = primeiro_indice + max_configs;
  if (ultimo_indice > NUM_CONFIGS) {
    ultimo_indice = NUM_CONFIGS;
  }
  unsigned int configs_executadas = ultimo_indice - primeiro_indice;

  AlgoritmoExperimento algoritmos[MAX_ALGORITMOS] = {
      {"programacao_dinamica", prog_dinamica_resolver, 0, "sem_limite"},
      {"backtracking", backtracking_resolver, limite_backtracking,
       "limite_n_backtracking"},
      {"branch_bound", branch_bound_resolver, limite_branch_bound,
       "limite_n_branch_bound"},
      {"branch_bound_sem_inicial", branch_bound_sem_inicial_resolver,
       limite_branch_bound, "limite_n_branch_bound"},
  };

  const unsigned int qtd_algoritmos = testar_bb_sem_inicial ? 4 : 3;

  FILE *f = fopen(saida, "w");
  if (!f) {
    fprintf(stderr, "Erro ao criar %s\n", saida);
    return 1;
  }

  fprintf(f, "algoritmo,n,W,V,instancia,semente,executado,tempo_s,"
             "valor_maximo,coerente,complexidade_nWV,observacao\n");

  printf("Executando experimentos (%u configuracoes x %u instancias).\n",
         configs_executadas, instancias_por_config);
  printf("Intervalo de configuracoes: %u ate %u.\n", config_inicio,
         ultimo_indice);
  printf("Limites: backtracking n<=%u, branch_bound n<=%u.\n",
         limite_backtracking, limite_branch_bound);
  printf("Branch-bound sem solucao inicial: %s.\n",
         testar_bb_sem_inicial ? "ativado" : "desativado");
  printf("Saida CSV: %s\n\n", saida);

  for (unsigned int c = primeiro_indice; c < ultimo_indice; c++) {
    ConfigExperimento cfg = CONFIGS[c];
    long long complexidade =
        (long long)cfg.n * (long long)cfg.W * (long long)cfg.V;
    ResumoAlgoritmo resumos[MAX_ALGORITMOS];

    for (unsigned int i = 0; i < qtd_algoritmos; i++) {
      resumo_inicializar(&resumos[i]);
    }

    printf("Configuracao %u/%u: n=%u W=%u V=%u (n*W*V=%lld)\n", c + 1,
           NUM_CONFIGS, cfg.n, cfg.W, cfg.V, complexidade);

    for (unsigned int k = 0; k < instancias_por_config; k++) {
      unsigned int semente = c * 100 + k + 1;
      Instancia *inst =
          instancia_gerar_aleatoria(cfg.n, cfg.W, cfg.V, semente);
      if (!inst) {
        fprintf(stderr, "Erro ao gerar instancia %u da configuracao %u.\n",
                k + 1, c + 1);
        continue;
      }

      unsigned int referencia = 0;
      int tem_referencia = executar_algoritmo(
          f, &algoritmos[0], &cfg, inst, k + 1, semente, complexidade, 0, 0,
          &referencia, &resumos[0]);

      for (unsigned int a = 1; a < qtd_algoritmos; a++) {
        executar_algoritmo(f, &algoritmos[a], &cfg, inst, k + 1, semente,
                           complexidade, tem_referencia, referencia, NULL,
                           &resumos[a]);
      }

      instancia_liberar(inst);
    }

    imprimir_resumo_config(algoritmos, resumos, qtd_algoritmos);
    printf("\n");
  }

  fclose(f);

  printf("Resultados salvos em: %s\n", saida);
  printf("Campos importantes do CSV:\n");
  printf("- executado=0 indica algoritmo ignorado por limite ou erro.\n");
  printf("- coerente=1 indica mesmo valor maximo da programacao dinamica.\n");
  printf("- coerente=-1 indica que nao havia referencia para comparar.\n");

  return 0;
}
