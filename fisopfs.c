#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include "types.h"
#include "funciones.c"

struct superbloque SB;
struct bloque bloques_data[BLOCK_DATA_NUMBER];
int bitmap_data[BLOCK_DATA_NUMBER];
struct inodo inodos[CANT_INODOS];
int bitmap_inodo[CANT_INODOS];

// nombre default del archivo que guarda el filesystem
char *archivo = "myfilesystem.fisopfs";

// funciones auxiliares
int buscar_inodo(const char *path);
int crear_archivo(const char *path, mode_t mode, int tipo);
int contar(const char *nombre);
int buscar_barra(const char *nombre);
int comp_str(const char *string1, char *string2);
void inicializar_archivo();
void levantar_archivo(FILE *file);
void guardar_archivo();
int buscar_bloque_libre(int indice, int offset);
int minimo(int a, int b);
int contar_bloques(int array[]);
void update_time(const char *path);

static void *
fisop_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	printf("[debug] fisop_init\n");
	FILE *file;
	file = fopen(archivo, "r");
	if (!file) {
		inicializar_archivo();
	} else {
		levantar_archivo(file);
		fclose(file);
	}
	void *resultado = 0;
	return resultado;
}

// devuelve los atributos de un archivo, dado un path y
// un struct stat (contiene los atributos de un archivo, como uid, permisos, etc)
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr(%s)\n", path);

	if (strcmp(path, "/") == 0) {  // si el path es el root
		st->st_uid = 1717;
		st->st_mode =
		        __S_IFDIR |
		        0755;  // 775 = rwx(owner)r-x(group)r-x(other) con r=read, w=write, x=execute
		st->st_nlink = 2;
	} else {
		// buscar el inode con el path y devolver sus datos de manera parecida al ejemplo
		int indice_inodo = buscar_inodo(path);
		if (indice_inodo < 0) {
			return -ENOENT;
		}
		st->st_uid = inodos[indice_inodo].st_uid;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
		if (inodos[indice_inodo].tipo_archivo == REG_T) {
			st->st_size = inodos[indice_inodo].tam;
			st->st_mode = __S_IFREG | 0644;
			st->st_nlink = 1;
			st->st_blksize = BLOCKSIZE;
			st->st_blocks = contar_bloques(
			        inodos[indice_inodo].indice_bloques);
		}
		st->st_atime = inodos[indice_inodo].fecha_de_acceso;
		st->st_mtime = inodos[indice_inodo].fecha_de_modificacion;
		st->st_ctime = inodos[indice_inodo].fecha_de_modificacion;
	}
	return 0;
}

// readdir lee el contenido de un DIRECTORIO, dado un path y un buffer donde llenar el contenido (ej de uso. ls)
static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir(%s)\n", path);
	// llena en el buffer el contenido de . (directorio actual) y .. (directorio padre)
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	if (strcmp(path, "/") == 0) {
		for (int i = 0; i < CANT_INODOS; i++) {
			if (SB.bitmap_inodes[i] == OCUPADO &&
			    contar(inodos[i].nombre) == 1)
				filler(buffer, inodos[i].nombre + 1, NULL, 0);
		}
		return 0;
	}
	int barritas = contar(path);
	for (int i = 0; i < CANT_INODOS; i++) {
		if (SB.bitmap_inodes[i] == OCUPADO &&
		    comp_str(path, inodos[i].nombre))
			if (contar(inodos[i].nombre) == barritas + 1)
				filler(buffer,
				       inodos[i].nombre +
				               buscar_barra(inodos[i].nombre),
				       NULL,
				       0);
	}
	return 0;

	// Los directorios '.' y '..'
	filler(buffer,
	       ".",
	       NULL,
	       0);  // llena en el buffer el contenido de . (directorio actual)
	filler(buffer, "..", NULL, 0);

	// busco el inodo del directorio
	int indice = buscar_inodo(path);
	if (indice < 0) {
		return -ENOENT;
	}
	// struct inodo inodo_dir = inodos[indice];
	// Si encuentro algo pero no es un directorio, lanzo una excepcion
	if (inodos[indice].tipo_archivo == REG_T) {
		return -ENOTDIR;
	}
	filler(buffer, inodos[indice].nombre, NULL, 0);
	return 0;
}


