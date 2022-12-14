#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//  idx = 3
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here (* = key or value)
//
// la funcion recibe un arreglo de strings y un entero que es la cantidad de strings
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	for (int i = 0; i < eargc; i++) {
		char key[BUFLEN];
		char value[BUFLEN];
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, block_contains(eargv[i], '='));  // block_contains devuelve el indice donde esta el '='
		setenv(key,
		       value,
		       1);  // 1 es para que sobreescriba la variable si ya existe
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
// S_IRUSR y S_IWUSR son los permisos de lectura y escritura para el usuario
// int open(const char *pathname, int flags, mode_t mode);  	//open(2)

static int
open_redir_fd(char *file, int flags)
{
	// Your code here
	if (flags &
	    O_CREAT)  //& es un AND bit a bit (true = cualquier nro, false = 0)
		return open(file, flags, S_IWUSR | S_IRUSR);
	else
		return open(file, flags);
}


// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		// int execvp(const char *file, char *const argv[]);
		// (ver execvp(3)) provide an array of pointers  to
		// null-terminated  strings that represent the argument list
		// available to the new program. Returns -1 on error

		// Your code here

		e = (struct execcmd *) cmd;  // casteo el cmd a un execcmd (ver types.h)
		set_environ_vars(e->eargv,
		                 e->eargc);  // seteo las variables de entorno
		int salida = execvp(e->argv[0], e->argv);  // ejecuto el comando
		if (salida < 0) {  // si no se pudo ejecutar
			perror("error en execvp: ");
			exit(-1);
		}
		break;
	}

	case BACK: {
		// runs a command in background
		//
		// Your code here
		b = (struct backcmd *) cmd;  // casteo el cmd a un backcmd (ver types.h)
		int i = fork();
		if (i < 0) {
			perror("Error en fork");
			exit(-1);
		}

		if (i == 0) {
			exec_cmd(b->c);  // ejecuto el comando que va en segundo plano
		}
		int wstatus;
		waitpid(i,
		        &wstatus,
		        WNOHANG);  // espero a que termine el proceso hijo y no
		                   // bloquea la shell, WNOHANG return immediately if no child has exited
		printf_debug("PID = %d\n", i);
		_exit(EXIT_SUCCESS);  // salgo del proceso hijo, con el _ porque
		                      // no quiero que se ejecute el exit del padre

		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		// Your code here
		r = (struct execcmd *) cmd;

		if (strlen(r->in_file) > 0) {  // redirijo la entrada (stdin = 0)
			int fd_in = open_redir_fd(
			        r->in_file,
			        O_RDONLY);  // el | es un OR bit a bit (O_RDONLY
			                    // = 0x0000, O_CREAT = 0x0100), observar que es unica la combinacion
			if (fd_in < 0) {
				perror("Error en open (IN)");
				exit(-1);
			}
			dup2(fd_in,
			     0);  // tambien podia usar dup2(fd_in, STDIN_FILENO);
			// duplico el fd del archivo de entrada (fd_in) en el fd de stdin (0). Diapo 14 Lab shell ppt
			close(fd_in);
		}
		if (strlen(r->out_file) > 0) {  // redirijo la salida (stdout = 1)
			int fd_out = open_redir_fd(r->out_file,
			                           O_WRONLY | O_CREAT | O_TRUNC);
			if (fd_out < 0) {
				perror("Error en open (OUT)");
				exit(-1);
			}
			dup2(fd_out, 1);
			close(fd_out);
		}
		if (strlen(r->err_file) >
		    0) {  // redirijo la salida de error (stderr = 2)
			// printf_debug("archivo err: %s\n", r->err_file);
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(1, 2);  // redirijo stderr = 2 a stdout = 1
				// otra forma era comparando con "r->err[0] ==
				// '&'" y luego cada caso pero era menos legible
			} else {
				int fd_err = open_redir_fd(r->err_file,
				                           O_WRONLY | O_CREAT |
				                                   O_TRUNC);
				if (fd_err < 0) {
					perror("Error en open (ERR)");
					exit(-1);
				}
				dup2(fd_err, 2);
				close(fd_err);
			}
		}

		// printf_debug("file in: %s\nfile out: %s\nfile err: %s\n",
		// r->in_file, r->out_file, r->err_file);
		r->type = EXEC;              // ahora es un comando "comun"
		exec_cmd((struct cmd *) r);  // solo puedo castear de esta forma,
		                             // sino me tira warning (incompatible pointer types)
		break;
	}

	case PIPE: {
		// pipes two commands
		// recomendacion: un fork inicial para separarlos y luego un
		// fork por cada pipe (ver ppt diapo nro 16) cada uno luego
		// tendrá un exec. Mediante el pipe se comunican ambas partes
		// (der e izq) del comando
		//
		// Your code here
		p = (struct pipecmd *) cmd;
		int fd[2];
		if (pipe(fd) < 0) {
			perror("Error en pipe");
			exit(-1);
		}

		int pid = fork();
		if (pid < 0) {
			perror("Error en fork");
			exit(-1);
		}

		if (pid == 0) {  // hijo izquierdo
			close(fd[0]);
			dup2(fd[1], 1);  // escribe (fd[1]) la salida (1)
			close(fd[1]);    // no es 100% necesario, ya que el dup2
			               // lo hace por mi ( If the file descriptor
			               // newfd was previously open, it is silently closed before being reused.)
			exec_cmd(p->leftcmd);
		} else {  // hijo derecho (o padre) que contiene el resto del arbol de comandos
			close(fd[1]);
			int pid2 = fork();
			if (pid2 < 0) {
				perror("Error en fork");
				exit(-1);
			}

			if (pid2 == 0) {
				dup2(fd[0], 0);  // lee (fd[0]) la entrada (0)
				close(fd[0]);
				exec_cmd(p->rightcmd);
				exit(-1);
			} else {
				close(fd[0]);
				wait(NULL);  // o bien waitpid(pid, &wstatus //un entero, 0);
				wait(NULL);
			}
		}
		// free the memory allocated for the pipe tree structure
		// free_command(parsed_pipe);		//me tira doble free, se puede quitar (DISCORD)
		break;
	}
	}
}
