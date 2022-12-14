#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	// Your code here
	if (strcmp(cmd, "exit") == 0)
		return 1;  // es un true en runcmd.c

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	// Your code here
	char *dir;
	if (strcmp(cmd, "cd") == 0) {
		// deberia soportar n espacios (el parser se deberia encargar de eso, no el builtin)
		dir = getenv("HOME");
	} else if (strncmp(cmd, "cd ", 3) == 0) {
		dir = cmd + 3;
	} else {
		return 0;
	}
	int result = chdir(dir);

	if (result < 0) {
		perror("error al cambiar de directorio");
		return 0;
	} else {
		// actualizo el prompt
		snprintf(prompt, sizeof(prompt), "%s", getcwd(dir, PRMTLEN));  // escribo el directorio actual en el prompt
		return 1;
		/*
		a tener en cuenta:
		getcwd nunca va a alocar memoria (dir no es null)
		por lo que no hace falta hacer free (ver abajo built-in pwd)
		printf("prompt1: %s\n", dir);
		//path relativo (salvo para cd a home) printf("prompt2: %s\n",
		getcwd(dir, PRMTLEN));	//path absoluto, dado path relativo
		printf("prompt3: %s\n", getcwd(NULL, 0));		//aloca
		memoria dinamicamente
		*/
	}
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	// Your code here
	if (strcmp(cmd, "pwd") == 0) {
		char *pwd = getcwd(NULL, 0);
		printf("%s", pwd);
		free(pwd);
		return 1;

		// otra forma era:   char cwd[1024];	printf("%s", getcwd(cwd, 1024));
	}
	return 0;

	// The getcwd() function copies an absolute pathname of the current
	// working directory to the array  pointed  to by buf, which is of
	// length size getcwd() allocates the buffer dynamically using malloc(3)
	// if buf is NULL. si size es 0, entonces el buffer se alocará con el
	// tamaño necesario The caller should free(3) the returned buffer.
}
