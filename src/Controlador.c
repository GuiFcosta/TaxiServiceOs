#include "Utils.h"

Agendamento agendamentos[MAX_AGENDAMENTOS];
int num_agendamentos = 0;

static Cliente clientes[MAX_UTILIZADORES];

static int num_clientes = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_agendamentos = PTHREAD_MUTEX_INITIALIZER;

bool verificaUserName(int num_clientes, Cliente clientes[], char *username)
{
  for (int i = 0; i < num_clientes; i++)
    if (strcmp(clientes[i].username, username) == 0)
      return true;

  return false;
}

void enviarVeiculo(Agendamento* ag)
{
  int p_fd[2]; // Pipe anónimo para ler o stdout do veiculo
    pipe(p_fd);  // Cria o pipe

    pid_t pid = fork();

    if (pid == 0) { // Processo FILHO (Veículo)
        // 1. Redirecionar stdout para o pipe (para o pai ler)
        close(p_fd[0]);   // Fecha leitura
        dup2(p_fd[1], STDOUT_FILENO); // stdout agora vai para o pipe
        close(p_fd[1]);

        // 2. Preparar argumentos
        char arg_hora[10], arg_dist[10];
        sprintf(arg_hora, "%d", ag->servico.hora);
        sprintf(arg_dist, "%.2f", ag->servico.distancia);

        // 3. Executar o módulo Veículo
        execl("./Veiculo", "Veiculo", 
              arg_hora,               // Arg 1: Hora (ou ID)
              ag->servico.local,      // Arg 2: Local
              arg_dist,               // Arg 3: Distância
              ag->cliente.fifo_cliente,// Arg 4: Contacto do cliente (Pipe)
              NULL);
        
        perror("Falha ao lançar Veiculo");
        exit(1);
    } 
    else { // Processo PAI (Controlador)
        close(p_fd[1]); // Fecha escrita
        
        ag->em_execucao = true;
        ag->pid_veiculo = pid;
        ag->pipe_leitura_veiculo = p_fd[0]; // Guardar para ler telemetria depois
        
        printf("[CONTROLADOR] Veículo lançado para serviço %d\n", ag->id);
    }
  
}

int agendar(Cliente cliente, Servico servico)
{
  char confirma[MAX_CHARACTERS * 2];
  sprintf(confirma, "Servico agendado com sucesso.\n Hora(s): %d, Local: %s, Distancia: %.2f km\n",
          servico.hora, servico.local, servico.distancia);

  int fd_cliente = abrirFIFO(cliente.fifo_cliente, true);
  write(fd_cliente, confirma, strlen(confirma));
  close(fd_cliente);

  pthread_mutex_lock(&lock_agendamentos);

  Agendamento novo_agendamento;
  novo_agendamento.id = num_agendamentos + 1;
  novo_agendamento.servico = servico;
  novo_agendamento.cliente = cliente;
  novo_agendamento.ativo = true;
  novo_agendamento.em_execucao = false;
  novo_agendamento.pid_veiculo = -1;
  novo_agendamento.pipe_leitura_veiculo = -1;

  agendamentos[num_agendamentos++] = novo_agendamento;

  pthread_mutex_unlock(&lock_agendamentos);
  
  return 200;
}

int consultar(Cliente cliente, int id)
{
  /* Implementar a lógica de consulta */

  /* Warning aqui porque nao estou a usar as variaveis cliente e id */

  return 200;
}

int cancelar(Cliente cliente)
{
  /* Implementar a lógica de cancelamento */

  /* Warning aqui porque nao estou a usar a variavel cliente */

  return 200;
}