#define MAX_CONTENIDO

// lee el contenido de un ARCHIVO
// Read size bytes from the given file into the buffer buf, beginning offset
// bytes into the file Returns the number of bytes transferred, or 0 if offset
// was at or beyond the end of the file.
static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read(%s, %lu, %lu)\n", path, offset, size);

	int indice = buscar_inodo(path);
	if (indice < 0) {
		return -ENOENT;
	}
	// printf("sizeof(buffer) es %ld\n",sizeof(buffer));
	inodos[indice].fecha_de_acceso = time(NULL);
	while ((offset) <
	       (BLOCKSIZE *
	        DATABLOCKMAX))  // que no se pase de los 4 bloques de memoria por archivo
		if (inodos[indice].indice_bloques[offset / BLOCKSIZE] >= 0) {
			memcpy(buffer,
			       inodos[indice].datos[offset / BLOCKSIZE]->contenido +
			               (offset % BLOCKSIZE),
			       minimo(size, BLOCKSIZE - offset % BLOCKSIZE));
			if (size == minimo(size, BLOCKSIZE - offset % BLOCKSIZE)) {
				return minimo(size,
				              BLOCKSIZE - offset % BLOCKSIZE);
			} else {
				size = size -
				       minimo(size,
				              BLOCKSIZE - offset % BLOCKSIZE);
				offset = (offset + BLOCKSIZE) -
				         (offset % BLOCKSIZE);
			}
		}
	return 0;
	char *contenido = inodos[indice].datos[0]->contenido;
	if (offset + size > strlen(contenido)) {
		size = strlen(contenido) - offset;
	}
	size = size > 0 ? size : 0;
	strncpy(buffer, contenido + offset, size);
	return size;
}

static int
fisopfs_createfiles(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_createfiles(%s)\n", path);
	if (strlen(path) > (MAX_NOMBRE)) {
		errno = ENAMETOOLONG;
		return ENAMETOOLONG;
	}
	int valor = crear_archivo(path, mode, REG_T);
	return valor;
}

static int
fisop_createdir(const char *path, mode_t mode)
{
	printf("[debug] fisop_createdir(%s) con modo: %d\n", path, mode);
	if (strlen(path) >
	    (MAX_NOMBRE_DIR)) {  // comparo con MAX_NOMBRE-5 para que quede espacio
		                 // para el path de un archivo que cree adentro
		errno = ENAMETOOLONG;
		return ENAMETOOLONG;
	}
	int valor = crear_archivo(path, mode, DIR_T);
	return valor;
}

static int
fisop_write(const char *path,
            const char *message,
            size_t size,
            off_t offset,
            struct fuse_file_info *fi)
{
	printf("[debug] fisop_write(%s)\n", path);
	int i = 0;
	int indice = buscar_inodo(path);
	if (indice < 0) {  // no encontré el archivo
		int valor =
		        crear_archivo(path, 0755, REG_T);
		if (valor != 0) {      // falló la creación
			return valor;  // devuelvo el valor de error
		} else {
			indice = buscar_inodo(path);
		}
	}

	// Si encuentro algo pero es un directorio, lanzo una excepcion
	if (inodos[indice].tipo_archivo == DIR_T) {
		errno = EISDIR;
		return -EISDIR;
	}
	if ((inodos[indice].permisos) & (S_IWUSR == 0)) {
		errno = EACCES;
		return EACCES;
	}

	inodos[indice].tam = minimo(offset + size, BLOCKSIZE * DATABLOCKMAX);

	// chequear tamaños y ver si necesita mas
	// bloques ver offset y size (variables)
	inodos[indice].fecha_de_modificacion = time(0);
	while (offset <
	       (BLOCKSIZE *
	        DATABLOCKMAX)) {  // que no se pase de los 4 bloques de memoria por archivo
		printf("entre al while\n");
		if (inodos[indice].indice_bloques[offset / BLOCKSIZE] >= 0) {
			printf("escribo\n");
			memcpy(inodos[indice].datos[offset / BLOCKSIZE]->contenido +
			               (offset % BLOCKSIZE),
			       message,
			       minimo(size, BLOCKSIZE - offset % BLOCKSIZE));
			printf("escribi %s\n",
			       inodos[indice].datos[offset / BLOCKSIZE]->contenido +
			               (offset % BLOCKSIZE));
			return minimo(size, BLOCKSIZE - offset % BLOCKSIZE);
		} else {
			printf("busco obtener un nuevo bloque de data\n");
			if (buscar_bloque_libre(indice, offset / BLOCKSIZE) ==
			    ENOMEM)
				return i;
		}
	}

