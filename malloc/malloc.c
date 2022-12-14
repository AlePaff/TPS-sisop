// #define _DEFAULT_SOURCE		//ya lo tiene stdio.h

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>  //para mmap
#include <string.h>
#include <errno.h>

#include "malloc.h"
#include "printfmt.h"
#include "testing.h"

// para los tests
int amount_of_mallocs = 0;
int amount_of_frees = 0;
int requested_memory = 0;

int HEADERSIZE = sizeof(bool) + sizeof(size_t) + sizeof(struct region *);

bool memoria_inicializada = false;
struct bloque *lista_bloque_peque = NULL;
struct bloque *lista_bloque_mediana = NULL;
struct bloque *lista_bloque_grande = NULL;


void
print_statistics(void)
{
	printfmt("###ESTADISTICAS###\n");
	printfmt("mallocs:   %d\n", amount_of_mallocs);
	printfmt("frees:     %d\n", amount_of_frees);
	printfmt("requested: %d\n", requested_memory);
}


// crea una nueva region que ocupa todo el bloque
struct region *
inicializar_region(struct region *nueva_region, struct bloque *nuevo_bloque)
{
	nueva_region = (struct region *) nuevo_bloque->region_inicial;
	nueva_region->free = true;
	nueva_region->size =
	        nuevo_bloque->size - sizeof(*nuevo_bloque) -
	        sizeof(*(nuevo_bloque->region_inicial));  // resto los 2 headers
	nueva_region->next = NULL;
	nueva_region->ptr = nuevo_bloque->ptr;
	nuevo_bloque->region_inicial = nueva_region;
	return nueva_region;
}

// inicializa un bloque con su tamaño pasado por parametro
// devuelve el mismo bloque (principalmente para controlar si mmap falla)
struct bloque *
inicializar_bloque(struct bloque *nuevo_bloque, size_t size_bloque)
{
	void *memoria =
	        mmap(NULL,  // el kernel elige la direccion
	             size_bloque,
	             PROT_READ | PROT_WRITE,
	             MAP_PRIVATE | MAP_ANONYMOUS,
	             0,  // Tambien, -1 para asegurar que sea portable. Ver mmap(2)
	             0);
	if (memoria == MAP_FAILED)
		return NULL;
	nuevo_bloque = (struct bloque *)
	        memoria;  // para darle algun valor inicial, no importa cual
	nuevo_bloque->ptr = (void *) memoria;
	nuevo_bloque->region_inicial =
	        memoria + sizeof(*nuevo_bloque);  // sumo el header del bloque
	nuevo_bloque->next = NULL;
	nuevo_bloque->size = size_bloque;

	switch (size_bloque) {
	case (SMALLSIZE):
		if (!lista_bloque_peque)
			lista_bloque_peque = nuevo_bloque;
		else {
			nuevo_bloque->next = lista_bloque_peque;
			lista_bloque_peque = nuevo_bloque;
		}
		break;
	case (MEDIUMSIZE):
		if (!lista_bloque_mediana)
			lista_bloque_mediana = nuevo_bloque;
		else {
			nuevo_bloque->next = lista_bloque_mediana;
			lista_bloque_mediana = nuevo_bloque;
		}
		break;

	case (LARGESIZE):
		if (!lista_bloque_grande)
			lista_bloque_grande = nuevo_bloque;
		else {
			nuevo_bloque->next = lista_bloque_grande;
			lista_bloque_grande = nuevo_bloque;
		}
		break;
	}
	return nuevo_bloque;
}


// finds the next free region
// that holds the requested size
struct region *
find_free(struct region *actual, size_t size_region)
{
	struct region *new;

	do {
		if (actual->free == true && actual->size >= size_region) {
			if (actual->size >=
			    size_region + sizeof(*actual) + MINSIZE) {
				// se fija si puede hacer el splitting

				new = actual + ((sizeof(*new) + size_region) /
				                sizeof(*actual));
				new->free = true;
				new->size = actual->size -
				            (size_region + sizeof(*actual));
				new->ptr = actual->ptr;
				new->next = actual->next;
				actual->free = false;
				actual->size = size_region;
				actual->next = new;
			} else
				actual->free =
				        false;  // si no puede hacer el splitting, lo ocupa todo
			return actual;
		} else
			actual = actual->next;  // si no hay un bloque libre, sigue buscando
	} while (!actual);
	return NULL;
}


