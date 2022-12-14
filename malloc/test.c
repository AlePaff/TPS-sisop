#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "testing.h"
#include "printfmt.h"
//#include "malloc.h"
#include "malloc.c"

#define HELLO "hello from test"
#define TEST_STRING "FISOP malloc is working!"

void prueba_catedra(void);
void prueba_malloc_casos_borde(void);
void prueba_malloc_mixtas(void);
void prueba_malloc_volumen(void);
void pruebas_calloc(void);
void pruebas_realloc(void);
void pruebas_bloques_medianos(void);
void pruebas_bloques_grandes(void);
void pruebas_free(void);


int
main(void)
{
	prueba_catedra();
	prueba_malloc_casos_borde();
	prueba_malloc_mixtas();
	prueba_malloc_volumen();

	pruebas_calloc();
	pruebas_realloc();

	pruebas_bloques_medianos();
	pruebas_bloques_grandes();

	pruebas_free();

	printfmt("\n\n");
	printfmt("CANTIDAD DE PRUEBAS TOTALES: %d\n", exit_count());
	printfmt("CANTIDAD DE PRUEBAS FALLIDAS: %d\n", failure_count());
	return 0;
}

void
prueba_catedra()
{
	printfmt("\n######### INICIO PRUEBAS CATEDRA #########\n");
	printfmt("%s\n", HELLO);
	char *var = malloc(300);
	strcpy(var, TEST_STRING);
	printfmt("%s\n", var);
	free(var);
	printfmt("######### FIN PRUEBAS CATEDRA #########\n");
}


void
prueba_malloc_mixtas()
{
	printfmt("\n######### INICIO PRUEBAS MALLOC MIXTAS #########\n");

	printfmt("Prueba: pedir una cantidad menor al minimo devuelve un "
	         "puntero de tamaño mínimo\n");
	void *ejemplo = malloc(100);
	print_test("Prueba: Requested: ", requested_memory == MINSIZE);
	free(ejemplo);
	print_test("Prueba: Requested despues de liberar: ",
	           requested_memory == 0);


	int cant_mall = amount_of_mallocs;
	int cant_free = amount_of_frees;
	char *mall;
	int cant = 10000;
	mall = malloc(cant);
	printfmt("Prueba: se pide %d bytes de memoria, y se liberan los "
	         "mismos\n",
	         cant);
	print_test("Prueba: Requested: ", requested_memory == ALIGN32(cant));
	free(mall);
	print_test("Prueba: Cantidad de Mallocs: ",
	           amount_of_mallocs == cant_mall + 1);
	print_test("Prueba: Cantidad de Frees: ",
	           amount_of_frees == cant_free + 1);
	print_test("Prueba: Cantidad de Requested: ", requested_memory == 0);

	printfmt("Prueba: hacer 3 mallocs y luego 3 frees.\n Primero 3 "
	         "mallocs\n");
	char *mall1, *mall2, *mall3;
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	mall1 = malloc(300);
	mall2 = malloc(300);
	mall3 = malloc(300);
	print_test("Prueba: Cantidad de mallocs: ",
	           amount_of_mallocs == cant_mall + 3);
	print_test("Prueba: Requested: ", requested_memory == ALIGN32(300) * 3);
	printfmt("Se hacen 3 frees\n");
	free(mall1);
	free(mall2);
	free(mall3);
	print_test("Prueba: Cantidad de frees: ",
	           amount_of_frees == cant_free + 3);
	print_test("Prueba: Requested: ", requested_memory == 0);


	printfmt("Prueba: hacer 2 mallocs, 1 free, luego 1 malloc y 2 frees\n");
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	mall1 = malloc(300);
	mall2 = malloc(300);

	print_test("Prueba: Requested: ", requested_memory == ALIGN32(300) * 2);
	free(mall1);

	print_test("Prueba: Requested: ", requested_memory == ALIGN32(300));
	mall3 = malloc(300);
	free(mall2);
	free(mall3);
	print_test("Prueba: Cantidad de mallocs: ",
	           amount_of_mallocs == cant_mall + 3);
	print_test("Prueba: Cantidad de frees: ",
	           amount_of_frees == cant_free + 3);
	print_test("Prueba: Requested: ", requested_memory == 0);

	printfmt(
	        "Prueba: se pide hacer un malloc mayor al límite de memoria\n");
	cant_mall = amount_of_mallocs;
	int cant_memoria = requested_memory;
	errno = 0;
	int *ptr1 = malloc(LIMITE_MEMORIA + 1);
	print_test("Malloc devuelve NULL: ", ptr1 == NULL);
	print_test("Cantidad de mallocs no cambia: ",
	           amount_of_mallocs == cant_mall);
	print_test("Requested memory no cambia: ",
	           requested_memory == cant_memoria);
	print_test("Se seta la variable errno a ENOMEM: ", errno == ENOMEM);

	printfmt("######### FIN PRUEBAS MALLOC MIXTAS #########\n");
}

