#include "Utils.h"

int main(int argc, char *argv[]) {
	if (argc != 5) {
		printf("Sintaxe: %s <hora> <local> <distancia> <utilizador>\n", argv[0]);
		return 1;
	}
  
	char user[MAX_CHARACTERS];
	Servico servico = criarServico(argv[1], argv[2], argv[3]);
	strcpy(user, argv[4]);

	printf("Modulo Veiculo compilado com sucesso.\n");
	return 0;
}