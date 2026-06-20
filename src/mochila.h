#ifndef MOCHILA_H
#define MOCHILA_H

typedef struct {
    int peso;
    int volume;
    int valor;
} Item;

typedef struct {
    int n;
    int W;
    int V;
    Item *itens;
} Instancia;

typedef struct {
    int valor_maximo;
    int *selecionados;
    double tempo_segundos;
} Resultado;

Instancia *instancia_criar(int n, int W, int V);
void instancia_liberar(Instancia *inst);
int instancia_ler_arquivo(const char *caminho, Instancia **inst);
void instancia_salvar_arquivo(const Instancia *inst, const char *caminho);

static int aleatorio_int(int min, int max);
Instancia *instancia_gerar_aleatoria(int n, int W, int V, unsigned int semente);

static int **alocar_matriz_int(int linhas, int cols);
static void liberar_matriz_int(int **m, int linhas);
static char **alocar_matriz_char(int linhas, int cols);
static void liberar_matriz_char(char **m, int linhas);
void resultado_liberar(Resultado *res);

double tempo_agora(void);

#endif