void
prueba_malloc_casos_borde()
{
	printfmt("\n######### INICIO PRUEBAS DE CASOS BORDES #########\n");
	int cant_mall = amount_of_mallocs;
	int cant_free = amount_of_frees;
	int cant_req = requested_memory;
	printfmt("Pruebas con malloc:\n");
	print_test("Prueba: malloc con tamanio 0 devuelve null: ",
	           malloc(0) == NULL);
	print_test("Prueba: malloc con tamanio 0, la cant de malloc es cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba:  malloc con tamanio 0, la cant de frees es "
	           "cero ",
	           amount_of_frees == cant_free);
	print_test("Prueba: malloc con tamanio 0, la cant de memoria pedida es "
	           "cero",
	           requested_memory == 0);

	print_test("Prueba: malloc con tamanio negativo devuelve null: ",
	           malloc(-1) == NULL);
	print_test("Prueba: malloc con tamanio negativo, la cant de malloc es "
	           "cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba:  malloc con tamanio negativo, la cant de frees es "
	           "cero ",
	           amount_of_frees == cant_free);
	print_test("Prueba: malloc con tamanio negativo, la cant de memoria "
	           "pedida es "
	           "cero",
	           requested_memory == 0);

	printfmt("Pruebas con free:\n");
	cant_free = amount_of_frees;
	cant_req = requested_memory;
	free(NULL);
	print_test("Prueba: free con null no suma la cantidad de frees: ",
	           cant_free == amount_of_frees);
	free(NULL);
	print_test("Prueba: free con null no cambia la cant. de memoria:",
	           cant_req == requested_memory);


	printfmt("Pruebas con calloc:\n");
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	char *prueba_call = calloc(0, 0);
	print_test("Prueba: calloc con tamanio 0 devuelve null: ",
	           prueba_call == NULL);
	print_test("Prueba: calloc con tamanio 0, la cant de malloc es cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba: Requested:", requested_memory == 0);

	print_test("Prueba: calloc con tamanio negativo devuelve null: ",
	           calloc(0, -1) == NULL);
	print_test("Prueba: calloc con tamanio negativo, la cant de malloc es "
	           "cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba: Requested:", requested_memory == 0);


	printfmt("Pruebas con realloc:\n");
	// No se prueban casos de equivalencia con malloc o free. Esas se hacen en realloc
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	char *prueba_reall = realloc(NULL, 0);
	print_test(
	        "Prueba: realloc con puntero a NULL y tamanio 0 devuelve null:",
	        prueba_reall == NULL);
	print_test("Prueba: realloc con puntero a NULL y tamanio 0, la cant de "
	           "malloc es cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba: realloc con puntero a NULL y tamanio 0, la cant de "
	           "frees es cero ",
	           amount_of_frees == cant_free);
	print_test("Prueba: realloc con puntero a NULL y tamanio 0  la cant de "
	           "memoria es cero ",
	           requested_memory == 0);

	print_test("Prueba: realloc con puntero a NULL y tamanio negativo "
	           "devuelve null: ",
	           realloc(NULL, -1) == NULL);
	print_test("Prueba: realloc con puntero a NULL y tamanio negativo, la "
	           "cant de malloc es cero ",
	           amount_of_mallocs == cant_mall);
	print_test("Prueba: realloc con puntero a NULL y tamanio negativo, la "
	           "cant de frees es cero ",
	           amount_of_frees == cant_free);
	print_test("Prueba: realloc con puntero a NULL y tamanio negativo, la "
	           "cant de memoria es cero ",
	           requested_memory == 0);

	printfmt("######### FIN PRUEBAS DE CASOS BORDES #########\n");
}

