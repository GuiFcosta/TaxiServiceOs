#include "Utils.h"

static char clientes[MAX_UTILIZADORES][MAX_USERNAME_TAM];
static int num_clientes = 0;

bool verificaUserName(int num_clientes, char clientes[][MAX_USERNAME_TAM], char* username) {
  for (int i = 0; i < num_clientes; i++) {
    if (strcmp(clientes[i], username) == 0) {
      return true;
    }
  }
  return false;
}

int enviarVeiculo(Servico agendamentos[]) {
  int veiculo = 111;

  // Warning aqui porque nao estou a usar a variavel agendamentos[]

  // Recebe: lista de servicos agendados, cliente que agendou o serviço
  // TODO: Lança o programa Veiculo para o cliente

  return veiculo;
}

int agendar(Cliente cliente, Servico servico) {
  char confirma[MAX_CHARACTERS * 2];
  sprintf(confirma,
          "Servico agendado com sucesso.\n Hora: %02d:%02d, Local: %s, "
          "Distancia: %.2f km\n",
          servico.hora.horas, servico.hora.minutos, servico.local,
          servico.distancia);

  int fd_cliente = abrirFIFO(cliente.fifo_cliente, true);
  write(fd_cliente, confirma, strlen(confirma));
  close(fd_cliente);

  // TODO: Adicionar o agendamento a uma lista
  // v1: 1 array com todos os agendamentos e cada agendamento com o cliente
  // associado v2: 1 array de listas ligadas, cada lista ligada associada a um
  // cliente

  return 200;
}

int consultar(Cliente cliente, int id) {
  /* Implementar a lógica de consulta */

  /* Warning aqui porque nao estou a usar as variaveis cliente e id */

  return 200;
}

int cancelar(Cliente cliente) {
  /* Implementar a lógica de cancelamento */

  /* Warning aqui porque nao estou a usar a variavel cliente */

  return 200;
}

int terminar(Cliente cliente) {
  char confirma[MAX_CHARACTERS];
  sprintf(confirma, "terminado");
  int fd_cliente = abrirFIFO(cliente.fifo_cliente, true);
  write(fd_cliente, confirma, strlen(confirma));
  close(fd_cliente);

  for(int i = 0; i < num_clientes; i++) {
    if (strcmp(clientes[i], cliente.username) == 0) {
      // Remover o cliente da lista
      for (int j = i; j < num_clientes - 1; j++) {
        strcpy(clientes[j], clientes[j + 1]);
      }
      num_clientes--;
      break;
    }
  }

  printf("Clientes atualmente registados:\n");
  for(int i = 0; i < num_clientes; i++) {
    printf("Cliente %d: %s\n", i, clientes[i]);
  }

  /* Warning aqui porque nao estou a usar a variavel cliente */

  return 200;
}

int executarOperacao(char* argumentos[], int n_argumentos, Cliente cliente) {
  int resposta;
  if (strcmp(argumentos[0], "agendar") == 0) {
    if (n_argumentos != 4) {
      printf("[ERRO] Número inválido de argumentos para agendar.\n");
      return -1;
    }
    char* hora = argumentos[1];
    char* local = argumentos[2];
    char* distancia = argumentos[3];

    resposta = agendar(cliente, criarServico(hora, local, distancia));
  } else if (strcmp(argumentos[0], "consultar") == 0) {
    if (n_argumentos != 2) {
      printf("[ERRO] Número inválido de argumentos para consultar.\n");
      return -1;
    }
    int id = atoi(argumentos[1]);
    resposta = consultar(cliente, id);
  } else if (strcmp(argumentos[0], "cancelar") == 0) {
    resposta = cancelar(cliente);
  } else if (strcmp(argumentos[0], "terminar") == 0) {
    resposta = terminar(cliente);
  } else {
    printf("Operação inválida. %s.\n", argumentos[0]);
    resposta = -1;
  }
  return resposta;
}

int filtraPedido(char pedido[], Cliente cliente) {
  char* argumentos[MAX_ARGUMENTOS];
  int n_argumentos = 0;
  char* token = strtok(pedido, " ");

  while (token != NULL && n_argumentos < MAX_ARGUMENTOS) {
    argumentos[n_argumentos++] = token;
    token = strtok(NULL, " ");
  }

  return executarOperacao(argumentos, n_argumentos, cliente);
}

int main() {
  Cliente cliente;
  int fd_servidor, fd_cliente;
  int res;
  char comando[MAX_CHARACTERS];

  criarFIFO(FIFO_SERVIDOR);  // cria o FIFO do servidor

  printf("Controlador a espera de clientes...\n");

  while (1) {
    char buffer[sizeof(Cliente) > sizeof(Pedido) ? sizeof(Cliente) : sizeof(Pedido)];

    fd_servidor = abrirFIFO(FIFO_SERVIDOR, false);  // abrir o FIFO do servidor para ler pedidos de conexao

    int bytes = read(fd_servidor, buffer, sizeof(buffer));

    close(fd_servidor);

    if (bytes <= 0) continue;

    if (bytes == (int)sizeof(Cliente)) {
      memcpy(&cliente, buffer, sizeof(Cliente));

      printf("[CONTROLADOR]: Pedido de conexao de <%s> FIFO: %s\n", cliente.username, cliente.fifo_cliente);

      bool nomeEmUso = verificaUserName(num_clientes, clientes, cliente.username);

      if (!nomeEmUso && num_clientes < MAX_UTILIZADORES) {
        strcpy(clientes[num_clientes], cliente.username);
        printf("[CONTROLADOR]: Registando novo cliente: %s\n", cliente.username);
      }

      fd_cliente = abrirFIFO(cliente.fifo_cliente, true);

      if (nomeEmUso && num_clientes > MAX_UTILIZADORES) {
        char mensagem[MAX_CHARACTERS];
        snprintf(mensagem, sizeof(mensagem), "%s", NEGADO);
        write(fd_cliente, mensagem, strlen(mensagem));
        printf("[CONTROLADOR]: Nome em uso\n");
      } else {
        char mensagem[MAX_CHARACTERS];
        snprintf(mensagem, sizeof(mensagem), "%s", REGISTADO);
        write(fd_cliente, mensagem, strlen(mensagem));
        printf("[CONTROLADOR]: Cliente %s registado\n", cliente.username);
      }
      close(fd_cliente);
    } else if (bytes == (int)sizeof(Pedido)) {
      Pedido* pedido_ptr = (Pedido*)buffer;
      strcpy(comando, pedido_ptr->comando);
      cliente.pid_cliente = pedido_ptr->pid_cliente;

      printf("[CONTROLADOR]: Pedido recebido do cliente PID: %d - Comando: %s\n", cliente.pid_cliente, comando);

      res = filtraPedido(comando, cliente);
      if (res == 200)
        printf("[CONTROLADOR]: Pedido executado com sucesso.\n");
      else
        printf("[CONTROLADOR]: Erro ao executar o pedido.\n");
    } else {
      printf("[CONTROLADOR]: Pedido inválido recebido.\n");
      continue;
    }
  }
  unlink(FIFO_SERVIDOR);
  return 0;
}
