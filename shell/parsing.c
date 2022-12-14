#include "parsing.h"

extern int status;
// parses an argument of the command stream input
static char *
get_token(char *buf, int idx)
{
	char *tok;
	int i;

	tok = (char *) calloc(ARGSIZE, sizeof(char));
	i = 0;

	while (buf[idx] != SPACE && buf[idx] != END_STRING) {
		tok[i] = buf[idx];
		i++;
		idx++;
	}

	return tok;
}

// parses and changes stdin/out/err if needed
static bool
parse_redir_flow(struct execcmd *c, char *arg)
{
	int inIdx, outIdx;

	// flow redirection for output
	if ((outIdx = block_contains(arg, '>')) >= 0) {
		switch (outIdx) {
		// stdout redir
		case 0: {
			strcpy(c->out_file, arg + 1);
			break;
		}
		// stderr redir
		case 1: {
			strcpy(c->err_file, &arg[outIdx + 1]);
			break;
		}
		}

		free(arg);
		c->type = REDIR;

		return true;
	}

	// flow redirection for input
	if ((inIdx = block_contains(arg, '<')) >= 0) {
		// stdin redir
		strcpy(c->in_file, arg + 1);

		c->type = REDIR;
		free(arg);

		return true;
	}

	return false;
}

// parses and sets a pair KEY=VALUE
// environment variable
static bool
parse_environ_var(struct execcmd *c, char *arg)
{
	// sets environment variables apart from the
	// ones defined in the global variable "environ"
	if (block_contains(arg, '=') > 0) {
		// checks if the KEY part of the pair
		// does not contain a '-' char which means
		// that it is not a environ var, but also
		// an argument of the program to be executed
		// (For example:
		// 	./prog -arg=value
		// 	./prog --arg=value
		// )
		if (block_contains(arg, '-') < 0) {
			c->eargv[c->eargc++] = arg;
			return true;
		}
	}

	return false;
}

// this function will be called for every token (palabras separadas por
// espacios), and it should expand environment variables. In other words, if the
// token happens to start with '$', the correct substitution with the environment
// value should be performed. Otherwise the same token is returned. If the
// variable does not exist, an empty string should be returned within the token
//
// Hints:
// - check if the first byte of the argument contains the '$'
// - expand it and copy the value in 'arg'
// - remember to check the size of variable's value
//		It could be greater than the current size of 'arg'
//		If that's the case, you should realloc 'arg' to the new size.
static char *
expand_environ_var(char *arg)
{
	// Your code here
	// ej. si escribí "echo $TERM" en la consola, me repite dos veces. La
	// primera "echo" y la segunda "$TERM"
	// printf("expand_environ_var: %s\n", arg);

	if (strcmp(arg, "$?") == 0) {
		sprintf(arg,
		        "%i",
		        status);  // escribo el entero status, en la cadena arg
		return arg;
	}
	if (arg[0] == '$') {
		char *env = getenv(
		        arg +
		        1);  // suma una posicion del puntero al str para que no me tome el $
		if (env != NULL) {
			int len = strlen(env);
			if (len >
			    ARGSIZE) {  // si el tamaño del env es mayor al tamaño del arg, realloc
				arg = realloc(arg, len);
			}
			strcpy(arg, env);  // copia env en arg
		} else {
			arg[0] = '\0';  // si no existe la variable, devuelve un string vacío
		}
	}

	return arg;
}

// parses one single command having into account:
// - the arguments passed to the program
// - stdin/stdout/stderr flow changes
// - environment variables (expand and set)
static struct cmd *
parse_exec(char *buf_cmd)
{
	struct execcmd *c;
	char *tok;
	int idx = 0, argc = 0;

	c = (struct execcmd *) exec_cmd_create(buf_cmd);


	while (buf_cmd[idx] !=
	       END_STRING) {  // mientras no sea el final de la linea
		tok = get_token(buf_cmd, idx);
		idx = idx + strlen(tok);

		if (buf_cmd[idx] != END_STRING)
			idx++;

		if (parse_redir_flow(c, tok))
			continue;

		if (parse_environ_var(c, tok))
			continue;

		tok = expand_environ_var(tok);

		if (strlen(tok) > 0) {  // DISCORD
			c->argv[argc++] = tok;
		}
	}

	c->argv[argc] = (char *) NULL;
	c->argc = argc;

	return (struct cmd *) c;
}

// parses a command knowing that it contains the '&' char
static struct cmd *
parse_back(char *buf_cmd)
{
	int i = 0;
	struct cmd *e;

	while (buf_cmd[i] != '&')
		i++;

	buf_cmd[i] = END_STRING;

	e = parse_exec(buf_cmd);

	return back_cmd_create(e);
}

// parses a command and checks if it contains the '&'
// (background process) character
static struct cmd *
parse_cmd(char *buf_cmd)
{
	if (strlen(buf_cmd) == 0)
		return NULL;

	int idx;

	// checks if the background symbol is after
	// a redir symbol, in which case
	// it does not have to run in in the 'back'
	if ((idx = block_contains(buf_cmd, '&')) >= 0 && buf_cmd[idx - 1] != '>')
		return parse_back(buf_cmd);

	return parse_exec(buf_cmd);
}

// parses the command line
// looking for the pipe character '|'
struct cmd *
parse_line(char *buf)
{
	struct cmd *r, *l;

	char *right = split_line(buf, '|');

	// para que soporte pipes multiples se debe reparsear el lado derecho (de acuerdo a mi implementacion en exec.c)
	if (block_contains(right, '|') >= 0) {
		r = parse_line(right);
	} else {
		r = parse_cmd(right);
	}
	l = parse_cmd(buf);

	return pipe_cmd_create(l, r);
}
