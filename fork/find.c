#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_PATH 1024
#define FLAG_i "-i"

char *sensitividad(char *nombre, char *needle, int i);
void recursiva(int fd_directorio, char *nombre_dir, char *needle, int i);

char *
sensitividad(char *nombre, char *needle, int i)
{
	if (i == 1)
		return strcasestr(nombre, needle);
	return strstr(nombre, needle);
}


void
recursiva(int fd_directorio, char *nombre_dir, char *needle, int i)
{
	DIR *dir_actual = fdopendir(fd_directorio);
	if (dir_actual == NULL) {
		perror("error con fdopendir");
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir_actual))) {  // leo el dir. actual (mientras no devuelva NULL)
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 ||
			    strcmp(entry->d_name, "..") == 0)
				continue;

			char path[PATH_MAX];
			snprintf(path,
			         sizeof(path),
			         "%s/%s",
			         nombre_dir,
			         entry->d_name);  // me ahorro usar strcat()
			if (sensitividad(entry->d_name, needle, i) != NULL) {
				printf("%s\n", path);
			}

			int fd_nuevo = openat(
			        fd_directorio,
			        entry->d_name,
			        O_DIRECTORY);  // apertura relativa a partir de directory
			recursiva(fd_nuevo, path, needle, i);
			close(fd_nuevo);
			continue;
		}

		if (entry->d_type == DT_REG) {
			if (sensitividad(entry->d_name, needle, i) != NULL) {
				printf("%s/%s\n", nombre_dir, entry->d_name);
			}
			continue;
		}
		printf("%s es otro tipo de archivo\n", entry->d_name);
	}
}


int
main(int argc, char *argv[])
{
	char *needle;
	char *path = ".";
	int i;

	if ((argc < 2) || (argc > 3)) {
		printf("cantidad de argumentos inválida\n");
		return -1;
	}

	// printf("palabra buscada: %s\n\n", needle);
	DIR *directory = opendir(path);  // abro el directorio actual
	if (directory == NULL) {
		perror("error con opendir");
		return (-1);
	}

	if ((argc == 3) && (strcmp(argv[1], FLAG_i) == 0)) {  // case sensitive
		i = 1;
		needle = argv[2];
		recursiva(dirfd(directory), path, needle, i);
	}

	if (argc == 2) {  // case insensitive
		i = 0;
		needle = argv[1];
		recursiva(dirfd(directory), path, needle, i);
	}

	closedir(directory);
	return 0;
}
