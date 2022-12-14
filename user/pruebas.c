// hello, world
#include <inc/lib.h>

int pruebas_exitosas = 0;
int pruebas_fallidas = 0;
int pruebas_totales = 0;

char* chequeo_pruebas(int booleano){		//int para no tener que usar stdbool.h
	pruebas_totales++;
	if (booleano){
		pruebas_exitosas++;
		return "OK";
	}
	else{
		pruebas_fallidas++;
		return "FAIL";
	}
}

void
umain(int argc, char **argv)
{
	

	cprintf("********* INICIO PRUEBAS SCHEDULER CON PRIORIDADES *********\n");

	//igual?? menor??
	// cprintf("PRUEBA (*): Luego de una llamada a fork el proceso hijo debe tener prioridad menor a la del padre\n");
	int prior_padre = 0;
	int prior_hijo = 0;
	// cprintf("Soy el proceso padre %d y mi prioridad es: %d \n", thisenv->env_id, thisenv->prioridad);
	prior_padre = thisenv->prioridad;	
	int id;
	if ((id = fork()) < 0)
		panic("fork: %e", id);
	if (id == 0){
		prior_hijo = thisenv->prioridad;
		cprintf("Soy el proceso hijo %d, mi padre es %d y mi prioridad es: %d \n",thisenv->env_id,thisenv->env_id,thisenv->prioridad);
		return ;		//el hijo termina
	}
	//espera a que termine el hijo
	for (int i = 0; i < 1000000; i++);
	cprintf("Prioridad hijo es: %d, prioridad padre es: %d\n",prior_hijo,prior_padre);
	cprintf("	La prioridad del proceso hijo es menor que la del padre %s\n", chequeo_pruebas(prior_hijo > prior_padre));

	// cprintf("PRUEBA (*): %s\n", chequeo_pruebas());




	cprintf("\n\n********* Pruebas totales: %d, exitosas: %d, fallidas: %d ********* \n", pruebas_totales, pruebas_exitosas, pruebas_fallidas);
}