	if (offset + size > strlen(message)) {
		size = strlen(message) - offset;
	}
	size = size > 0 ? size : 0;
	strncpy(inodos[indice].datos[0]->contenido,
	        message,
	        strlen(message));  // si size no diera negativo, habría que poner size en vez de strlen(message)
	return 0;
}

static int
fisop_removefile(const char *path)
{
	printf("[debug] fisop_removefile(%s)\n", path);

	int indice = buscar_inodo(path);
	if (indice < 0) {  // no encontré el directorio
		errno = ENOENT;
		return -ENOENT;
	}

	// Si encuentro algo pero es un directorio, lanzo una excepcion
	if (inodos[indice].tipo_archivo == DIR_T) {
		errno = EISDIR;
		return -EISDIR;
	}

	// piso los datos con ceros como dijo Juan y marco el inodo como libre
	SB.bitmap_inodes[indice] = LIBRE;
	for (int j = 0; j < DATABLOCKMAX; j++) {
		if (inodos[indice].indice_bloques[j] >= 0) {
			SB.bitmap_data[inodos[indice].indice_bloques[j]] = LIBRE;
			inodos[indice].indice_bloques[j] = -1;  // marco como invalido
			memset(inodos[indice].datos[j], 0, sizeof(struct bloque));
			memset(inodos[indice].nombre, 0, MAX_NOMBRE);
		}
	}
	update_time(path);
	return 0;
}

static int
fisop_removedir(const char *path)
{
	printf("[debug] fisop_removedir(%s)\n", path);

	// buscar el directorio en los inodos
	int indice = buscar_inodo(path);
	if (indice < 0) {  // no encontré el directorio
		errno = ENOENT;
		return -ENOENT;
	}

	// Si encuentro algo pero no es un directorio, lanzo una excepcion
	if (inodos[indice].tipo_archivo == REG_T) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	// Si el directorio no está vacio, elimino todos los archivos a mano y luego borro
	for (int i = 0; i < CANT_INODOS; i++) {
		if (SB.bitmap_inodes[i] == OCUPADO &&
		    comp_str(path, inodos[i].nombre)) {
			if (inodos[i].tipo_archivo == REG_T && contar(inodos[i].nombre)>contar(path))
				fisop_removefile(inodos[i].nombre);
			if (inodos[i].tipo_archivo == DIR_T &&
			    strcmp(inodos[i].nombre, path) > 0)
				fisop_removedir(inodos[i].nombre);
		}
	}

	// si el directorio está vacío
	// piso los datos con ceros y marco el inodo como libre
	SB.bitmap_inodes[indice] = LIBRE;
	// cada bloque de datos como libre
	for (int j = 0; j < DATABLOCKMAX; j++) {
		if (inodos[indice].indice_bloques[j] >= 0) {
			SB.bitmap_data[inodos[indice].indice_bloques[j]] = LIBRE;
			inodos[indice].indice_bloques[j] =
			        -1;  // invalido, de esa forma no se puede acceder a ese bloque
			memset(inodos[indice].datos[j], 0, sizeof(struct bloque));
		}
	}
	// cuando se cree un nuevo archivo se sobreescriben los datos
	// es por si en alguna busqueda se llega a acceder a un bloque
	// que fué borrado (ej. buscar por nombre)
	for (int i = 0; i < MAX_NOMBRE; i++) {
		inodos[indice].nombre[i] = '\0';
	}
	update_time(path);

	inodos[indice].tam = -1;
	inodos[indice].inumero = -1;
	return 0;
}

