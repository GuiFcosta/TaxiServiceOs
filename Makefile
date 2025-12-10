# Makefile
CC=gcc
CFLAGS=-Iinclude -Wall -Wextra -g -pthread
SRCDIR=src

CLIENTE=Cliente
CONTROLADOR=Controlador
VEICULO=Veiculo

OBJS_COMMON=$(SRCDIR)/Utils.o
OBJS_CLIENT=$(SRCDIR)/Cliente.o
OBJS_CONTROL=$(SRCDIR)/Controlador.o
OBJS_VEICULO=$(SRCDIR)/Veiculo.o


.PHONY: all clean

all: $(CONTROLADOR) $(CLIENTE) $(VEICULO)

$(CONTROLADOR): $(OBJS_CONTROL) $(OBJS_COMMON)
	$(CC) -o $@ $^

$(CLIENTE): $(OBJS_CLIENT) $(OBJS_COMMON)
	$(CC) -o $@ $^

$(VEICULO): $(OBJS_VEICULO) $(OBJS_COMMON)
	$(CC) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(CLIENTE) $(CONTROLADOR) $(VEICULO)