#include <sys/stat.h>
#include <time.h>
#define CANT_INODOS 100
#define BLOCK_DATA_NUMBER 400
#define DATABLOCKMAX 4		// cantidad de bloques de datos que puede tener un inodo
#define BLOCKSIZE 4096		// tamaño de cada bloque
#define MAX_NOMBRE 60 //largo de nombre de archivo
#define MAX_NOMBRE_DIR 55 //largo del path de un directorio
#define OCUPADO 1
#define LIBRE 0
#define DIR_T 0			//tipo de archivo directorio
#define REG_T 1			//tipo de archivo regular


struct bloque {
	char contenido[BLOCKSIZE];
};


struct inodo {
	int inumero;
	off_t tam;
	pid_t pid;
	mode_t permisos; 		//permisos => rwx(owner)r-x(group)r-x(other) con r=read, w=write, x=execute
	time_t fecha_de_creacion;
	time_t fecha_de_modificacion;
	time_t fecha_de_acceso;
	char nombre[MAX_NOMBRE];
	uid_t st_uid;
	int indice_bloques[DATABLOCKMAX];  //id de cada bloque de data, si es negativo no se usa
	struct bloque *datos[DATABLOCKMAX];	//apunta a los bloques de data (un inodo puede tener mas de un bloque de data)
	int tipo_archivo; 	//0 es directorio, 1 es archivo regular
};

struct superbloque {
	int numero_magico;		//sirve para identificar el sistema de archivos
	int cant_inodes;
	int cant_bloques_data;
	int* bitmap_data;			//indica si un bloque esta ocupado o no
	int* bitmap_inodes;
};


/*
Descripcion de las constantes de error

 ENOENT          No such file or directory (POSIX.1-2001).

                       Typically, this error results when a specified pathname
                       does not exist, or one of the components in the  direc‐
                       tory prefix of a pathname does not exist, or the speci‐
                       fied pathname is a dangling symbolic link.
EFBIG           File too large (POSIX.1-2001).
EADDRNOTAVAIL   Address not available (POSIX.1-2001).
EDESTADDRREQ    Destination address required (POSIX.1-2001).
EEXIST          File exists (POSIX.1-2001).
EFAULT          Bad address (POSIX.1-2001).
EINVAL          Invalid argument (POSIX.1-2001).
EISDIR          Is a directory (POSIX.1-2001).
EMSGSIZE        Message too long (POSIX.1-2001).
ENAMETOOLONG    Filename too long (POSIX.1-2001).
ENFILE          Too  many  open  files  in  system  (POSIX.1-2001).  On
                       Linux, this is probably a result  of  encountering  the
                       /proc/sys/fs/file-max limit (see proc(5)).
ENOBUFS         No   buffer   space  available  (POSIX.1  (XSI  STREAMS
                       option)).
ENOLCK          No locks available (POSIX.1-2001).
ENOMEM          Not enough space/cannot allocate memory (POSIX.1-2001).
ENOSPC          No space left on device (POSIX.1-2001).
ENOSYS          Function not implemented (POSIX.1-2001).
ENOTDIR         Not a directory (POSIX.1-2001).
ENOTEMPTY       Directory not empty (POSIX.1-2001).
ERANGE          Result too large (POSIX.1, C99).
*/