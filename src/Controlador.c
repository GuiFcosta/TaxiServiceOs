#include "Utils.h"

Agendamento agendamentos[MAX_AGENDAMENTOS];
static int num_agendamentos = 0;

static Cliente clientes[MAX_UTILIZADORES];
static int num_clientes = 0;

static int tempo_simulado = 0;
static float distancia_atual = 0.0f;
static float distancia_total = 0.0f;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_agendamentos = PTHREAD_MUTEX_INITIALIZER;

void enviarVeiculo(Agendamento *ag)
{
  int p_fd[2]; // Pipe anónimo para ler o stdout do veiculo
  pipe(p_fd);  // Cria o pipe

  pid_t pid = fork();

  // Processo FILHO (Veículo - pid = 0) ou PAI (Controlador)
  if (pid == 0)
  {
    // Redirecionar stdout para o pipe (para o pai ler)
    close(p_fd[0]);               // Fecha leitura
    dup2(p_fd[1], STDOUT_FILENO); // stdout agora vai para o pipe
    close(p_fd[1]);

    char hora_agendamento[10];
    char distancia_agendamento[10];
    sprintf(hora_agendamento, "%d", ag->servico.hora);
    sprintf(distancia_agendamento, "%.2f", ag->servico.distancia);

    // Executar o Veículo
    execl("./Veiculo", "Veiculo",
          hora_agendamento,         // Arg 1: Hora
          ag->servico.local,        // Arg 2: Local
          distancia_agendamento,    // Arg 3: Distância
          ag->cliente.fifo_cliente, // Arg 4: Pipe do cliente
          NULL);

    perror("Falha ao lançar Veiculo");
    exit(1);
  }
  else
  {
    close(p_fd[1]); // Fecha escrita

    ag->em_execucao = true;
    ag->pid_veiculo = pid;
    ag->pipe_leitura_veiculo = p_fd[0];

    pthread_t t_monitor;

    Veiculo *dados = malloc(sizeof(Veiculo));
    dados->pipe_leitura = p_fd[0];
    dados->id_veiculo = ag->id;

    if (pthread_create(&t_monitor, NULL, thread_escuta_veiculo, dados) != 0)
    {
      perror("Erro ao criar thread monitor");
    }

    printf("[CONTROLADOR] Veículo (PID: %d) lançado para serviço\n", ag->pid_veiculo);
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
  char resposta[MAX_CHARACTERS * 3];

  pthread_mutex_lock(&lock_agendamentos);

  Agendamento *encontrado = encontrarAgendamento(agendamentos, cliente, num_agendamentos, id);

  if (encontrado == NULL)
  {
    snprintf(resposta, sizeof(resposta), "Nao existe um servico ativo com ID %d associado a este utilizador.\n", id);
  }
  else
  {
    const char *estado = encontrado->em_execucao ? "Em execucao" : "Agendado";
    Servico s = encontrado->servico;

    snprintf(resposta, sizeof(resposta),
             "Servico %d (%s):\n"
             "- Tempo restante: %d segundo(s)\n"
             "- Local: %s\n"
             "- Distancia: %.2f km\n",
             encontrado->id, estado, s.hora, s.local, s.distancia);
  }

  pthread_mutex_unlock(&lock_agendamentos);

  // Enviar ao cliente pela FIFO dele
  int fd = abrirFIFO(cliente.fifo_cliente, true);
  write(fd, resposta, strlen(resposta));
  close(fd);

  return 200;
}

int cancelar(Cliente cliente, int id)
{
  char resposta[MAX_CHARACTERS * 2];

  pthread_mutex_lock(&lock_agendamentos);

  Agendamento *encontrado = encontrarAgendamento(agendamentos, cliente, num_agendamentos, id);

  if (encontrado == NULL)
  {
    snprintf(resposta, sizeof(resposta), "Nao existe um servico ativo com ID %d associado a este utilizador.\n", id);
  }
  else
  {
    // cancelar
    if (encontrado->em_execucao && encontrado->pid_veiculo > 0)
    {
      // srviço em execução: cancelar via SIGUSR1
      if (kill(encontrado->pid_veiculo, SIGUSR1) == -1)
      {
        perror("[CONTROLADOR] Erro ao enviar SIGUSR1 ao veiculo");
        snprintf(resposta, sizeof(resposta), "Falha ao cancelar o servico %d (erro ao contactar veiculo).\n", id);
      }
      else
      {
        snprintf(resposta, sizeof(resposta), "Pedido de cancelamento enviado ao veiculo para o servico %d.\n", id);
      }
    }
    else
    {
      // Ainda não começou: só marcamos como inativo
      snprintf(resposta, sizeof(resposta), "Servico %d cancelado antes de iniciar.\n", id);
    }

    encontrado->ativo = false;
  }

  pthread_mutex_unlock(&lock_agendamentos);

  int fd = abrirFIFO(cliente.fifo_cliente, true);
  write(fd, resposta, strlen(resposta));
  close(fd);

  return 200;
}

int terminar(Cliente cliente)
{
  const char msg[] = "terminado";
  int fd = abrirFIFO(cliente.fifo_cliente, true);
  write(fd, msg, strlen(msg));
  close(fd);

  // Remover cliente da lista de clientes
  pthread_mutex_lock(&lock);
  for (int i = 0; i < num_clientes; i++)
  {
    if (clientes[i].pid_cliente == cliente.pid_cliente)
    {
      clientes[i] = clientes[num_clientes - 1];
      num_clientes--;
      break;
    }
  }
  pthread_mutex_unlock(&lock);

  // Cancelar todos os serviços futuros deste cliente
  pthread_mutex_lock(&lock_agendamentos);
  for (int i = 0; i < num_agendamentos; i++)
  {
    if (agendamentos[i].ativo &&
        !agendamentos[i].em_execucao &&
        agendamentos[i].cliente.pid_cliente == cliente.pid_cliente)
    {
      agendamentos[i].ativo = false;
    }
  }
  pthread_mutex_unlock(&lock_agendamentos);

  printf("[CONTROLADOR] Cliente %s (PID %d) terminou a sessao.\n", cliente.username, cliente.pid_cliente);

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
    if (n_argumentos != 2)
    {
      printf("[ERRO] Número inválido de argumentos para cancelar.\n");
      return -1;
    }
    int id = atoi(argumentos[1]);
    resposta = cancelar(cliente, id);
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
  else if (strcmp(comando, "frota") == 0)
  {
    printf("--- Estado da Frota ---\n");
    for (int i = 0; i < num_agendamentos; i++)
    {
      Agendamento *ag = &agendamentos[i];
      if (ag->em_execucao && ag->ativo)
      {
        printf("Veículo PID: %d | Distancia: %.2f km\n",
               ag->pid_veiculo,
               distancia_atual);
      }
    }
  }
  else if (strncmp(comando, "cancelar", 8) == 0)
  {
    char *arg = comando + 9; // Pular o comando e o espaço
    int id = atoi(arg);
    printf("Cancelando serviço com ID: %d\n", id);
    pthread_mutex_lock(&lock_agendamentos);
    Agendamento *encontrado = NULL;
    for (int i = 0; i < num_agendamentos; i++)
    {
      if (agendamentos[i].id == id && agendamentos[i].ativo)
      {
        encontrado = &agendamentos[i];
        break;
      }
    }

    if (encontrado == NULL)
    {
      printf("Nao existe um servico ativo com ID %d.\n", id);
    }
    else
    {
      if (encontrado->em_execucao && encontrado->pid_veiculo > 0)
      {
        if (kill(encontrado->pid_veiculo, SIGUSR1) == -1)
        {
          perror("[CONTROLADOR] Erro ao enviar SIGUSR1 ao veiculo");
          printf("Falha ao cancelar o servico %d (erro ao contactar veiculo).\n", id);
        }
        else
        {
          printf("Pedido de cancelamento enviado ao veiculo para o servico %d.\n", id);
        }
      }
      else
      {
        printf("Servico %d cancelado antes de iniciar.\n", id);
      }
      encontrado->ativo = false;
    }
    pthread_mutex_unlock(&lock_agendamentos);
  }
  else if (strcmp(comando, "km") == 0)
  {
    printf("--- Quilometragem Total ---\n");
    printf("Quilometragem total: %.2f km\n", distancia_total);
  }
  else if (strcmp(comando, "hora") == 0)
  {
    printf("--- Hora Atual ---\n");
    printf("Hora atual (simulada): %d segundos.\n", tempo_simulado);
  }
  else if (strcmp(comando, "terminar") == 0)
  {
    printf("Terminando o controlador...\n");
    printf("A avisar todos os clientes...\n");
    for (int i = 0; i < num_clientes; i++)
    {
      terminar(clientes[i]);
    }
    unlink(FIFO_SERVIDOR);
    exit(0);
  }
  else
  {
    printf("[ERRO] Comando desconhecido: %s\n", comando);
  }
}

void *thread_relogio(void *arg)
{
  while (1)
  {
    sleep(1);
    pthread_mutex_lock(&lock_agendamentos);
    for (int i = 0; i < num_agendamentos; i++)
    {
      if (agendamentos[i].ativo && !agendamentos[i].em_execucao)
      {
        tempo_simulado += 1;
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

  while (1)
  {
    printf("[CONTROLADOR] Escolha alguma das opções abaixo:\n"
           "- listar\n"
           "- utiliz\n"
           "- frota\n"
           "- cancelar <id>\n"
           "- km\n"
           "- hora\n"
           "- terminar\n");

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

void *thread_escuta_veiculo(void *arg)
{
  char buffer[1024];
  int n, dec;

  Veiculo *dados = (Veiculo *)arg;
  Agendamento *ag = NULL;

  for (int i = 0; i < num_agendamentos; i++)
  {
    if (agendamentos[i].id == dados->id_veiculo)
    {
      ag = &agendamentos[i];
      break;
    }
  }

  dec = ag->servico.distancia / 10.0f;

  // Lê do pipe enquanto o pipe estiver aberto (ou seja, enquanto o veiculo corre)
  while ((n = read(dados->pipe_leitura, buffer, sizeof(buffer) - 1)) > 0)
  {
    buffer[n] = '\0';

    pthread_mutex_lock(&lock);

    printf("[VEICULO %d]: %s", dados->id_veiculo, buffer);

    distancia_total += dec;

    pthread_mutex_unlock(&lock);
  }

  close(dados->pipe_leitura);
  free(dados);
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

      for (int i = 0; i < num_clientes; i++)
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
