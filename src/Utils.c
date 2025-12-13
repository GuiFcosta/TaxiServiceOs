#include "Utils.h"

void criarFIFO(const char *fifo_nome) {
	if (mkfifo(fifo_nome, 0666) < 0) {
		if (errno != EEXIST) {
			perror("Erro ao criar FIFO do cliente");
			exit(EXIT_FAILURE);
		}
	}
}

int abrirFIFO(const char *fifo_nome, bool write) {
	int fd;
	if (write)
		fd = open(fifo_nome, O_WRONLY);
	else
		fd = open(fifo_nome, O_RDONLY);

	if (fd < 0) {
		perror("Erro ao abrir FIFO");
		unlink(fifo_nome);
		exit(EXIT_FAILURE);
		return -1;
	}

	return fd;
}

Servico criarServico(char *hora, char *local, char *distancia) {
	Servico servico;
	
	strncpy(servico.local, local, sizeof(servico.local) - 1);
	servico.local[sizeof(servico.local) - 1] = '\0'; // Garantir terminação nula

	servico.hora = atoi(hora);

	servico.distancia = atof(distancia);

	return servico;
}

bool verificaUserName(int num_clientes, Cliente clientes[], char *username)
{
  for (int i = 0; i < num_clientes; i++)
    if (strcmp(clientes[i].username, username) == 0)
      return true;

  return false;
}

Agendamento *encontrarAgendamento(Agendamento agendamentos[], Cliente cliente, int num_agendamentos, int id)
{
  Agendamento *encontrado = NULL;
  for (int i = 0; i < num_agendamentos; i++)
  {
    if (agendamentos[i].ativo &&
        agendamentos[i].id == id &&
        agendamentos[i].cliente.pid_cliente == cliente.pid_cliente)
    {
      encontrado = &agendamentos[i];
      break;
    }
  }
  return encontrado;
}