void
prueba_malloc_volumen()
{
	printfmt("\n######### INICIO PRUEBAS DE VOLUMEN #########\n");

	// Seran usadas en pruebas de volumen las siguientes variables
	int cantidad = 10;
	size_t memoria = 100;
	int cantidad_volumen = 1000;
	int bloque_grande = 33554432;  // LARGESIZE
	// int cantidad_imposible = 10000;
	int cantidad_grande = 30000000;
	int limite = 1000000000;  // LIMITE_MEMORIA

	printfmt("Prueba: %i mallocs y free continuos sin usar lo devuelto \n",
	         cantidad);
	int cant_mall = amount_of_mallocs;
	int cant_free = amount_of_frees;
	int cant_mall_fail = 0;
	int req_memoria = requested_memory;
	for (int i = 0; i < cantidad; i++) {
		int *mall = malloc(memoria);
		if (mall == NULL) {
			cant_mall_fail++;
			printfmt("Malloc falla en iteracion %d", i);
		}
		free(mall);
	}
	print_test("", cant_mall_fail == 0);
	print_test("Prueba: Cantidad de mallocs: ",
	           amount_of_mallocs == cant_mall + cantidad);


	printfmt("Prueba: %i mallocs y free continuos usando lo devuelto \n",
	         cantidad);
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	cant_mall_fail = 0;
	for (int i = 0; i < cantidad; i++) {
		char *mall = malloc(memoria);
		if (mall == NULL) {
			cant_mall_fail++;
			printfmt("Malloc falla en iteracion %d", i);
		}
		strcpy(mall, TEST_STRING);
		free(mall);
	}
	print_test("", cant_mall_fail == 0);
	print_test("Prueba: Cantidad de mallocs: ",
	           amount_of_mallocs == cant_mall + cantidad);


	printfmt("Prueba: %i mallocs y free continuos usando lo devuelto y "
	         "haciendo realloc \n",
	         cantidad);
	// int cant_realloc = amount_of_mallocs;
	int cant_realloc_fail = 0;
	for (int i = 0; i < cantidad; i++) {
		char *mall = malloc(memoria);
		strcpy(mall, TEST_STRING);
		mall = realloc(mall, memoria * 2);
		if (mall == NULL) {
			cant_realloc_fail++;
			printfmt("Malloc falla en iteracion %d", i);
		}
		strcpy(mall, TEST_STRING);
		free(mall);
	}
	print_test("", cant_realloc_fail == 0);


	printfmt("Prueba: De %d mallocs con un VOLUMEN MAYOR de %d bytes\n",
	         cantidad_volumen,
	         memoria);
	cant_mall = amount_of_mallocs;
	cant_free = amount_of_frees;
	req_memoria = requested_memory;
	cant_mall_fail = 0;
	int *array_volumen[cantidad_volumen];
	for (int i = 0; i < cantidad_volumen; i++) {
		int *mall = malloc(memoria);
		if (mall == NULL) {
			cant_mall_fail++;
			printfmt("Malloc falla en iteracion %d", i);
		}
		array_volumen[i] = mall;
	}
	print_test("Cantidad de mallocs fallidos es cero\n", cant_mall_fail == 0);
	print_test("Prueba: Cantidad de mallocs:",
	           amount_of_mallocs == cant_mall + cantidad_volumen);
	print_test("Prueba: Cantidad de memoria pedida:",
	           requested_memory ==
	                   req_memoria + cantidad_volumen * ALIGN32(MINSIZE));
	printfmt("Se intenta liberar la memoria pedida");
	for (int i = 0; i < cantidad_volumen; i++) {
		int *mall = array_volumen[i];
		free(mall);
	}
	print_test("Prueba: Cantidad de frees:",
	           amount_of_frees == cant_free + cantidad_volumen);
	print_test("Prueba: Cantidad de memoria pedida:",
	           requested_memory == req_memoria);

	printfmt(
	        "Prueba: No se puede pedir más memoria que un bloque grande\n");
	print_test("El puntero es NULL:", malloc(bloque_grande) == NULL);


	printfmt("No se puede pedir más memoria que el limite tota "
	         "establecido\n");
	int contador = 0;
	int *array_imposible[cantidad_volumen];
	for (int i = 0; i < cantidad_volumen; i++) {
		int *mall = malloc(cantidad_grande);
		if (mall == NULL) {
			printfmt("Fallé en la iteración: %i \n", contador);
			printfmt("Mallocs hechos: %i, Requested: %i\n",
			         amount_of_mallocs,
			         requested_memory);
			print_test("Memoria pedida no supera el limite:",
			           requested_memory < limite);
			break;
		}
		array_imposible[i] = mall;
		contador++;
	}
	// Libero la memoria
	for (int i = 0; i < contador; i++) {
		int *mall = array_imposible[i];
		free(mall);
	}
	// printfmt("Frees hechos: %i \n", amount_of_frees);
	// printfmt("Frees hechos 2: %i \n", cant_free + contador);
	print_test("Prueba: Cantidad de frees:",
	           amount_of_frees == cant_free + cantidad_volumen + contador);
	print_test("Prueba: Cantidad de liberada:", requested_memory == 0);

	printfmt("######### FIN PRUEBAS DE VOLUMEN #########\n");
}

