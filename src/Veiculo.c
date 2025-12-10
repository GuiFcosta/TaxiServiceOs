#include "Utils.h"
#include <pthread.h>

int fd_cliente;        
int fd_veiculo; // Pipe onde o veículo recebe comandos do cliente (entrar, sair)
char fifo_veiculo[MAX_CHARACTERS]; // Nome do pipe deste veículo

bool em_viagem = false;
bool cancelarViagem = false;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// --- Tratamento de Sinal (Cancelamento pelo Controlador) ---
void handle_sigusr1(int sig) {
    // Envia aviso ao cliente
    char msg[] = "Servico cancelado pelo controlador (SIGUSR1).";
    write(fd_cliente, msg, strlen(msg) + 1);
    
    printf("TERMINADO ABORTADO\n");
    fflush(stdout);
    
    // Limpeza
    close(fd_cliente);
    close(fd_veiculo);
    unlink(fifo_veiculo);
    exit(0);
}

// --- Thread que escuta o Cliente (Comandos: "entrar", "sair") ---
void* thread_escuta_cliente(void* arg) {
    char buffer[MAX_CHARACTERS];
    
    while(1) {
        int n = read(fd_veiculo, buffer, sizeof(buffer));
        if (n > 0) {
            buffer[n-1] = '\0'; // Remove \n se existir (cuidado com strings puras)
            
            // Comando: entrar <destino>
            if (strncmp(buffer, "entrar", 6) == 0) {
                pthread_mutex_lock(&lock);
                if (!em_viagem) { // Só aceita entrar se ainda não começou
                    em_viagem = true; // Sinaliza à main thread para arrancar
                    char* destino = buffer + 7; // Pula "entrar "
                    printf("INICIO %s\n", destino); // Telemetria: Cliente entrou
                    fflush(stdout);
                }
                pthread_mutex_unlock(&lock);
            }
            // Comando: sair
            else if (strncmp(buffer, "sair", 4) == 0) {
                pthread_mutex_lock(&lock);
                cancelarViagem = true; // Sinaliza cancelamento
                printf("TERMINADO PARAGEM_CLIENTE\n");
                fflush(stdout);
                pthread_mutex_unlock(&lock);
                
                char msg[] = "Viagem terminada a pedido do cliente.";
                write(fd_cliente, msg, strlen(msg) + 1);
                
                // Sair do programa
                unlink(fifo_veiculo);
                exit(0);
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "[ERRO] Veiculo args invalidos.\n");
        return 1;
    }

    signal(SIGUSR1, handle_sigusr1);

    float distancia = atof(argv[3]);
    char* fifo_cliente_nome = argv[4];

    sprintf(fifo_veiculo, "/tmp/fifo_veiculo_%d", getpid());
    criarFIFO(fifo_veiculo);
    fd_veiculo = open(fifo_veiculo, O_RDWR); // RDWR evita EOF loop se não houver escritores

    fd_cliente = abrirFIFO(fifo_cliente_nome, O_WRONLY);

    char mensagem[MAX_CHARACTERS * 2];
    
    sprintf(mensagem, "Ola! O Veiculo chegou ao local %s. Escreva 'entrar <destino>' para iniciar.", argv[2]);
    write(fd_cliente, mensagem, strlen(mensagem) + 1);
    
    // Pequeno truque: Mandar o nome do pipe para o cliente saber onde mandar o "entrar"
    // (O ideal seria alterar o protocolo do cliente para suportar isto, mas por agora 
    // assumimos que o cliente vai ter de saber ou usamos um protocolo fixo. 
    // Vê nota abaixo sobre a alteração no cliente).
    sprintf(mensagem, "PIPE_VEICULO %s", fifo_veiculo);
    write(fd_cliente, mensagem, strlen(mensagem) + 1);

    pthread_t t_escuta;
    pthread_create(&t_escuta, NULL, thread_escuta_cliente, NULL);

    pthread_join(t_escuta, NULL);

    close(fd_cliente);
    close(fd_veiculo);
    unlink(fifo_veiculo);
    return 0;
}