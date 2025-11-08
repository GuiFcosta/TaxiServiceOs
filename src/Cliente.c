#include "Utils.h"

int escolheServico(int fd_servidor, int fd_cliente, Cliente cliente)
{
    char escolha[MAX_CHARACTERS];
    char resposta[MAX_CHARACTERS];

    printf("[CLIENTE] Escolha alguma das opções abaixo:\n");
    printf("- agendar <hora> <local> <distancia>\n- consultar\n- cancelar <id>\n- terminar\n");

    if (fgets(escolha, sizeof(escolha), stdin) == NULL)
    {
        printf("[ERRO] ao ler a entrada\n");
        return 1;
    }
    escolha[strcspn(escolha, "\n")] = 0; // remover o '\n' do final da string

    if (strcmp(escolha, "terminar") == 0)
    {
        printf("A sair...\n");
        return 0;
    }

    fd_servidor = abrirFIFO(FIFO_SERVIDOR, true);
    write(fd_servidor, &escolha, sizeof(escolha));
    close(fd_servidor);

    fd_cliente = abrirFIFO(cliente.fifo_cliente, false);
    memset(resposta, 0, sizeof(resposta));
    read(fd_cliente, &resposta, sizeof(resposta));
    if (strcmp(resposta, "aceite") == 0)
        return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Sintaxe: %s <username>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    Cliente cliente;

    int fd_servidor; /* Identificador do FIFO do servidor */
    int fd_cliente;  /* Identificador do FIFO do cliente */

    int servico;

    char mensagem[MAX_CHARACTERS];
    cliente.pid_cliente = getpid();

    snprintf(cliente.fifo_cliente, sizeof(cliente.fifo_cliente), FIFO_CLIENTE, cliente.pid_cliente);
    criarFIFO(cliente.fifo_cliente); // cria o FIFO do cliente

    printf("[CLIENTE]: Fazendo cliente de conexão para o servidor...\n");
    sleep(3);
    strcpy(cliente.username, argv[1]);

    fd_servidor = abrirFIFO(FIFO_SERVIDOR, true); // abrir o FIFO do servidor para escrever o cliente de conexao

    write(fd_servidor, &cliente, sizeof(Cliente));
    close(fd_servidor);

    fd_cliente = abrirFIFO(cliente.fifo_cliente, false); // abrir o FIFO do cliente para ler a resposta do controlador

    memset(mensagem, 0, sizeof(mensagem));
    read(fd_cliente, mensagem, sizeof(mensagem));
    close(fd_cliente);

    printf("[CONTROLADOR]: %s\n", mensagem);
    unlink(cliente.fifo_cliente);

    if (strcmp(mensagem, "registado") == 0)
    {
        printf("[CLIENTE]: Registo efetuado com sucesso.\n");
        while (1)
        {
            servico = escolheServico(fd_servidor, fd_cliente, cliente);
            if (servico)
            {
                continue;
            }
            break;
        }
        printf("[CLIENTE]: A sair...\n");
    }
    else if (strcmp(mensagem, "negado") == 0)
    {
        printf("[CLIENTE]: Username em uso.\n");
    }
    unlink(cliente.fifo_cliente);
    return 0;
}