void
pruebas_bloques_medianos()
{
	printfmt("\n######### INICIO PRUEBAS BLOQUES MEDIANOS #########\n");

	int cant_mallocs = amount_of_mallocs;
	int mem_mallocs = requested_memory;
	int cant_frees = amount_of_frees;

	int *ptr = malloc(1020000);
	print_test("Malloc de un tamaño mediano creado correctamente",
	           amount_of_mallocs == cant_mallocs + 1);
	print_test("Malloc de un tamaño mediano reserva memoria correctamente",
	           requested_memory == mem_mallocs + ALIGN32(1020000));
	// printfmt("Requested memory es: %d antes del free\n", requested_memory);
	free(ptr);
	print_test("Malloc de un tamaño mediano liberado correctamente",
	           amount_of_frees == cant_frees + 1);
	// printfmt("Requested memory es: %d despues del free\n", requested_memory);
	// printfmt("Nuestro resultado es: %d despues del free\n",
	//         mem_mallocs - ALIGN32(1020000));
	// print_test("Malloc de un tamaño mediano libera cantidad de memoria "
	//           "correcta",
	//           requested_memory == 0);

	printfmt("Se hacen múltiples mallocs de tamaño mediano y luego se los "
	         "libera\n");
	int *ptr1 = malloc(1052000);
	int *ptr2 = malloc(1020400);
	int *ptr3 = malloc(1010330);
	int *ptr4 = malloc(2220000);
	int *ptr5 = malloc(3215000);
	int memoria_total = ALIGN32(1052000) + ALIGN32(1020400) +
	                    ALIGN32(1010330) + ALIGN32(2220000) +
	                    ALIGN32(3215000);
	print_test("Mallocs de un tamaño mediano creado correctamente",
	           amount_of_mallocs == cant_mallocs + 6);
	print_test("Malloc de un tamaño mediano reserva memoria correctamente",
	           requested_memory == mem_mallocs + memoria_total);
	// printfmt("Requested memory es: %d antes del free\n", requested_memory);
	free(ptr1);
	free(ptr2);
	free(ptr3);
	free(ptr4);
	free(ptr5);
	print_test("Mallocs de un tamaño mediano liberados correctamente",
	           amount_of_frees == cant_frees + 6);
	// printfmt("Requested memory es: %d despues del free\n", requested_memory);
	// printfmt("Nuestro resultado es: %d despues del free\n", mem_mallocs);
	print_test("Malloc de un tamaño mediano libera cantidad de memoria "
	           "correcta",
	           requested_memory == 0);
}