static int
fisop_truncate(const char *path_archivo, off_t offset, struct fuse_file_info *fi)
{
	printf("[debug] fisop_truncate\n");
	int indice = buscar_inodo(path_archivo);
	if (indice < 0)
		return 0;
	for (int j = offset / BLOCKSIZE; j < DATABLOCKMAX; j++) {
		if (inodos[indice].indice_bloques[j] >= 0) {
			if (j > offset / BLOCKSIZE) {
				memset(inodos[indice].datos[j],
				       0,
				       sizeof(struct bloque));
				SB.bitmap_data[inodos[indice].indice_bloques[j]] =
				        LIBRE;
				inodos[indice].indice_bloques[j] = -1;
				inodos[indice].tam =
				        inodos[indice].tam - BLOCKSIZE;
			} else {
				memset(&(inodos[indice].datos[j]->contenido[offset]),
				       0,
				       (BLOCKSIZE - offset) * sizeof(char));
			}
		}
	}
	return 0;
}

static void
fisop_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");
	guardar_archivo();
}

static int
fisop_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisop_flush\n");
	guardar_archivo();
	return 0;
}

static int
fisop_utimens(const char *path_archivo,
              const struct timespec tv[2],
              struct fuse_file_info *fi)
{
	printf("[debug] fisop_utimens\n");
	int indice = buscar_inodo(path_archivo);
	inodos[indice].fecha_de_acceso = tv[0].tv_sec;
	inodos[indice].fecha_de_modificacion = tv[1].tv_sec;
	return 0;
}


static struct fuse_operations operations = {
	.access = fisop_access,
	.chmod = fisop_chmod,
	.chown = fisop_chown,
	.create = fisopfs_createfiles,
	.destroy = fisop_destroy,
	.getattr = fisopfs_getattr,
	.flush = fisop_flush,
	.fsync = fisop_fsync,
	.init = fisop_init,
	.link = fisop_link,
	.mkdir = fisop_createdir,
	.mknod = fisop_mknod,
	//.open = fisop_open,
	.opendir = fisop_opendir,
	.read = fisopfs_read,
	.readdir = fisopfs_readdir,
	.readlink = fisop_readlink,
	.release = fisop_release,
	.rmdir = fisop_removedir,
	.rename = fisop_rename,
	.statfs = fisop_stats,
	.unlink = fisop_removefile,
	.utimens = fisop_utimens,
	.write = fisop_write,
	///////////////////////////////////
	.symlink = fisop_symlink,
	.truncate = fisop_truncate,
	.setxattr = fisop_setxattr,
	.getxattr = fisop_getxattr,
	.listxattr = fisop_listxattr,
	.removexattr = fisop_removexattr,
	.releasedir = fisop_releasedir,
	.fsyncdir = fisop_fsyncdir,
	.lock = fisop_lock,
	.bmap = fisop_bmap,
	.ioctl = fisop_ioctl,
	.poll = fisop_poll,
	//	.write_buf = fisop_write_buf,
	//.read_buf = fisop_read_buf,
	.flock = fisop_flock,
	.fallocate = fisop_fallocate,
	//.copy_file_range = fisop_copy_file_range,
	//.lseek = fisop_lseek
};

int
main(int argc, char *argv[])
{
	// fuse_main es la funcion que se encarga de montar el filesystem
	return fuse_main(argc, argv, &operations, NULL);
}

/////////////////////////////////////////////////////////////
// Funciones auxiliares
////////////////////////////////////////////////////////////

// Busca la posición en el array de inodos, el inodo que tiene el nombre path
// importante: busca "/un_path" y no "un_path"
int
buscar_inodo(const char *path)
{
	for (int i = 0; i < CANT_INODOS; i++) {
		if (SB.bitmap_inodes[i] == LIBRE) {
			continue;
		}
		if (strcmp(inodos[i].nombre, path) == 0) {
			return i;
		}
	}
	return -1;
}

