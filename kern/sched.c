#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

int ciclos = 0;

// para las estadisticas
int cantidad_reinicios = 0;
int cantidad_llamadas_sched = 0;
int ejecuciones_totales = 0;
int cantidad_ciclos_totales = 0;

void imprimir_estadisticas();
void sched_halt(void);
void ejecucion_auxiliar_prioridad(int inicio, int fin, int prioridad);
void ejecucion_auxiliar(int inicio, int fin);

// resetea el contador de ciclos y pone a todos los procesos en la prioridad 0
void
reset()
{
	ciclos = 0;
	for (int i = 0; i < NENV; i++) {
		envs[i].prioridad = 0;
	}
	cantidad_reinicios++;
}


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	cantidad_llamadas_sched++;
	struct Env *idle;  // idle is the last environment that was running

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	// Without scheduler, keep runing the last environment while it exists

	int pos_env_actual = 0;  // posicion actual en el arreglo de envs
	idle = curenv;

#ifdef ROUND_ROBIN

	if (idle != NULL) {  // aca empiezo en una pos q no es la incial
		pos_env_actual =
		        ENVX(idle->env_id) + 1;  // entonces quiero la sgte
	}

	// busco el siguiente runnable
	ejecucion_auxiliar(pos_env_actual, NENV);
	ejecucion_auxiliar(0, pos_env_actual);

	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	if (idle && idle->env_status == ENV_RUNNING) {
		env_run(idle);
	}
#endif

#ifdef SCHED_PRIOR
	int prioridad = 0;

	if (ciclos == RESET) {  // si ya se hicieron RESET ciclos resetea
		reset();
	}

	// mientras no esté en la prioridad máxima, busco el siguiente runnable
	while (prioridad <= PRIORMIN) {
		// empieza la búsqueda por la pos actual
		ejecucion_auxiliar_prioridad(pos_env_actual, NENV, prioridad);

		// continúa la búsqueda por la pos inicial
		ejecucion_auxiliar_prioridad(0, pos_env_actual, prioridad);

		prioridad++;  // pasa a la siguiente prioridad
	}
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	if (idle && idle->env_status == ENV_RUNNING) {
		env_run(idle);
	}


#endif
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		imprimir_estadisticas();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here
	//	imprimir_estadisticas();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

// busca el siguiente runnable con prioridad menor o igual a la que se le pasa por parametro y lo ejecuta
void
ejecucion_auxiliar_prioridad(int inicio, int fin, int prioridad)
{
	int pos_env_actual = inicio;
	for (pos_env_actual; pos_env_actual < fin; pos_env_actual++) {
		if (envs[pos_env_actual].env_status == ENV_RUNNABLE &&
		    envs[pos_env_actual].prioridad <= prioridad) {
			ciclos++;
			cantidad_ciclos_totales++;  //[estadisticas]
			ejecuciones_totales++;      //[estadisticas]
			envs[pos_env_actual].prioridades_stats[prioridad]++;  //[estadisticas]
			envs[pos_env_actual]
			        .prioridad++;  // se disminuye la prioridad del proceso en cada ejecución
			env_run(&envs[pos_env_actual]);
		}
	}
}

// se encarga de buscar el siguiente entorno runnable y ejecutarlo
void
ejecucion_auxiliar(int inicio, int fin)
{
	int pos_env_actual = inicio;
	for (pos_env_actual; pos_env_actual < fin; pos_env_actual++) {
		if (envs[pos_env_actual].env_status == ENV_RUNNABLE) {
			ejecuciones_totales++;  //[estadisticas]
			env_run(&envs[pos_env_actual]);
		}
	}
}


void
imprimir_estadisticas()
{
	cprintf("==== Estadisticas de scheduling: ====\n");

	cprintf(" (*) Numero de ejecuciones de cada proceso:\n");
	int ejec_totales = 0;
	int historial_procesos[NENV];
	for (int j = 0; j < NENV; j++) {  // cant. de procesos
		if (envs[j].env_runs == 0)
			continue;
		cprintf("	EnvID %d: %d veces\n",
		        envs[j].env_id,
		        envs[j].env_runs);
		historial_procesos[j] = envs[j].env_id;
		for (int i = 0; i < PRIORMIN;
		     i++) {  // cant. de prioridades para cada proceso
			if (envs[j].prioridades_stats[i] == 0)
				continue;  // si nunca estuvo en la prioridad i, no imprimo nada
			cprintf("	La cant. de veces que %d tuvo "
			        "prioridad %d fueron: %d \n",
			        envs[j].env_id,
			        i,
			        envs[j].prioridades_stats[i]);
		}
		ejec_totales = ejec_totales + envs[j].env_runs;
	}
	cprintf(" (*) Ejecuciones totales de todos los procesos: %d\n",
	        ejec_totales);
	cprintf(" (*) Nro. de llamadas al scheduler: %d\n",
	        cantidad_llamadas_sched);
	cprintf(" (*) Historial de procesos ejecutados (PID): ");
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs == 0)
			continue;
		cprintf("%d   ", historial_procesos[i]);
	}
	cprintf("\n");

#ifdef SCHED_PRIOR
	cprintf(" (*) Veces que se reseteo el contador de ciclos: %d\n",
	        cantidad_reinicios);
	cprintf(" (*) Ciclos totales: %d\n", cantidad_ciclos_totales);
	int prioridad0 = 0;
	int prioridadMIN = 0;
	// int prioridadPromedio = 0;
	for (int ii = 0; ii < NENV; ii++) {
		if (envs[ii].env_runs == 0)
			continue;
		prioridad0 = prioridad0 +
		             envs[ii].prioridades_stats[0];  // cada proceso en la prioridad 0
		prioridadMIN =
		        prioridadMIN + envs[ii].prioridades_stats[PRIORMIN - 1];  // cada proceso en la prioridad MIN		
	}
	cprintf(" (*) Cuantas veces en total se estuvo en prioridad más alta "
	        "(cero): %d veces\n",
	        prioridad0);
	cprintf(" (*) Cuantas veces en total se estuvo en prioridad más baja "
	        "(PRIORMIN): %d veces\n",
	        prioridadMIN);
#endif
}