#include "Utils.h"
#include <pthread.h>

int fd_cliente;        

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// --- Tratamento de Sinal (Cancelamento pelo Controlador) ---
void handle_sigusr1(int sig) {
    (void)sig; // evitar warning de unused parameter

    // Envia aviso ao cliente
    char msg[] = "Servico cancelado pelo controlador (SIGUSR1).";
    write(fd_cliente, msg, strlen(msg) + 1);
    
    printf("TERMINADO ABORTADO\n");
    fflush(stdout);
    
    // Limpeza
    close(fd_cliente);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "[ERRO] Veiculo args invalidos.\n");
        return 1;
    }

    signal(SIGUSR1, handle_sigusr1);

    float distancia = atof(argv[3]);
    char* fifo_cliente_nome = argv[4];


    fd_cliente = abrirFIFO(fifo_cliente_nome, O_WRONLY);

    char mensagem[MAX_CHARACTERS * 2];
    
    sprintf(mensagem, "Ola! O Taxi chegou ao local %s", argv[2]);
    write(fd_cliente, mensagem, strlen(mensagem) + 1);

    printf("Taxi iniciou viagem em %s\n", argv[2]);
    fflush(stdout);

    float tempo_viagem = distancia; // velocidade = 1 unidade por segundo
    float intervalo = tempo_viagem / 10;
    for(int i = 1; i <=10; i++) {
        sleep(intervalo);

        printf("Viagem: %.0f%% concluido.\n", (i / 10.0) * 100);
        fflush(stdout);
    }
    
    printf("TERMINADO CONCLUIDO\n");
    fflush(stdout);

    strcpy(mensagem, "Servico concluido com sucesso.");
    write(fd_cliente, mensagem, strlen(mensagem) + 1);
    
    close(fd_cliente);
    return 0;
}