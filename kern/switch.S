/*
 * Your code here for context_switch
 *
 * It should restores the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 * 
 * La firma de la funcion si fuese escrita en C seria:
 * void context_switch(struct Trapframe *tf);
 *
 * Para tener una referencia de que esta haciendo context_switch
 * ver el diagrama https://i.imgur.com/4Y0prK3.png
 */

.globl context_switch;
context_switch:
	add	$4,%esp			/* acomoda el stack pointer... */
	mov	(%esp),%esp		/* ... para que el contenido al puntero tf quede en el stack */
	popal				/* restaura los registros de proposito general */
	pop %es
	pop	%ds	
	add	$8,%esp			/* acomoda el stack para que apunte al valor de eip */
	iret
	/* iret hace pop de eip, cs, eflags y esp (con esp continua la ejecucion, o sea vuelve al stack) */
	
spin:
	jmp spin

