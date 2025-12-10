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
#define FIFO_VEICULO "/tmp/fifo_veiculo_%d"

// Constantes
#define MAX_UTILIZADORES 30
#define NVEICULOS 10
#define MAX_USERNAME_TAM 64
#define MAX_CHARACTERS 256
#define MAX_ARGUMENTOS 5
#define MAX_AGENDAMENTOS 100
#define REGISTADO "registado"
#define NEGADO "negado"

// Estruturas
typedef struct
{
    int hora; // segundos desde o agendamento
    char local[128];
    float distancia;
} Servico;

typedef struct
{
    pid_t pid_cliente;
    char fifo_cliente[MAX_CHARACTERS];
    char username[MAX_USERNAME_TAM];
} Cliente;

typedef struct {
    int id;                 
    Servico servico;       
    Cliente cliente;        
    bool ativo;          
    bool em_execucao;       
    pid_t pid_veiculo;      
    int pipe_leitura_veiculo; 
} Agendamento;

typedef struct
{
    pid_t pid_cliente;
    char comando[MAX_CHARACTERS];
} Pedido;

typedef struct
{
    pid_t pid_veiculo;
    char fifo_veiculo[MAX_CHARACTERS];
} Veiculo;

// Funções do Cliente
void* thread_recebe_mensagens(void* arg);
void* thread_envia_pedidos(void* arg);

// Funções do Controlador
int agendar(Cliente cliente, Servico servico);
int consultar(Cliente cliente, int id);
int cancelar(Cliente cliente);
int filtraPedido(char pedido[], Cliente cliente);
int executarOperacao(char *argumentos[], int n_argumentos, Cliente cliente);
void processar_comandos_controlador(char comando[]);
void* thread_gestao_comandos(void* arg);
void* thread_relogio(void* arg);

// Funções partilhadas
Servico criarServico(char *hora, char *local, char *distancia);
void criarFIFO(const char *fifo_name);
int abrirFIFO(const char *fifo_name, bool write);