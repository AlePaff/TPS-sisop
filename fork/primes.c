#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void primos(int fd);

// recibe el fd (un nro.) por donde el padre le manda mensajes al hijo
void
primos(int fd1)
{
	int primo;

	if (read(fd1, &primo, sizeof(primo))) {  // lee el nro. del padre
		printf("primo %d\n", primo);

		// pipe que se usa para comunicar al hijo del hijo
		int fd2[2];
		int p2 = pipe(fd2);
		if (p2 < 0) {
			perror("Error en pipe");
			exit(-1);
		}

		int i2 = fork();
		if (i2 < 0) {
			perror("Error en fork");
			exit(-1);
		}

		if (i2 == 0) {  // hijo del hijo
			close(fd1);
			close(fd2[1]);  // el hijo solo va a leer
			primos(fd2[0]);
			close(fd2[0]);
		} else {
			// el padre es quien va a filtrar los numeros primos, el hijo crea los filtros
			close(fd2[0]);
			int aux;
			while (read(fd1,
			            &aux,
			            sizeof(aux))) {  // va al siguiente primo porque
				                     // en el read de arriba ya leyo el primero
				//
				if (aux % primo != 0) {
					int w = write(fd2[1], &aux, sizeof(aux));
					if (w < 0) {
						perror("Error en write");
						exit(-1);
					}
				}
			}
			close(fd2[1]);
			close(fd1);
			wait(NULL);
		}
	}
	close(fd1);  // si no hay nada mas para leer, cierro el pipe izquierdo
	exit(-1);
}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Cantidad de argumentos inválida (solo 1)");
		exit(-1);
	}

	int fds[2];
	int p = pipe(fds);
	if (p < 0) {
		perror("Error en pipe");
		exit(-1);
	}

	int i = fork();
	if (i < 0) {
		perror("Error en fork");
		exit(-1);
	}

	if (i == 0) {
		close(fds[1]);
		primos(fds[0]);  // va a leer lo que haya escrito el padre
	} else {
		close(fds[0]);

		// el primer proceso escribe en el pipe (2, 3, 4, 5, ..., n)
		int n = atoi(argv[1]);  // atoi convierte un string (ej. "42") a un int (42)
		for (int i = 2; i <= n; i++) {
			int w = write(fds[1], &i, sizeof(i));
			if (w < 0) {
				perror("Error en write");
				exit(-1);
			}
		}
		close(fds[1]);
		wait(NULL);
	}
	exit(0);
}
