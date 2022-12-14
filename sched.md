# sched.md

### Para compilar y ejecutar
La compilación se realiza mediante `$ make`. En el directorio `obj/kern` se puede encontrar:

* kernel — el binario ELF con el kernel
* kernel.asm — assembler asociado al binario

Cuando ejecutemos y compilemos se va a crear la carpeta "obj" ahi está el programa compilado de cada proceso. Para ejecutar un proceso de usuario en particular dentro del kernel, se puede usar `make run-<proceso>` o `make run-<proceso>-nox`.
Por ejemplo, `make run-hello-nox` correrá el proceso de usuario `user/hello.c`.

<u>IMPORTANTE</u>: (Ctrl+a)+x para salir!

Aparte de ejecutar algún comando tanto para compilar como para los tests se debe agregar la flag `RR=true` o bien `SP=true` si se quiere usar el scheduler Round Robin o el implementado por nosotros, respectivamente.
Para ejecutar con una determinada cantidad de CPUs ejecutar con la flag `CPUS=n` donde n es la cantidad de CPUs que se quieren usar
Por ejemplo `make run-hello RR=true CPUS=2` ejecutará el proceso de usuario `user/hello.c` con el scheduler Round Robin y con 2 CPUs.

`$ make qemu`: corre JOS con interfaz grafica
`$ make qemu-nox`: corre JOS sin modo (o interfaz) gráfico (redirige salida a la terminal)
`$ make grade`: corre las pruebas automatizadas
`$ make qemu-nox-gdb`: inicializa qemu pero espera una conexión de gdb para debuggear (abrir otra terminal)
`$ make gdb`: inicia una sesión de gdb para debuggear JOS


### Capturas para visualizar el cambio de contexto
En las siguientes capturas se puede ver el comportamiento del stack, registros antes y despues del cambio de contexto y la llamada a iret para volver a user land.

TrapFrame en memoria al comienzo del context switch
![registros al comienzo del context switch](./Screenshots/Trapframe%20en%20el%20stack.png)

Registros antes de llamar a iret
![registros antes de llamar a iret](./Screenshots/registros%20antes%20iret.png)

El stack antes de llamar a iret
![stack antes de llamar a iret](./Screenshots/stack%20antes%20del%20iret.png)

Los registros al comienzo del alltraps
![registros al comienzo del alltraps](./Screenshots/Registros%20al%20comienzo%20del%20alltraps.png)

El stack al final del alltraps viendose como un trapframe
![stack al final del alltraps](./Screenshots/registros%20guardados%20en%20stack%20en%20all%20traps.png)



### Scheduler con prioridades

Máxima prioridad: 0
Mínima prioridad: `PRIORMIN` (en nuestro caso es 10)

Todos los procesos inician con la mayor prioridad (cero). Antes de llamar a `env_run`, su prioridad disminuye en uno. Consideramos que hubiese sido más correcto hacer la disminución de prioridad dentro de `env_run`, pero de esa forma fallaban algunos tests.
Luego de cierta cantidad de llamadas al scheduler (`RESET` = 15), las prioridades de todos los procesos pendientes se resetean, es decir, vuelven a la mayor prioridad. Inicialmente elegimos `RESET` = 20 porque nos pareció razonable, pero a algunos miembros del equipo le fallaba el test `dumb_fork`. Decidimos probar distintos valores para la constante y llegamos a que con 15 o menos, funcionaba bien para todos.
Partimos de la base de Round Robin, vamos recorriendo los procesos de envs, y nos vamos fijando las prioridades. Se ejecutarán aquellos procesos que tengan la prioridad mayor o igual a la prioridad actual. Ejemplo:

Prioridad_actual = 3

Se ejecutarán todos los procesos de envs que estén en estado ready to run y que tengan prioridad <= 3:

PA con prioridad 1: se va a ejecutar
PB con prioridad 3: se va a ejecutar
PC con prioridad 4: no se va a ejecutar
PD con prioridad 0: se va a ejecutar
PE con prioridad 8: no se va a ejecutar

Luego de recorrer el array envs, disminuirá la prioridad actual y repite el proceso con la nueva prioridad, en el ejemplo sería 4. De esta forma, evitaremos que los procesos con mayor prioridad esperen un ciclo para ejecutarse. 

En el caso de hacer un fork, el proceso hijo tendrá menor prioridad que su padre. Esto evita que fork se transforme en una forma de aumentar la prioridad de un proceso.

###Inconvenientes

La prueba de primes falla cuando usamos el scheduler de prioridades:

`qemu-system-i386: terminating on signal 15 from pid 14769 (make)
    MISSING 'CPU .: 1877 .00001120. new env 00001121'`

En algunos casos, la prueba llega a más procesos. 


