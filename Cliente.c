#include "Utils.h"

Servico criarServico(char *hora, char *local, char *distancia)
{
    Servico servico;

    sscanf(hora, "%d:%d", &servico.hora.horas, &servico.hora.minutos);

    strncpy(servico.local, local, sizeof(servico.local) - 1);
    servico.local[sizeof(servico.local) - 1] = '\0'; // Garantir terminação nula

    servico.distancia = atof(distancia);

    return servico;
}

int executarOperacao(char *argumentos[], int n_argumentos, int fd_servidor, int fd_cliente, Cliente pedido)
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

        resposta = agendar(fd_servidor, pedido, criarServico(hora, local, distancia));
    }
    else if (strcmp(argumentos[0], "consultar") == 0)
    {
        if (n_argumentos != 2)
        {
            printf("[ERRO] Número inválido de argumentos para consultar.\n");
            return -1;
        }
        int id = atoi(argumentos[1]);
        resposta = consultar(fd_servidor, id);
    }
    else if (strcmp(argumentos[0], "cancelar") == 0)
    {
        resposta = cancelar(fd_servidor, pedido);
    }
    else
    {
        printf("Operação inválida. %s.\n", argumentos[0]);
        resposta = -1;
    }
    return resposta;
}

int escolheServico(int fd_servidor, int fd_cliente, Cliente pedido)
{

    char escolha[MAX_CHARACTERS];
    char *argumentos[MAX_ARGUMENTOS];

    while (1)
    {
        printf("Escolha alguma das opções abaixo:\n");
        printf("- agendar <hora> <local> <distancia>\n- consultar\n- cancelar <id>\n- terminar\n");

        if (fgets(escolha, sizeof(escolha), stdin) == NULL)
        {
            printf("[ERRO] ao ler a entrada\n");
            continue;
        }
        escolha[strcspn(escolha, "\n")] = 0; // remover o '\n' do final da string

        if (strcmp(escolha, "terminar") == 0)
        {
            printf("A sair...\n");
            break;
        }

        int n_argumentos = 0;
        char *token = strtok(escolha, " ");

        while (token != NULL && n_argumentos < MAX_ARGUMENTOS)
        {
            argumentos[n_argumentos++] = token;
            token = strtok(NULL, " ");
        }

        return executarOperacao(argumentos, n_argumentos, fd_servidor, fd_cliente, pedido);
    }
}

int agendar(int fd_servidor, Cliente pedido, Servico servico)
{
    printf("Servico agendado com sucesso.\n");
    printf("Hora: %02d:%02d, Local: %s, Distancia: %.2f km\n",
           servico.hora.horas, servico.hora.minutos,
           servico.local, servico.distancia);
    /* Implementar a lógica de agendamento */
    return 100;
}

int consultar(int fd_servidor, int id)
{
    /* Implementar a lógica de consulta */
    return 200;
}

int cancelar(int fd_servidor, Cliente pedido)
{
    /* Implementar a lógica de cancelamento */
    return 300;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Sintaxe: %s <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    Cliente pedido;

    int fd_servidor; /* Identificador do FIFO do servidor */
    int fd_cliente;  /* Identificador do FIFO do cliente */

    int servico;

    char mensagem[MAX_RESPOSTA_TAM];
    pedido.pid_cliente = getpid();

    snprintf(pedido.fifo_cliente, sizeof(pedido.fifo_cliente), FIFO_CLIENTE, pedido.pid_cliente);
    criarFIFO(pedido.fifo_cliente); // cria o FIFO do cliente

    printf("[CLIENTE]: Fazendo pedido de conexão para o servidor...\n");
    sleep(3);
    strcpy(pedido.username, argv[1]);

    fd_servidor = abrirFIFO(FIFO_SERVIDOR, true); // abrir o FIFO do servidor para escrever o pedido de conexao

    write(fd_servidor, &pedido, sizeof(Cliente));
    close(fd_servidor);

    fd_cliente = abrirFIFO(pedido.fifo_cliente, false); // abrir o FIFO do cliente para ler a resposta do controlador

    memset(mensagem, 0, sizeof(mensagem));
    read(fd_cliente, mensagem, sizeof(mensagem));
    close(fd_cliente);

    printf("[CONTROLADOR]: %s\n", mensagem);
    unlink(pedido.fifo_cliente);

    if (strcmp(mensagem, "registado") == 0)
    {
        printf("[CLIENTE]: Registo efetuado com sucesso.\n");
        servico = escolheServico(fd_servidor, fd_cliente, pedido);
        printf("[CLIENTE]: Servico com ID %d recebido.\n", servico);
    }
    else if (strcmp(mensagem, "negado") == 0)
    {
        printf("[CLIENTE]: Username em uso.\n");
    }
    unlink(pedido.fifo_cliente);
    return 0;
}