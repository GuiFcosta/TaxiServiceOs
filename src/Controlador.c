#include "Utils.h"

bool verificaUserName(int num_clientes, char clientes[][MAX_USERNAME_TAM], char *username)
{
    for (int i = 0; i < num_clientes; i++)
    {
        if (strcmp(clientes[i], username) == 0)
        {
            return true;
        }
    }
    return false;
}

int enviarVeiculo(Servico agendamentos[]){
    int veiculo = 111;

    // Recebe: lista de servicos agendados, cliente que agendou o serviço
    // TODO: Lança o programa Veiculo para o cliente

    return veiculo;
}

int agendar(Cliente cliente, Servico servico)
{
    char confirma[MAX_CHARACTERS];
    sprintf(confirma, "Servico agendado com sucesso.\n Hora: %02d:%02d, Local: %s, Distancia: %.2f km\n",
            servico.hora.horas, servico.hora.minutos, servico.local, servico.distancia);

    int fd_cliente = abrirFIFO(cliente.fifo_cliente, true);
    write(fd_cliente, confirma, strlen(confirma));
    close(fd_cliente);

    // TODO: Adicionar o agendamento a uma lista

    return 200;
}

int consultar(Cliente cliente, int id)
{
    /* Implementar a lógica de consulta */
    return 200;
}

int cancelar(Cliente cliente)
{
    /* Implementar a lógica de cancelamento */
    return 200;
}

int executarOperacao(char *argumentos[], int n_argumentos, Cliente cliente)
{
    int resposta;
    if (strcmp(argumentos[0], "agendar") == 0)
    {
        if (n_argumentos != 4)
        {
            printf("[ERRO] Número inválido de argumentos para agendar.\n");
            return -1;
        }
        char *hora = argumentos[1];
        char *local = argumentos[2];
        char *distancia = argumentos[3];

        resposta = agendar(cliente, criarServico(hora, local, distancia));
    }
    else if (strcmp(argumentos[0], "consultar") == 0)
    {
        if (n_argumentos != 2)
        {
            printf("[ERRO] Número inválido de argumentos para consultar.\n");
            return -1;
        }
        int id = atoi(argumentos[1]);
        resposta = consultar(cliente, id);
    }
    else if (strcmp(argumentos[0], "cancelar") == 0)
    {
        resposta = cancelar(cliente);
    }
    else
    {
        printf("Operação inválida. %s.\n", argumentos[0]);
        resposta = -1;
    }
    return resposta;
}

int filtraPedido(char pedido[], Cliente cliente)
{
    char *argumentos[MAX_ARGUMENTOS];
    int n_argumentos = 0;
    char *token = strtok(pedido, " ");

    while (token != NULL && n_argumentos < MAX_ARGUMENTOS)
    {
        argumentos[n_argumentos++] = token;
        token = strtok(NULL, " ");
    }

    return executarOperacao(argumentos, n_argumentos, cliente);
}

int main()
{
    Cliente cliente;
    int fd_servidor, fd_cliente;
    int num_clientes = 0;
    char clientes[MAX_UTILIZADORES][MAX_USERNAME_TAM];
    char pedido[MAX_CHARACTERS];
    int res;

    criarFIFO(FIFO_SERVIDOR); // cria o FIFO do servidor

    printf("Controlador a espera de clientes...\n");

    while (1)
    {
        fd_servidor = abrirFIFO(FIFO_SERVIDOR, false); // abrir o FIFO do servidor para ler pedidos de conexao

        int bytes = read(fd_servidor, &cliente, sizeof(Cliente));
        close(fd_servidor);

        if (bytes < 0)
            continue;

        printf("[CONTROLADOR]: Pedido de conexao de <%s> FIFO: %s\n",
               cliente.username, cliente.fifo_cliente);

        bool nomeEmUso = verificaUserName(num_clientes, clientes, cliente.username);

        fd_cliente = abrirFIFO(cliente.fifo_cliente, true);

        if (nomeEmUso)
        {
            char mensagem[MAX_CHARACTERS] = "negado";
            write(fd_cliente, mensagem, strlen(mensagem));
            printf("[CONTROLADOR]: Nome em uso\n");
        }
        else
        {
            char mensagem[MAX_CHARACTERS] = "registado";
            write(fd_cliente, mensagem, strlen(mensagem));
            printf("[CONTROLADOR]: Cliente %s registado\n", cliente.username);
            strcpy(clientes[num_clientes++], cliente.username);
        }
        close(fd_cliente);
    }

    // ciclo para receber os pedidos do cliente que foi conectado
    // erro aqui, corrigir amanhã
    // TODO
    while (1)
    {
        fd_servidor = abrirFIFO(FIFO_SERVIDOR, false);
        memset(pedido, 0, sizeof(pedido));
        read(fd_servidor, pedido, sizeof(pedido));
        close(fd_servidor);

        res = filtraPedido(pedido, cliente);
        if (res == 200)
            continue;
        break;
    }
    unlink(FIFO_SERVIDOR);
    return 0;
}
