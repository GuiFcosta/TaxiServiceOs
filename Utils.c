#include "Utils.h"

void criarFIFO(const char *fifo_nome)
{
    if (mkfifo(fifo_nome, 0666) < 0)
    {
        if (errno != EEXIST)
        {
            perror("Erro ao criar FIFO do cliente");
            exit(EXIT_FAILURE);
        }
    }
}

int abrirFIFO(const char *fifo_nome, bool write)
{
    int fd;
    if (write)
        fd = open(fifo_nome, O_WRONLY);
    else
        fd = open(fifo_nome, O_RDONLY);

    if (fd < 0)
    {
        perror("Erro ao abrir FIFO");
        unlink(fifo_nome);
        exit(EXIT_FAILURE);
    }
    return fd;
}