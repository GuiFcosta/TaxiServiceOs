#include "Utils.h"

int main()
{
    Cliente pedido;
    int fd_servidor, fd_cliente;
    int num_clientes = 0;
    char clientes[MAX_UTILIZADORES][MAX_USERNAME_TAM];

    criarFIFO(FIFO_SERVIDOR); // cria o FIFO do servidor

    printf("Controlador a espera de clientes...\n");

    while (1)
    {
        fd_servidor = abrirFIFO(FIFO_SERVIDOR, false); // abrir o FIFO do servidor para ler pedidos de conexao

        int bytes = read(fd_servidor, &pedido, sizeof(Cliente));
        close(fd_servidor);

        if (bytes < 0)
            continue;

        printf("[CONTROLADOR]: Pedido de conexao de <%s> FIFO: %s\n",
               pedido.username, pedido.fifo_cliente);

        bool nomeEmUso = false;
        for (int i = 0; i < num_clientes; i++)
        {
            if (strcmp(pedido.username, clientes[i]) == 0)
            {
                nomeEmUso = true;
                break;
            }
        }

        fd_cliente = abrirFIFO(pedido.fifo_cliente, true);

        if (nomeEmUso)
        {
            char mensagem[MAX_RESPOSTA_TAM] = "negado";
            write(fd_cliente, mensagem, strlen(mensagem));
            printf("[CONTROLADOR]: Nome em uso\n");
        }
        else
        {
            char mensagem[MAX_RESPOSTA_TAM] = "registado";
            printf("[CONTROLADOR]: Cliente %s registado\n", pedido.username);
            write(fd_cliente, mensagem, strlen(mensagem));
            strcpy(clientes[num_clientes++], pedido.username);
        }
        close(fd_cliente);
    }
    unlink(FIFO_SERVIDOR);
    return 0;
}