// finds the next free region with FF or BF
// that holds the requested size, using find_free()
static struct region *
find_free_region(size_t size_region)
{
	struct region *actual = NULL;
	struct bloque *bloques_actuales;
	int tam;
	if (size_region < MINSIZE)
		size_region = MINSIZE;

	if (size_region < LARGESIZE - sizeof(*actual) - sizeof(*bloques_actuales))
		tam = 3;
	if (size_region < MEDIUMSIZE - sizeof(*actual) - sizeof(*bloques_actuales))
		tam = 2;
	if (size_region < SMALLSIZE - sizeof(*actual) - sizeof(*bloques_actuales))
		tam = 1;

#ifdef FIRST_FIT
	bool terminado = false;
	while (!terminado) {
		switch (tam) {
		case (1):
			bloques_actuales = lista_bloque_peque;
			break;
		case (2):
			bloques_actuales = lista_bloque_mediana;
			break;
		case (3):
			bloques_actuales = lista_bloque_grande;
			break;
		default:  // es mayor que el maximo tamaño
			return NULL;
		}
		while (!terminado && bloques_actuales) {
			actual = bloques_actuales->region_inicial;
			actual = find_free(actual, size_region);
			if (actual)
				terminado = true;
			else
				bloques_actuales = bloques_actuales->next;
		}
		tam++;
	}
	return actual;
#endif

#ifdef BEST_FIT
	struct region *best = NULL;
	while (tam < 4) {
		switch (tam) {
		case (1):
			bloques_actuales = lista_bloque_peque;
			break;
		case (2):
			bloques_actuales = lista_bloque_mediana;
			break;
		case (3):
			bloques_actuales = lista_bloque_grande;
			break;
		default:  // es mayor que el maximo tamaño
			return NULL;
		}
		while (bloques_actuales) {
			actual = bloques_actuales->region_inicial;
			do {
				if (actual->free == true &&
				    actual->size > size_region) {
					if (!best)
						best = actual;
					else if (actual->size < best->size)
						best = actual;
				}
				if (actual->next)
					actual = actual->next;  // si no hay un bloque libre, sigue buscando
			} while (!actual);
			bloques_actuales = bloques_actuales->next;
		}
		tam++;
	}
	if (best)
		best = find_free(best, size_region);
	return best;
#endif
	printfmt("Advertencia: No se está usando BF ni FF. Compilar usando "
	         "alguna\n");
	return actual;
}

void *
malloc(size_t size)
{
	if ((int) size <= 0) {
		return NULL;
	}

	if (requested_memory + size > LIMITE_MEMORIA) {
		errno = ENOMEM;
		return NULL;  // Nota: Realmente debería lanzar una excepción
	}

	size = ALIGN32(size);  // alineado al tamanio del header
	if (size < MINSIZE)
		size = MINSIZE;

	// inicializadas a NULL para evitar warnings del compilador
	struct region *region_actual = NULL;     // region a devolver
	struct bloque *bloques_actuales = NULL;  // lista de bloques a buscar
	struct bloque *nuevo_bloque = NULL;
	struct region *nueva_region = NULL;
	int tam;

	// si el tamaño es mayor que el maximo, no se puede alocar
	if (size > LARGESIZE - sizeof(*region_actual) - sizeof(*bloques_actuales)) {
		errno = ENOMEM;
		return NULL;
	}

	// primero se busca si existe un bloque donde se pueda guardar
	region_actual = find_free_region(size);

	// si no se encontro un lugar para alocar la memoria entonces se agranda el heap
	if (!region_actual) {
		if (size <
		    SMALLSIZE - sizeof(*nueva_region) - sizeof(*nuevo_bloque))
			tam = SMALLSIZE;
		else if ((size > SMALLSIZE - sizeof(*nueva_region) -
		                         sizeof(*nuevo_bloque)) &&
		         (size < MEDIUMSIZE - sizeof(*nueva_region) -
		                         sizeof(*nuevo_bloque)))
			tam = MEDIUMSIZE;
		else
			tam = LARGESIZE;

		nuevo_bloque = inicializar_bloque(nuevo_bloque, tam);
		bloques_actuales = nuevo_bloque;

		region_actual = inicializar_region(nueva_region, nuevo_bloque);
		region_actual = find_free(region_actual, size);
	}
	amount_of_mallocs++;
	requested_memory += size;

	return REGION2PTR(region_actual);
}


