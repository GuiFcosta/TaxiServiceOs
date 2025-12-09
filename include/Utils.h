#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

// Pipes nomeados
#define FIFO_SERVIDOR "/tmp/fifo_servidor"
#define FIFO_CLIENTE "/tmp/fifo_cliente_%d"

// Constantes
#define MAX_UTILIZADORES 30
#define NVEICULOS 10
#define MAX_USERNAME_TAM 64
#define MAX_CHARACTERS 256
#define MAX_ARGUMENTOS 5
#define REGISTADO "registado"
#define NEGADO "negado"

// Estruturas
typedef struct
{
    int minutos, horas;
} Hora;

typedef struct
{
    Hora hora;
    char local[128];
    float distancia;
} Servico;

// Estrutura de mensagem correspondente a um pedido cliente -> servidor
typedef struct
{
    pid_t pid_cliente;
    char fifo_cliente[MAX_CHARACTERS];
    char username[MAX_USERNAME_TAM];
} Cliente;

typedef struct
{
    pid_t pid_cliente;
    char comando[MAX_CHARACTERS];
} Pedido;

// Funções do Cliente
void* thread_recebe_mensagens(void* arg);
void* thread_envia_pedidos(void* arg);

// Funções do Controlador
int agendar(Cliente cliente, Servico servico);
int consultar(Cliente cliente, int id);
int cancelar(Cliente cliente);
int filtraPedido(char pedido[], Cliente cliente);
int executarOperacao(char *argumentos[], int n_argumentos, Cliente cliente);
void* thread_gestao_comandos(void* arg);

// Funções partilhadas
Servico criarServico(char *hora, char *local, char *distancia);
void criarFIFO(const char *fifo_name);
int abrirFIFO(const char *fifo_name, bool write);