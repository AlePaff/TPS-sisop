#include <stdio.h>   //printf(), perror()
#include <unistd.h>  //fork(), pipe()
#include <stdlib.h>  //exit()
#include <time.h>    //time()

int
main(void)
{
	int fd1[2];
	int fd2[2];
	int nro_random;
	time_t t;
	srandom((unsigned) time(
	        &t));  // Intializes random number generator, para que sea usado por random()

	if (pipe(fd1) == -1) {
		fprintf(stderr, "Falló el pipe 1");
		return 1;
	}
	if (pipe(fd2) == -1) {
		fprintf(stderr, "Falló el pipe 2");
		return 1;
	}

	printf("Hola, soy PID %d:\n", getpid());
	printf("  - primer pipe me devuelve: [%d, %d]\n", fd1[0], fd1[1]);
	printf("  - segundo pipe me devuelve: [%d, %d]\n\n", fd2[0], fd2[1]);

	int i = fork();

	if (i < 0) {
		perror("Error en fork!");
		exit(1);
	}

	if (i == 0) {
		// rama hijo
		close(fd1[1]);  // cierro el extremo de escritura del pipe1
		if (read(fd1[0], &nro_random, sizeof(nro_random)) == -1) {
			perror("Error en read");
			exit(1);
		}

		printf("Donde fork me devuelve %d:\n", i);  // i = 0
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());
		printf("  - recibo valor %d vía fd=%d\n", nro_random, fd1[0]);
		printf("  - reenvío valor en fd=%d y termino\n", fd2[1]);

		close(fd1[0]);  // cierro el extremo de lectura del pipe1
		close(fd2[0]);  // cierro el extremo de lectura del pipe2
		if (write(fd2[1], &nro_random, sizeof(nro_random)) == -1) {
			perror("Error en write");
			exit(1);
		}               // escribo en el pipe2
		close(fd2[1]);  // cierro el extremo de escritura del pipe2


	} else {
		// rama padre
		nro_random = random();
		printf("Donde fork me devuelve %d:\n", i);
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());
		printf("  - random me devuelve: %d\n", nro_random);
		printf("  - envío valor %d a través de fd=%d\n\n",
		       nro_random,
		       fd1[1]);
		close(fd1[0]);  // cierro el extremo de lectura del pipe1
		if (write(fd1[1], &nro_random, sizeof(nro_random)) == -1) {
			perror("Error en write");
			exit(1);
		}
		close(fd1[1]);  // cierro el extremo de escritura del pipe1

		close(fd2[1]);  // cierro el extremo de escritura del pipe2
		if (read(fd2[0], &nro_random, sizeof(nro_random)) == -1) {
			perror("Error en read");
			exit(1);
		}
		printf("\nHola, de nuevo PID %d:\n", getpid());
		printf("  - recibí valor %d vía fd=%d\n", nro_random, fd2[0]);
		close(fd2[0]);  // cierro el extremo de lectura del pipe2
	}

	return 0;
}