void
free(void *ptr)
{
	// caso 0: si ptr es null no se hace nada
	if (ptr == NULL)
		return;

	// se define una variable de control que va a ayudar a determinar cuando se hace, si se hace, un free
	bool terminado = false;

	// estructuras que vamos a usar para iterar
	struct bloque *lista_actual;  // apunta a la lista sobre la que trabajamos
	struct region *region_actual;  // apunta a la region sobre la que trabajamos

	// se lee la metadatada asociada al puntero. Si se paso un puntero no allocado llevara a un segfault
	region_actual = PTR2REGION(ptr);

	// primero se debe buscar en que bloque de que lista esta el puntero
	terminado = free_region(lista_bloque_peque, terminado, ptr);
	lista_actual = lista_bloque_peque;
	if (!terminado) {
		terminado = free_region(lista_bloque_mediana, terminado, ptr);
		lista_actual = lista_bloque_mediana;
	}
	if (!terminado) {
		terminado = free_region(lista_bloque_grande, terminado, ptr);
		lista_actual = lista_bloque_grande;
	}
	if (!terminado) {  // no encontró nada
		errno = ENOMEM;
		return;
	}

	amount_of_frees++;
	requested_memory =
	        requested_memory -
	        (region_actual->size);  // se actualiza la memoria pedida (tests)

	// ahora hay que ver si hay regiones libres para mergear en el bloque donde se hizo free
	region_actual = coalescing((struct bloque *) (region_actual->ptr),
	                           region_actual);

	// Ya se termino de hacer coalescing dentro del bloque,

	free_bloque(lista_actual);  // se fija si el bloque entero esta para liberar
	return;
}

// se encarga de buscar el puntero a liberar en las regiones de la lista de
// bloques pasada por parametro devuelve un booleano si encontró o no al puntero
bool
free_region(struct bloque *lista, bool terminado, void *ptr)
{
	struct bloque *lista_actual = lista;  // buscamos en la lista
	struct region *region_actual;
	while (!terminado &&
	       lista_actual) {  // mientras no hayamos terminado y exista un bloque actual)
		region_actual =
		        lista_actual->region_inicial;  // primera region del primer bloque
		while (!terminado && region_actual) {
			if (REGION2PTR(region_actual) ==
			    ptr) {  // si el puntero de la region actual
				    // mas el tamaño de struct de metadata
				    // es igual al puntero que se le paso a free, lo encontramos
				region_actual->free =
				        true;  // se marca la region como libre
				terminado = true;  // terminamos
			} else {
				region_actual =
				        region_actual->next;  // la region actual pasa a ser la siguiente a si misma
			}
		}
		if (!terminado)
			lista_actual =
			        lista_actual->next;  // si esta aca significa
			                             // que no se encontro
			                             // o el bloque actual
			                             // se quedo sin regiones a verificar y se pasa al siguiente
	}  // si se salio del while significa que o se encontro o no estaba en la lista pequeña
	return terminado;
}

// se encarga de mergear las regiones libres que se encuentren contiguas
struct region *
coalescing(struct bloque *lista, struct region *region_actual)
{
	region_actual = lista->region_inicial;  // volvemos al comienzo del bloque
	while (region_actual->next) {  // mientras haya un siguiente, si no hay terminamos
		if (region_actual->free) {  // si la region actual esta libre
			if (region_actual->next->free) {  // si la siguiente esta libre
				region_actual->size = region_actual->size +
				                      region_actual->next->size +
				                      sizeof(*region_actual);  // se le suma al tamaño actual el tamaño de la proxima region y el tamaño del header
				region_actual->next = region_actual->next->next;  // la region actual pasa a apuntar a la region que apuntaba la region mergeada
			}
		} else
			region_actual = region_actual->next;
	}
	return region_actual;
}

// se fija si el bloque está vacio y de ser así lo libera
void
free_bloque(struct bloque *lista_actual)
{
	if (lista_actual->region_inicial->free &&
	    (!lista_actual->region_inicial
	              ->next)) {  // significa que el bloque esta completamente liberado
		void *lista_a_liberar =
		        lista_actual;  // reservamos el puntero al bloque a liberar
		switch (lista_actual->size) {  // para determinar sobre cual lista se va a trabajar
		case (SMALLSIZE):
			lista_actual = lista_bloque_peque;
			if (lista_actual ==
			    lista_a_liberar) {  // es el primero de la lista
				lista_bloque_peque =
				        lista_actual->next;  // la lista apunta al segundo
			} else {
				while (lista_actual->next != lista_a_liberar)  // mientras el bloque a liberar no sea el siguiente de la lista
					lista_actual = lista_actual->next;
				lista_actual->next = lista_actual->next->next;  // el que estaba antes del bloque
				                                                // a eliminar apunta al siguiente del bloque a eliminar
			}
			// se libera la memoria
			break;
		case (MEDIUMSIZE):
			lista_actual = lista_bloque_mediana;
			if (lista_actual ==
			    lista_a_liberar) {  // es el primero de la lista
				lista_bloque_mediana = lista_actual->next;  // la lista apunta al segundo
			} else {
				while (lista_actual->next != lista_a_liberar)  // mientras el bloque a liberar no sea el siguiente de la lista
					lista_actual = lista_actual->next;
				lista_actual->next = lista_actual->next->next;  // el que estaba antes del bloque
				                                                // a eliminar apunta al siguiente del bloque a eliminar
			}
			break;
		case (LARGESIZE):
			lista_actual = lista_bloque_grande;
			if (lista_actual ==
			    lista_a_liberar) {  // es el primero de la lista
				lista_bloque_grande =
				        lista_actual->next;  // la lista apunta al segundo
			} else {
				while (lista_actual->next != lista_a_liberar)  // mientras el bloque a liberar no sea el siguiente de la lista
					lista_actual = lista_actual->next;
				lista_actual->next = lista_actual->next->next;  // el que estaba antes del bloque
				                                                // a eliminar apunta al siguiente del bloque a eliminar
			}
			break;
		}
		munmap(lista_a_liberar, lista_actual->size);
	}
}