int terminar(Cliente cliente)
{

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
  else if (strcmp(argumentos[0], "terminar") == 0)
  {
    resposta = terminar(cliente);
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

void processar_comandos_controlador(char comando[])
{
  if (strcmp(comando, "listar") == 0)
  {
    printf("--- Lista de Serviços Agendados ---\n");
    for (int i = 0; i < num_agendamentos; i++)
    {
      Agendamento *ag = &agendamentos[i];
      if (ag->ativo)
      {
        printf("ID: %d | Cliente: %s | Hora(s): %d | Local: %s | Distancia: %.2f km | Em Execução: %s\n",
               ag->id,
               ag->cliente.username,
               ag->servico.hora,
               ag->servico.local,
               ag->servico.distancia,
               ag->em_execucao ? "Sim" : "Não");
      }
    }
  }
  else if (strcmp(comando, "utiliz") == 0)
  {
    printf("--- Lista de Utilizadores ---\n");
    for (int i = 0; i < num_clientes; i++)
      printf("- %s\n", clientes[i].username);
  }
  else if(strcmp(comando, "frota") == 0)
  {
    printf("--- Estado da Frota ---\n");
  }
  else if (strncmp(comando, "cancelar", 8) == 0)
  {
    char *arg = comando + 9; // Pular o comando e o espaço
    int id = atoi(arg);
    printf("Cancelando serviço com ID: %d\n", id);
  }
  else if (strcmp(comando, "km") == 0)
  {
    printf("--- Quilometragem Total ---\n");
  }
  else if (strcmp(comando, "hora") == 0)
  {
    printf("--- Hora Atual ---\n");
  }
  else if (strcmp(comando, "terminar") == 0)
  {
    printf("Terminando o controlador...\n");
    unlink(FIFO_SERVIDOR);
    exit(0);
  }
  else
  {
    printf("[ERRO] Comando desconhecido: %s\n", comando);
  }
}

void* thread_relogio(void* arg)
{
  while (1)
  {
    sleep(1);
    pthread_mutex_lock(&lock_agendamentos);
    for (int i = 0; i < num_agendamentos; i++)
    {
      if (agendamentos[i].ativo && !agendamentos[i].em_execucao)
      {
        agendamentos[i].servico.hora -= 1;
        if (agendamentos[i].servico.hora <= 0)
        {
          printf("[CONTROLADOR] Iniciando serviço agendado ID: %d\n", agendamentos[i].id);
          agendamentos[i].em_execucao = true;
          enviarVeiculo(&agendamentos[i]);
        }
      }
    }
    pthread_mutex_unlock(&lock_agendamentos);
  }
  return NULL;
}

void *thread_gestao_comandos(void *arg)
{
  char comando[MAX_CHARACTERS];

  printf("[CONTROLADOR] Escolha alguma das opções abaixo:\n"
         "- listar\n"
         "- utiliz\n"
         "- frota\n"
         "- cancelar <id>\n"
         "- km\n"
         "- hora\n"
         "- terminar\n");

  while (1)
  {
    printf("> ");

    if (fgets(comando, sizeof(comando), stdin) == NULL)
      continue;

    comando[strcspn(comando, "\n")] = 0;

    pthread_mutex_lock(&lock);
    processar_comandos_controlador(comando);
    pthread_mutex_unlock(&lock);
  }
  return NULL;
}

int main()
{
  Cliente cliente;
  int fd_servidor, fd_cliente;
  char comando[MAX_CHARACTERS];

  pthread_t th_comandos, th_relogio;

  if (pthread_create(&th_comandos, NULL, thread_gestao_comandos, NULL) != 0)
  {
    perror("Erro ao criar a thread de gestão de comandos");
    exit(1);
  }

  if (pthread_create(&th_relogio, NULL, thread_relogio, NULL) != 0)
  {
    perror("Erro ao criar a thread do relogio");
    exit(1);
  }

  criarFIFO(FIFO_SERVIDOR); // cria o FIFO do servidor

  printf("Controlador a espera de clientes...\n");

  while (1)
  {
    char buffer[sizeof(Cliente) > sizeof(Pedido) ? sizeof(Cliente) : sizeof(Pedido)];

    fd_servidor = abrirFIFO(FIFO_SERVIDOR, false); // abrir o FIFO do servidor para ler pedidos de conexao

    int bytes = read(fd_servidor, buffer, sizeof(buffer));

    close(fd_servidor);

    if (bytes <= 0)
      continue;

    pthread_mutex_lock(&lock);
    if (bytes == (int)sizeof(Cliente))
    {
      memcpy(&cliente, buffer, sizeof(Cliente));

      printf("\n[CONTROLADOR]: Pedido de conexao de <%s> FIFO: %s\n", cliente.username, cliente.fifo_cliente);

      bool nomeEmUso = verificaUserName(num_clientes, clientes, cliente.username);

      if (!nomeEmUso && num_clientes < MAX_UTILIZADORES)
      {
        clientes[num_clientes++] = cliente;
        printf("[CONTROLADOR]: Registando novo cliente: %s\n", cliente.username);
      }

      fd_cliente = abrirFIFO(cliente.fifo_cliente, true);

      if (nomeEmUso || num_clientes > MAX_UTILIZADORES)
      {
        char mensagem[MAX_CHARACTERS];
        snprintf(mensagem, sizeof(mensagem), "%s", NEGADO);
        write(fd_cliente, mensagem, strlen(mensagem));
        printf("[CONTROLADOR]: Nome em uso\n");
      }
      else
      {
        char mensagem[MAX_CHARACTERS];
        snprintf(mensagem, sizeof(mensagem), "%s", REGISTADO);
        write(fd_cliente, mensagem, strlen(mensagem));
      }
      close(fd_cliente);
    }
    else if (bytes == (int)sizeof(Pedido))
    {
      Pedido *pedido_ptr = (Pedido *)buffer;

      for(int i = 0; i < num_clientes; i++)
      {
        if (clientes[i].pid_cliente == pedido_ptr->pid_cliente)
        {
          cliente = clientes[i];
          break;
        }
      }

      cliente.pid_cliente = pedido_ptr->pid_cliente;

      printf("[CONTROLADOR]: Pedido de: %s\n", cliente.username);

      strcpy(comando, pedido_ptr->comando);

      if (filtraPedido(comando, cliente) == 200)
        printf("[CONTROLADOR]: Pedido executado com sucesso.\n");
      else
        printf("[CONTROLADOR]: Erro ao executar o pedido.\n");
    }
    else
    {
      printf("[CONTROLADOR]: Pedido inválido recebido.\n");
      continue;
    }

    pthread_mutex_unlock(&lock);
  }
  unlink(FIFO_SERVIDOR);
  return 0;
}