// crea un archivo (regular o directorio) inicializandolo y asignandole un inodo
// asi como su metadata correspondiente
int
crear_archivo(const char *path, mode_t mode, int tipo)
{
	int i = 0;
	int b = 0;

	// buscar un bloque libre
	while (b < BLOCK_DATA_NUMBER && SB.bitmap_data[b] == OCUPADO) {
		b++;
	}
	if (b >= BLOCK_DATA_NUMBER)
		return -ENOMEM;

	// Buscar un inodo libre
	while (i < CANT_INODOS && SB.bitmap_inodes[i] == OCUPADO) {
		i++;
	}
	if (i >= CANT_INODOS)
		return -ENOMEM;

	// marcarlo como ocupado
	SB.bitmap_inodes[i] = OCUPADO;  // inodo ocupado
	SB.bitmap_data[b] = OCUPADO;    // data ocupado

	// Asignarle toda la info correspondiente
	struct inodo archivo = inodos[i];
	archivo.permisos = mode;
	archivo.fecha_de_creacion = time(NULL);
	archivo.fecha_de_acceso = time(NULL);
	archivo.fecha_de_modificacion = inodos[i].fecha_de_creacion;
	archivo.indice_bloques[0] = b;
	archivo.datos[0] =
	        &bloques_data[b];  // asigna el primer bloque al bloque libre encontrado
	archivo.st_uid = getuid();
	archivo.tam = 0;
	archivo.inumero = i;
	archivo.tipo_archivo = tipo;

	strcpy(archivo.nombre, path);
	inodos[i] = archivo;
	update_time(path);

	return 0;
}

// cuenta cuantas veces aparece la barra '/' en un string (ej. "/prueba/archivo.txt" -> 2)
int
contar(const char *nombre)
{
	int contador = 0;
	int i = 0;
	while (nombre[i] != '\0') {
		if (nombre[i] == '/')
			contador++;
		i++;
	}
	return contador;
}

// buscar la posición de la barra que separa el nombre del directorio padre del
// nombre del archivo (ej. "/prueba/archivo.txt" -> 8)
int
buscar_barra(const char *nombre)
{
	int i = 0;
	int barrita = 0;
	while (nombre[i] != '\0') {
		if (nombre[i] == '/')
			barrita = i;
		i++;
	}
	return (barrita + 1);
}

// Compara dos strings, devuelve 0 si uno está contenido dentro del otro
// Y 1 si son diferentes
int
comp_str(const char *string1, char *string2)
{
	int i = 0;
	while (string1[i] != '\0' && string2[i] != '\0') {
		if (string1[i] != string2[i])
			return 0;
		i++;
	}
	return 1;
}

