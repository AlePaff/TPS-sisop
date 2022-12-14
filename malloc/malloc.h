#ifndef _MALLOC_H_
#define _MALLOC_H_


// Formula para alinear a 4 y 8 bytes (se usan los shifts (>> y <<)) que son mas rapidos para dividir/multiplicar)
#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define ALIGN8(s) (((((s) -1) >> 3) << 3) + 8)
#define ALIGN32(s) (((((s) -1) >> 5) << 5) + 32)
#define REGION2PTR(r) ((r) + 1)

// Se resta 1 para que apunte al inicio de la region (resta el header)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)

#define MINSIZE 256
#define SMALLSIZE 16384
#define MEDIUMSIZE 1048576
#define LARGESIZE 33554432
#define LIMITE_MEMORIA 1000000000  // 1GB segun google, podría ser más


struct bloque {
	// bool free; //el bloque tiene espacio libre
	// size_t free_space; //cantidad de espacio libre
	size_t size;                    // tamaño del bloque
	struct region *region_inicial;  // puntero a la 1er region del bloque
	struct bloque *next;            // puntero al siguiente bloque
	void *ptr;  // puntero a la memoria obtenida con mmap
};

struct region {
	bool free;
	size_t size;
	struct region *next;  // puntero a la siguiente region (siguiente struct)
	void *ptr;            // puntero a una region del bloque
};

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void print_statistics(void);


struct region *inicializar_region(struct region *nueva_region,
                                  struct bloque *nuevo_bloque);
struct bloque *inicializar_bloque(struct bloque *nuevo_bloque, size_t size_bloque);
struct region *find_free(struct region *actual, size_t size_region);
static struct region *find_free_region(size_t size_region);
bool free_region(struct bloque *lista, bool terminado, void *ptr);
bool bloque_vacio();
void eliminar_bloque();
struct region *coalescing(struct bloque *lista, struct region *region);
void free_bloque(struct bloque *lista_actual);



#endif // _MALLOC_H_
