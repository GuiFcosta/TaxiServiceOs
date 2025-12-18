#include "Utils.h"
#include <pthread.h>

int fd_cliente;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Cancelado pelo controlador
void handle_sigusr1(int sig)
{
    (void)sig;

    // Envia aviso ao cliente
    char msg[] = "Servico cancelado pelo controlador (SIGUSR1).";
    write(fd_cliente, msg, strlen(msg) + 1);

    printf("ABORTADO\n");
    fflush(stdout);

    close(fd_cliente);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "[ERRO] Veiculo args invalidos.\n");
        return 1;
    }

    signal(SIGUSR1, handle_sigusr1);

    float distancia = atof(argv[3]);   // Distancia a percorrer
    char *fifo_cliente_nome = argv[4]; // FIFO do cliente para comunicar

    fd_cliente = abrirFIFO(fifo_cliente_nome, true);

    char mensagem[MAX_CHARACTERS];

    sprintf(mensagem, "O Taxi chegou ao local %s", argv[2]);
    write(fd_cliente, mensagem, strlen(mensagem) + 1);

    printf("INICIO %s\n", argv[2]);
    fflush(stdout);

    float tempo_viagem = distancia; // velocidade = 1 unidade por segundo
    float intervalo = tempo_viagem / 10;
    for (int i = 1; i <= 10; i++)
    {
        usleep(intervalo * 1000000);

        printf("ANDAMENTO %.0f\n", (i / 10.0) * 100);
        fflush(stdout);
    }

    printf("CONCLUIDO\n");
    fflush(stdout);

    strcpy(mensagem, "Servico concluido com sucesso.");
    write(fd_cliente, mensagem, strlen(mensagem) + 1);

    close(fd_cliente);
    return 0;
}