// inicializa el archivo donde se guardan los datos (superbloque, inodos y
// bloques de datos) y los carga en memoria
void
inicializar_archivo()
{
	int valor = 0;
	struct superbloque *sb;
	struct inodo *inodes;
	struct bloque *info;
	FILE *file;
	file = fopen(archivo, "w");
	sb = calloc(1, sizeof(struct superbloque));
	sb->numero_magico = 413136;
	SB.numero_magico = 413136;
	sb->cant_bloques_data = BLOCK_DATA_NUMBER;
	SB.cant_bloques_data = BLOCK_DATA_NUMBER;
	sb->cant_inodes = CANT_INODOS;
	SB.cant_inodes = CANT_INODOS;
	sb->bitmap_data = calloc(sizeof(int), BLOCK_DATA_NUMBER);
	sb->bitmap_inodes = calloc(sizeof(int), CANT_INODOS);
	SB.bitmap_data = bitmap_data;
	SB.bitmap_inodes = bitmap_inodo;
	inodes = calloc(sizeof(struct inodo), CANT_INODOS);
	info = calloc(sizeof(struct bloque), BLOCK_DATA_NUMBER);
	valor = fwrite(sb, sizeof(struct superbloque), 1, file);
	if (valor < 0) {
		printf("He fallado\n");
	}
	//inicializo bitmaps data
	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		SB.bitmap_data[i] = sb->bitmap_data[i];
		valor = fwrite(
		        &sb->bitmap_data[i], sizeof(sb->bitmap_data[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	//inicio bitmap inodos
	for (int i = 0; i < CANT_INODOS; i++) {
		SB.bitmap_inodes[i] = sb->bitmap_inodes[i];
		valor = fwrite(&sb->bitmap_inodes[i],
		               sizeof(sb->bitmap_inodes[i]),
		               1,
		               file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	//inicializo inodos con su array de bloques
	for (int i = 0; i < CANT_INODOS; i++) {
		for (int j = 0; j < DATABLOCKMAX; j++)
			inodes[i].indice_bloques[j] = -1;
		memcpy(&inodos[i], &inodes[i], sizeof(struct inodo));
		valor = fwrite(&inodes[i], sizeof(struct inodo), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	//inicializo bloques de datos
	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		memcpy(&bloques_data[i], &info[i], sizeof(struct bloque));
		valor = fwrite(&info[i],
		               sizeof(struct bloque),
		               1,
		               file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	// libero memoria y cierro archivo
	free(sb->bitmap_data);
	free(sb->bitmap_inodes);
	free(inodes);
	free(info);
	free(sb);
	fclose(file);
}

// si el archivo donde se guardan los datos en disco existe, lo carga en memoria
void
levantar_archivo(FILE *file)
{
	int valor = 0;
	valor = fread(&SB, sizeof(SB), 1, file);
	if (valor < 0) {
		printf("He fallado\n");
	}

	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		valor = fread(&bitmap_data[i], sizeof(bitmap_data[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	for (int i = 0; i < CANT_INODOS; i++) {
		valor = fread(&bitmap_inodo[i], sizeof(bitmap_inodo[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	SB.bitmap_data = bitmap_data;
	SB.bitmap_inodes = bitmap_inodo;
	for (int i = 0; i < CANT_INODOS; i++) {
		valor = fread(&inodos[i], sizeof(inodos[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
		for (int j = 0; j < DATABLOCKMAX; j++)
			if (inodos[i].indice_bloques[j] >= 0)
				inodos[i].datos[j] =
				        &bloques_data[inodos[i].indice_bloques[j]];
	}
	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		valor = fread(&bloques_data[i], sizeof(bloques_data[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
}

// guarda los datos del file system en disco
void
guardar_archivo()
{
	int valor = 0;
	FILE *file;
	file = fopen(archivo, "w");
	valor = fwrite(&SB, sizeof(SB), 1, file);
	if (valor < 0) {
		printf("He fallado\n");
	}
	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		valor = fwrite(&bitmap_data[i], sizeof(bitmap_data[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	for (int i = 0; i < CANT_INODOS; i++) {
		valor = fwrite(&bitmap_inodo[i], sizeof(bitmap_inodo[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	for (int i = 0; i < CANT_INODOS; i++) {
		valor = fwrite(&inodos[i], sizeof(inodos[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
	for (int i = 0; i < BLOCK_DATA_NUMBER; i++) {
		valor = fwrite(&bloques_data[i], sizeof(bloques_data[i]), 1, file);
		if (valor < 0) {
			printf("He fallado\n");
		}
	}
}

// busca en el array de bloques de datos un bloque libre y lo asigna al archivo
int
buscar_bloque_libre(int indice, int offset)
{
	int i = 0;
	while (i < BLOCK_DATA_NUMBER) {
		if (SB.bitmap_data[i] == LIBRE) {
			SB.bitmap_data[i] = OCUPADO;
			inodos[indice].indice_bloques[offset] = i;
			inodos[indice].datos[offset] = &bloques_data[i];
			return 0;
		}
		i++;
	}
	return ENOMEM;
}

// busca el minimo entre dos numeros
int
minimo(int a, int b)
{
	if (a < b)
		return a;
	return b;
	// return a < b ? a : b;
}

// esta funcion es para contar cuantos bloques de memoria esta usando un inodo
int
contar_bloques(int array[])
{
	int contador = 0;
	for (int i = 0; i < DATABLOCKMAX; i++) {
		if (array[i] >= 0)
			contador++;
	}
	return contador;
}

// dado un path actualiza la fecha de modificacion del inodo
void
update_time(const char *path)
{
	if (contar(path) > 1) {  // sino no estoy en el dir. raiz
		// busco el nombre del directorio padre (ej. "/prueba/archivo.txt" -> "/prueba")
		int indice_barra = buscar_barra(path) - 1;
		char nombre_directorio_padre[MAX_NOMBRE];
		strncpy(nombre_directorio_padre, path, indice_barra);
		nombre_directorio_padre[indice_barra] = '\0';
		int index_dir_padre = buscar_inodo(nombre_directorio_padre);
		inodos[index_dir_padre].fecha_de_modificacion = time(NULL);
	}
}