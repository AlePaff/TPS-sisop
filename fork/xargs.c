#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef NARGS
#define NARGS 4
#endif
// si no se define NARGS, se toma el valor por defecto 4

void ejecutar_comando(char *argumentos[]);
void liberar_argumentos(char *argumentos[], int i);

void
ejecutar_comando(char *argumentos[])
{
	int i = fork();
	if (i < 0) {
		perror("Error en fork");
		exit(-1);
	}

	if (i == 0) {
		execvp(argumentos[0],
		       argumentos);  // ejecuta el comando (argv[0]) con sus
		                     // argumentos (argv[1], argv[2], ...)
	} else {
		wait(NULL);  // espera a que termine el hijo
	}
}

// libera la memoria de los argumentos que se van a usar en el comando
void
liberar_argumentos(char *argumentos[], int i)
{
	for (int j = 1; j < i; j++) {
		free(argumentos[j]);
	}
}

int
main(int argc, char *argv[])
{
	(void) argc;  // para el warning: unused parameter 'argc'
	size_t n = 0;
	ssize_t read = 0;
	char *linea = NULL;
	char *argumentos[NARGS + 2] = {
		argv[1]
	};  // los brackets son para inicializar el array
	int i = 1;

	while ((read = getline(&linea, &n, stdin)) != -1) {
		// getline lee de stdin y guarda en line, la cantidad de bytes leidos es n
		if (i == (NARGS + 1)) {  // si llego al limite de argumentos
			argumentos[i] = NULL;
			ejecutar_comando(argumentos);
			liberar_argumentos(argumentos, i);
			i = 1;  // reinicio el contador
		}
		linea[strlen(linea) - 1] =
		        '\0';  // borro el salto de linea \n del final
		argumentos[i] = strdup(linea);
		i++;
	}

	if (i > 1) {  // si quedaron elementos sin evaluar
		argumentos[i] = NULL;
		ejecutar_comando(argumentos);
		liberar_argumentos(argumentos, i);
	}

	free(linea);
	exit(0);
}