void *
calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
		return NULL;
	if (nmemb * size > LARGESIZE) {
		errno = ENOMEM;
		return NULL;
	}
	void *ptr = malloc(nmemb * size);
	if (ptr != NULL)
		memset(ptr, 0, nmemb * size);  // setea los bytes en 0
	return ptr;
}

void *
realloc(void *ptr, size_t size)
{
	struct region *region_actual = PTR2REGION(ptr);
	struct region *nueva;

	// Si ptr es igual a NULL, el comportamiento es igual a malloc(size)
	if (ptr == NULL) {
		ptr = malloc(size);
		return ptr;
	}

	// Si size es igual a cero (y ptr no es NULL) debería ser equivalente a
	// free(ptr) nuestra implementación soporta hacer free(NULL)
	if (size == 0) {
		free(ptr);
		return NULL;  // Es para quitar un warning pero podría ser conceptualmente erroneo
	}
	size = ALIGN32(size);
	if (size < MINSIZE)
		size = MINSIZE;

	if (size > LARGESIZE) {
		errno = ENOMEM;
		return NULL;
	}
	// si se paso un puntero erroneo salta segfault
	if (size < region_actual->size) {  // ACHICAR REGION
		if (region_actual->next &&
		    region_actual->next->free) {  // ACHICAR REGION Y NEXT LIBRE
			requested_memory =
			        requested_memory - (region_actual->size - size);
			nueva = region_actual->next -
			        (region_actual->size - size) /
			                sizeof(*region_actual);
			nueva->free = region_actual->next->free;
			nueva->next = region_actual->next->next;
			nueva->ptr = region_actual->next->ptr;
			nueva->size = region_actual->next->size;
			region_actual->size = size;
			region_actual->next = nueva;
		} else {  // ACHICAR REGION Y NEXT NO ESTA LIBRE
			if (region_actual->size - size >=
			    MINSIZE + sizeof(*region_actual)) {  //
				requested_memory = requested_memory -
				                   (region_actual->size - size);
				nueva = region_actual +
				        (sizeof(*region_actual) + size) /
				                sizeof(*region_actual);
				nueva->free = true;
				nueva->size = region_actual->size - size -
				              sizeof(*nueva);
				nueva->next = region_actual->next;
				nueva->ptr = nueva;
				region_actual->size = size;
				region_actual->next = nueva;
			}
		}
	} else {  // AGRANDAR REGION
		if (region_actual->next && region_actual->next->free &&
		    (region_actual->next->size + region_actual->size >=
		     size + sizeof(*region_actual))) {  // EL SIGUIENTE ESTÁ LIBRE
			if (region_actual->next->size >
			    size + MINSIZE) {  // SOBRA ESPACIO PARA EL SPLIT
				requested_memory = requested_memory +
				                   (size - region_actual->size);
				nueva = region_actual->next +
				        size / sizeof(*region_actual);
				nueva->ptr = nueva;
				nueva->next = region_actual->next->next;
				nueva->size = region_actual->next->size - size;
				nueva->free = true;
				region_actual->size = size;
				region_actual->next = nueva;
			} else {  // NO SOBRA PARA UN SPLIT
				requested_memory = requested_memory +
				                   region_actual->next->size +
				                   sizeof(*region_actual);
				region_actual->size = region_actual->size +
				                      region_actual->next->size +
				                      sizeof(*region_actual);
				region_actual->next = region_actual->next->next;
			}
		} else {  // EL SIGUIENTE NO ESTÁ LIBRE
			nueva = malloc(size);
			ptr = memcpy(REGION2PTR(nueva),
			             REGION2PTR(region_actual),
			             region_actual->size);
			free(REGION2PTR(region_actual));
		}
	}
	return ptr;
}