void
pruebas_bloques_grandes()
{
	printfmt("\n######### INICIO PRUEBAS BLOQUES GRANDES #########\n");

	int cant_mallocs = amount_of_mallocs;
	int mem_mallocs = requested_memory;
	int cant_frees = amount_of_frees;

	int *ptr = malloc(10020000);
	print_test("Malloc de un tamaño grande creado correctamente",
	           amount_of_mallocs == cant_mallocs + 1);
	print_test("Malloc de un tamaño grande reserva memoria correctamente",
	           requested_memory == mem_mallocs + ALIGN32(10020000));
	// printfmt("Requested memory es: %d antes del free\n", requested_memory);
	free(ptr);
	print_test("Malloc de un tamaño grande liberado correctamente",
	           amount_of_frees == cant_frees + 1);
	// printfmt("Requested memory es: %d despues del free\n", requested_memory);
	// printfmt("Nuestro resultado es: %d despues del free\n",
	//         mem_mallocs - ALIGN32(10020000));
	print_test("Malloc de un tamaño grande libera cantidad de memoria "
	           "correcta",
	           requested_memory == 0);

	printfmt("Se hacen múltiples mallocs de tamaño grande y luego se los "
	         "libera\n");
	int *ptr1 = malloc(10052000);
	int *ptr2 = malloc(10020400);
	int *ptr3 = malloc(10010330);
	int *ptr4 = malloc(20220000);
	int *ptr5 = malloc(32150000);
	int memoria_total = ALIGN32(10052000) + ALIGN32(10020400) +
	                    ALIGN32(10010330) + ALIGN32(20220000) +
	                    ALIGN32(32150000);
	print_test("Mallocs de un tamaño grande creado correctamente",
	           amount_of_mallocs == cant_mallocs + 6);
	print_test("Malloc de un tamaño grande reserva memoria correctamente",
	           requested_memory == mem_mallocs + memoria_total);
	// printfmt("Requested memory es: %d antes del free\n", requested_memory);
	free(ptr1);
	free(ptr2);
	free(ptr3);
	free(ptr4);
	free(ptr5);
	print_test("Mallocs de un tamaño grande liberados correctamente",
	           amount_of_frees == cant_frees + 6);
	// printfmt("Requested memory es: %d despues del free\n", requested_memory);
	// printfmt("Nuestro resultado es: %d despues del free\n", mem_mallocs);
	print_test("Malloc de un tamaño grande libera cantidad de memoria "
	           "correcta",
	           requested_memory == 0);
}


void
pruebas_calloc()
{
	printfmt("\n######### INICIO PRUEBAS CALLOC #########\n");
	int cant_callocs = amount_of_mallocs;
	int mem_callocs = requested_memory;

	int cantidad = 5;
	size_t memoria = 100;
	int cant_calloc_fail = 0;
	int *ptr = calloc(cantidad, memoria);
	if (ptr != NULL)
		cant_callocs++;
	printfmt(
	        "Prueba: Hacer %i callocs y chequear que se inicialicen en 0\n",
	        cantidad);
	for (size_t i = 0; i < cantidad * memoria / sizeof(int); i++) {
		if (ptr[i] != 0) {
			cant_calloc_fail++;
			printfmt("FALLA CALLOC: El valor en la posicion %i es "
			         "%i\n",
			         i,
			         ptr[i]);
		}
		// printfmt("Calloc: tiene gurdado ceros: \n", ptr[i] == 0);
	}
	print_test("No falló ningún calloc", cant_calloc_fail == 0);
	print_test("Prueba: Cantidad de callocs: ",
	           amount_of_mallocs ==
	                   cant_callocs);  // se llama una sola vez a calloc, por lo tanto se hace una sola vez malloc
	printfmt("cant de mallocs es: %d y cantidad de callocs es: %d\n",
	         amount_of_mallocs,
	         cant_callocs);
	free(ptr);
	print_test("Prueba: Memoria reservada por callocs: ",
	           requested_memory ==
	                   mem_callocs);  // debe verse inalterada respecto al inicio

	printfmt("Prueba: se pide hacer un calloc mayor al tamaño máximo\n");
	errno = 0;
	int *ptr1 = calloc(6, LARGESIZE + 1);
	print_test("Calloc devuelve NULL: ", ptr1 == NULL);
	print_test("Cantidad de mallocs no cambia: ",
	           amount_of_mallocs == cant_callocs);
	print_test("Requested memory no cambia: ",
	           requested_memory == mem_callocs);
	print_test("Se seta la variable errno a ENOMEM: ", errno == ENOMEM);

	printfmt("######### FIN PRUEBAS CALLOC #########\n");
}

