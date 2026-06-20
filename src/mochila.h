#ifndef MOCHILA_H
#define MOCHILA_H

typedef struct {
    unsigned int peso;
    unsigned int volume;
    unsigned int valor;
} Item;

/**
 * @struct Instancia
 * @brief Representa uma instância do problema da mochila 0/1.
 * 
 * Contém os parâmetros do problema e os itens para um problema de mochila 0/1. 
 */
typedef struct {
    unsigned int n;       /**< Número de itens */
    unsigned int W;       /**< Capacidade de peso da mochila */
    unsigned int V;       /**< Capacidade de volume da mochila */
    Item *itens; /**< Array de itens na instância */
} Instancia;

typedef struct {
    unsigned int valor_maximo;
    unsigned int *selecionados;
    double tempo_segundos;
} Resultado;

void imprimir_resultado(const Instancia *inst, const Resultado *res);

Instancia *instancia_criar(unsigned int n, unsigned int W, unsigned int V);
void instancia_liberar(Instancia *inst);
int instancia_ler_arquivo(const char *caminho, Instancia **inst);
void instancia_salvar_arquivo(const Instancia *inst, const char *caminho);

unsigned int aleatorio_int(unsigned int min, unsigned int max);
Instancia *instancia_gerar_aleatoria(unsigned int n, unsigned int W, unsigned int V, unsigned int semente);

int **alocar_matriz_int(int linhas, int cols);
void liberar_matriz_int(int **m, int linhas);
char **alocar_matriz_char(int linhas, int cols);
void liberar_matriz_char(char **m, int linhas);
void resultado_liberar(Resultado *res);

double tempo_agora(void);

unsigned int ***alocar_matriz3d_uint(int d1, int d2, int d3);
void liberar_matriz3d_uint(unsigned int ***m, int d1, int d2);

#endif
