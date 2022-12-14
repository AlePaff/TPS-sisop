#include "defs.h"
#include "readline.h"
#include "runcmd.h"
#include "types.h"

char prompt[PRMTLEN] = { 0 };  // las llaves son para inicializar el array a 0

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) !=
	       NULL)  // mientras haya comandos (leer linea de la shell)
		if (run_cmd(cmd) == EXIT_SHELL)  // ejecutar comando
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
