#include "Utils.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char fifo_cliente_nome[MAX_CHARACTERS];
int fd_cliente;

void *thread_recebe_mensagens(void *arg)
{
  char resposta[MAX_CHARACTERS];

  fd_cliente = open(fifo_cliente_nome, O_RDWR);

  while (1)
  {
    int n = read(fd_cliente, resposta, sizeof(resposta));
    if (n > 0)
    {
      resposta[n] = '\0';

      pthread_mutex_lock(&lock);
      printf("\n[CONTROLADOR]: %s\n", resposta);
      printf("> "); // Volta a mostrar o prompt
      fflush(stdout);
      pthread_mutex_unlock(&lock);

      if (strcmp(resposta, "terminado") == 0)
      {
        printf("[CLIENTE]: Sessao terminada pelo servidor.\n");
        close(fd_cliente);
        exit(0);
      }
    }
  }
  return NULL;
}

void *thread_envia_pedidos(void *arg)
{
  Cliente *cliente = (Cliente *)arg;
  Pedido pedido;
  pedido.pid_cliente = cliente->pid_cliente;

  while (1)
  {
    pthread_mutex_lock(&lock);

    printf("[CLIENTE] Escolha alguma das opções abaixo:\n"
           "- agendar <hora> <local> <distancia>\n"
           "- consultar <id>\n"
           "- cancelar <id>\n"
           "- terminar\n");

    pthread_mutex_unlock(&lock);

    if (fgets(pedido.comando, sizeof(pedido.comando), stdin) == NULL)
    {
      printf("[ERRO] ao ler a entrada\n");
      break;
    }
    pedido.comando[strcspn(pedido.comando, "\n")] = 0;

    int fd_servidor = abrirFIFO(FIFO_SERVIDOR, true);
    write(fd_servidor, &pedido, sizeof(Pedido));
    close(fd_servidor);
  }
  return NULL;
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
  cliente.pid_cliente = getpid();    // obter o PID do cliente
  strcpy(cliente.username, argv[1]); // copiar o username passado como argumento

  snprintf(cliente.fifo_cliente, sizeof(cliente.fifo_cliente), FIFO_CLIENTE, cliente.pid_cliente);

  criarFIFO(cliente.fifo_cliente); // cria o FIFO do cliente

  strcpy(fifo_cliente_nome, cliente.fifo_cliente); // guardar o nome do FIFO do cliente para a thread

  printf("[CLIENTE]: Fazendo pedido de conexão para o servidor...\n");
  sleep(2);

  // abrir o FIFO do servidor para escrever o cliente de conexao
  fd_servidor = abrirFIFO(FIFO_SERVIDOR, true);

  write(fd_servidor, &cliente, sizeof(Cliente));
  close(fd_servidor);

  // abrir o FIFO do cliente para ler a resposta do controlador
  fd_cliente = abrirFIFO(cliente.fifo_cliente, false);

  memset(mensagem, 0, sizeof(mensagem));
  read(fd_cliente, mensagem, sizeof(mensagem));
  close(fd_cliente);

  printf("[CONTROLADOR]: %s\n", mensagem);

  if (strcmp(mensagem, REGISTADO) == 0)
  {
    printf("[CLIENTE]: Registo efetuado com sucesso.\n");
    pthread_t thread_recebe, thread_envia;

    // criar threads para receber mensagens e enviar pedidos
    pthread_create(&thread_recebe, NULL, thread_recebe_mensagens, NULL);
    pthread_create(&thread_envia, NULL, thread_envia_pedidos, &cliente);

    pthread_join(thread_envia, NULL);
  }
  else if (strcmp(mensagem, NEGADO) == 0)
  {
    printf("[CLIENTE]: Username em uso.\n");
  }

  close(fd_cliente);
  unlink(cliente.fifo_cliente);
  return 0;
}