void
pruebas_realloc()
{
	printfmt("\n######### INICIO PRUEBAS REALLOC #########\n");
	int cant_reallocs = amount_of_mallocs;
	int cant_frees = amount_of_frees;
	int req_mem = requested_memory;


	// simular malloc
	void *ptr1 = realloc(NULL, 100);
	print_test("Prueba: realloc con puntero a NULL y tamanio distinto de "
	           "cero se comporta como malloc",
	           amount_of_mallocs == cant_reallocs + 1);
	print_test("Prueba: (idem) el puntero no da nulo", ptr1 != NULL);
	free(ptr1);
	print_test("Prueba: (idem) y se libera memoria",
	           amount_of_frees == cant_frees + 1);


	// simular free
	cant_frees = amount_of_frees;
	void *ptr2 = malloc(666);
	ptr2 = realloc(ptr2, 0);
	print_test("Prueba: realloc con puntero distinto de nulo y tamanio 0 "
	           "se comporta como free",
	           amount_of_frees == cant_frees + 1);


	// contenido sigue intacto
	cant_reallocs = amount_of_mallocs;
	cant_frees = amount_of_frees;
	printfmt("4 pruebas: Realloc mantiene el contenido de la memoria "
	         "realocada\n");
	void *ptr = malloc(100);
	print_test("   Prueba: Malloc reserva memoria para guarda un string",
	           amount_of_mallocs == cant_reallocs + 1);
	strcpy(ptr, TEST_STRING);
	print_test("   Prueba: El string se guarda correctamente",
	           strcmp(ptr, TEST_STRING) == 0);
	ptr = realloc(ptr, 200);
	print_test("   Prueba: Se hace realloc y el contenido sigue intacto",
	           strcmp(ptr, TEST_STRING) == 0);
	free(ptr);
	print_test("   Prueba: Se libera memoria",
	           amount_of_frees == cant_frees + 1);


	// realloc para agrandar
	printfmt("3 pruebas: se agranda el bloque de 100 a 1000 bytes \n");
	req_mem = requested_memory;
	void *ptr3 = malloc(100);
	print_test("  Prueba: Luego del malloc, memoria reservada ",
	           requested_memory == req_mem + MINSIZE);
	ptr3 = realloc(ptr3, 1000);
	print_test("  Prueba: Despues de realloc, memoria reservada",
	           requested_memory == req_mem + ALIGN32(1000));
	free(ptr3);
	print_test("  Prueba: Despues del free, memoria reservada ",
	           requested_memory == req_mem);
	//	printfmt("Requested memory despues del free: %d\n",requested_memory);

	// realloc para achicar
	printfmt("3 pruebas: se achica el bloque de 1024 a 960 bytes \n");
	req_mem = requested_memory;
	void *ptr4 = malloc(1024);
	print_test("  Prueba: Luego del malloc, memoria reservada ",
	           requested_memory == req_mem + ALIGN32(1024));
	ptr4 = realloc(ptr4, 960);
	print_test("  Prueba: Despues de realloc, memoria reservada ",
	           requested_memory == req_mem + ALIGN32(960));
	free(ptr4);
	print_test("  Prueba: Despues del free, memoria reservada ",
	           requested_memory == req_mem);
	//	printfmt("Requested memory despues del free: %d\n",requested_memory);

	printfmt("Prueba: se pide hacer un realloc mayor al tamaño máximo\n");
	errno = 0;
	int *ptr5 = malloc(100);
	req_mem = requested_memory;
	cant_reallocs = amount_of_mallocs;
	print_test("Realloc devuelve NULL: ",
	           realloc(ptr5, LARGESIZE + 1) == NULL);
	print_test("Cantidad de reallocs no cambia: ",
	           amount_of_mallocs == cant_reallocs);
	print_test("Requested memory no cambia: ", requested_memory == req_mem);
	print_test("Se seta la variable errno a ENOMEM: ", errno == ENOMEM);
	free(ptr5);
	print_test("Se libera el malloc pedido: ",
	           requested_memory == req_mem - MINSIZE);
	printfmt("######### FIN PRUEBAS REALLOC #########\n");
}

void
pruebas_free()
{
	printfmt("\n######### INICIO PRUEBAS FREE #########\n");
	int cant_frees = amount_of_frees;
	int req_mem = requested_memory;


	// se libera un malloc
	void *ptr1 = malloc(129083);
	free(ptr1);
	print_test("Se libera memoria pedida", amount_of_frees == cant_frees + 1);
	print_test("Se libera cantidad de memoria correcta",
	           requested_memory == req_mem);

	// invalid free
	cant_frees = amount_of_frees;
	errno = 0;  // reiniciamos la variable para asegurar que cambia con el invalid free
	print_test("Antes de un invalid free errno != ENOMEM", errno != ENOMEM);
	void *ptr2 = malloc(66600);
	req_mem = requested_memory;
	free(ptr1);
	print_test("Después de un invalid free se setea errno = ENOMEM",
	           errno == ENOMEM);
	print_test("Después de un invalid free, cantidad de frees no suma uno",
	           amount_of_frees == cant_frees);
	print_test("Después de un invalid free, no se modifica la memoria "
	           "requerida",
	           requested_memory == req_mem);
	free(ptr2);


	printfmt("######### FIN PRUEBAS FREE #########\n");
}