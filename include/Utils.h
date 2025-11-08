#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

// Pipes nomeados
#define FIFO_SERVIDOR "/tmp/fifo_servidor"
#define FIFO_CLIENTE "/tmp/fifo_cliente_%d"

// Constantes
#define MAX_UTILIZADORES 30
#define NVEICULOS 10
#define MAX_USERNAME_TAM 64
#define MAX_RESPOSTA_TAM 128
#define MAX_CHARACTERS 256
#define MAX_ARGUMENTOS 5

// Estruturas
typedef struct
{
    int minutos, horas;
} Hora;

typedef struct
{
    Hora hora;
    char local[MAX_CHARACTERS];
    float distancia;
} Servico;

// Estrutura de mensagem correspondente a um pedido cliente -> servidor
typedef struct
{
    pid_t pid_cliente;
    char fifo_cliente[MAX_CHARACTERS];
    char username[MAX_CHARACTERS];
} Cliente;

// Funções do Cliente
int escolheServico(int fd_servidor, int fd_cliente, Cliente cliente);

// Funções do Controlador
int agendar(Cliente cliente, Servico servico);
int consultar(Cliente cliente, int id);
int cancelar(Cliente cliente);
int filtraPedido(char pedido[], Cliente cliente);
int executarOperacao(char *argumentos[], int n_argumentos, Cliente cliente);

// Funções partilhadas
Servico criarServico(char *hora, char *local, char *distancia);
void criarFIFO(const char *fifo_name);
int abrirFIFO(const char *fifo_name, bool write);