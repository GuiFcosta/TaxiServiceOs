#include "Utils.h"

int escolheServico(Cliente cliente)
{
    Pedido pedido;
    char resposta[MAX_CHARACTERS]; // guarda a resposta do servidor

    memset(&pedido, 0, sizeof(Pedido));

    printf("[CLIENTE] Escolha alguma das opções abaixo:\n");
    printf("- agendar <hora> <local> <distancia>\n- consultar\n- cancelar <id>\n- terminar\n");

    if (fgets(pedido.comando, sizeof(pedido.comando), stdin) == NULL)
    {
        printf("[ERRO] ao ler a entrada\n");
        return 0;
    }
    pedido.comando[strcspn(pedido.comando, "\n")] = 0; // remover o '\n' do final da string

    if (strcmp(pedido.comando, "terminar") == 0)
    {
        printf("A sair...\n");
        return 0;
    }

    pedido.pid_cliente = cliente.pid_cliente;

    int fd_servidor = abrirFIFO(FIFO_SERVIDOR, true);
    write(fd_servidor, &pedido, sizeof(Pedido));
    close(fd_servidor);

    int fd_cliente = abrirFIFO(cliente.fifo_cliente, false);
    memset(resposta, 0, sizeof(resposta));
    if(read(fd_cliente, &resposta, sizeof(resposta)) < 0){
        printf("[ERRO] ao ler a resposta do servidor\n");
        return 0;
    }
    printf("[CONTROLADOR]: %s\n", resposta);
    close(fd_cliente);

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

    printf("[CLIENTE]: Fazendo pedido de conexão para o servidor...\n");
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

    if (strcmp(mensagem, REGISTADO) == 0)
    {
        printf("[CLIENTE]: Registo efetuado com sucesso.\n");
        while (escolheServico(cliente)){ /* continua enquanto a função retornar 1*/ }
        printf("[CLIENTE]: A sair...\n");
    }
    else if (strcmp(mensagem, NEGADO) == 0)
    {
        printf("[CLIENTE]: Username em uso.\n");
    }
    unlink(cliente.fifo_cliente);
    return 0;
}