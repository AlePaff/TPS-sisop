# Lab: shell

### Búsqueda en $PATH
**Responder: ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?**

Un [[wrapper|https://techterms.com/definition/wrapper]] es una función que *envuelve* a otra función, en el sentido de que encapsula y oculta la complejidad subyacente, en este caso la familia de funciones `exec*(3)` envuelve a la system call `execve(2)` [[se puede diferenciar por los caracteres extra|https://stackoverflow.com/a/47609180/3325706]].
Por lo tanto la diferencia entre ambas es que la syscall `execve(2)` es la que se encarga de ejecutar un programa, mientras que la familia de funciones `exec*(3)` son las que se encargan de llamar a la syscall `execve(2)` y pasarle los parámetros necesarios para que se ejecute el programa.


**Responder: ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?**
Si, puede fallar, por ejemplo si el programa que se quiere ejecutar no existe, o si no se tienen los permisos necesarios para ejecutarlo. La familia de funciones `exec*(3)` devuelven -1 en caso de error, y la variable `errno` se encarga de indicar el error que ocurrió. La shell en ese caso imprime el error correspondiente y vuelve a mostrar el prompt. Devuelve el mismo resultado que devuelve `execve(2)` en caso de error. Citando del manual de `exec*(3)`:
>All of these functions may fail and set errno for any of the errors specified for `execve(2)`.

---

### Comandos built-in
**Pregunta: ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false).**
Asumo que como "true" y "false" se refiere a que la shell no tiene que llamar a un programa externo para ejecutarlos, sino que los ejecuta internamente. 
`cd` se necesitaría que sea un built-in, ya que en el fork (que en nuestra shell se hace en `run_cmd()` en `runcmd.c`) el proceso hijo cambiaría el directorio pero no el proceso padre (quien mantiene una referencia) entonces no tendría el directorio actualizado. 
`pwd` en cambio no es necesario que sea un built-in, ya que no modifica el estado del proceso padre, simplemente imprime el directorio actual. Aún asi se mantiene como built-in, ya que es una operación muy común y por eficiencia no tiene sentido llamar a un programa externo para ejecutarla (en nuestra shell sería usando `execvp()` en `exec.c` el programa externo)
 

---

### Variables de entorno adicionales
**Pregunta: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?**
*Contexto: se refiere a la llamada al fork que en nuestra shell está en `run_cmd()` en `runcmd.c`*
Si se hace antes de la llamada al fork éstas van a estar viviendo en la shell y no en la ejecución del programa, que es justo donde se necesitan, ya que son temporales. Por lo tanto se hace después del fork para que estén viviendo en el proceso hijo, y no en el padre.

**Pregunta: En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un array y se lo coloca en el tercer argumento de una de las funciones de exec(3).**
**¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.**
**Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.**

Algunos ejemplos de estas funciones son `execle`, `execvpe`, etc.
```c                 
int execle(const char *path, const char *arg, ...          
                /*, (char *) NULL, char * const envp[] */);         

int execvpe(const char *file, char *const argv[],          
                char *const envp[]);  
...                     
```
El comportamiento no es el mismo, en el <u>primer caso</u> el proceso que se ejecuta con `exec` tiene acceso a las variables de entorno que tiene la shell (más las que se le pasan por parámetro), mientras que en el <u>segundo caso</u> el proceso que se ejecuta con `exec` tiene acceso solo a las variables de entorno que se le pasan por parámetro y no a las que tiene la shell
Para que el comportamiento sea el mismo una posible implementación es pasarle a `execve` TODAS las variables de entorno (el array de variables de entorno que tiene la shell y las que se le pasan por parámetro) y por supuesto que el proceso tenga acceso a todas las variables de entorno.

--- 

### Procesos en segundo plano
**Responder: Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano**

Cuando se ejecuta un comando en segundo plano, se le agrega un `&` al final del comando. El mecanismo que se utilizó en ésta implementación de la shell es esperar a que el proceso hijo termine pero sin bloquear a la shell. Para esto se utilizó la función `waitpid(2)` con la flag `WNOHANG`. 

---

### Flujo estándar
**Responder: Investigar el significado de 2>&1, explicar cómo funciona su forma general y mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo invertiendo el orden de las redirecciones. ¿Cambió algo?**

Combina salida estandar (1 es stdout) y errores (2 es stderr) en un solo fd (file descriptor), en este caso 1, que es stdout. 
Se puede analizar por partes:
 * `2>` redirecciona stderr a un archivo (sin especificar)
 * `&1` redirecciona stderr a stdout.
Y porque no usar simplemente `2>1`? Porque `2>1` redirecciona stderr a un archivo llamado 1, y no a stdout^[https://stackoverflow.com/questions/818255/what-does-21-mean].

En el ejemplo de `cat out.txt`

```bash
$ ls -C /home /noexiste >out.txt 2>&1

$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
ubuntu  vagrant
```
Se lista el contenido de `/home` y `/noexiste`, y se redirecciona la salida a `out.txt`. Como `/noexiste` no existe, se imprime el error en stderr, y como se redireccionó stderr a stdout, queda todo en `out.txt`
Si invierto el orden de las redirecciones de la siguiente manera:

```bash
$ ls -C /home /noexiste 2>&1 >out.txt
ls: cannot access '/noexiste': No such file or directory

$ cat out.txt
/home:
ubuntu  vagrant
```
Debido a `2>&1` se va a imprimir el error por la salida estandar (la terminal) y esto ocurre antes de que se redireccione la salida estandar a `out.txt`, entonces en `out.txt` queda solo la salida de ls y no el error.

---

### Tuberías simples (pipes)
**Responder: Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con la implementación del este lab.**

El exit code va a ser del proceso que se ejecuta al final del pipe, se puede ver el exit code del ultimo proceso con `echo $?`. Por ejemplo:
```bash
$ ls | sleep 1 | cosa_que_no_existe
cosa_que_no_existe: command not found
$ echo $?
127
```
Ahora si se cambia el orden y se pone primero el comando que no existe, y por ultimo algun otro con salida exitosa, el exit code va a ser 0.
```bash
$ cosa_que_no_existe | echo hola
hola
cosa_que_no_existe: command not found
$ echo $?
0
```
Para nuestra shell como no soporta ésta característica el exit code siempre va a ser 0 (exitoso), puesto que en `exec.c` al final de el case **PIPE** éste termina siempre con `_exit(0)`, no guarda el status code de los procesos que se ejecutan en el pipe. Como se puede ver
```bash
$ ls | sleep 1 | cosa_que_no_existe
error en execvp: : No such file or directory
$ echo $?
0
```
Y luego
```bash
$ cosa_que_no_existe | echo hola
error en execvp: : No such file or directory
hola
$ echo $?
0
```





### Pseudo-variables
**Pregunta: Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).**

* `$0` devuelve el nombre del script o comando que se está ejecutando.
``` bash
$ echo $0
-bash
```

* `$?` devuelve el código de salida (exit code) del último comando ejecutado
``` bash
$ ls /noexiste
$ echo $?
2
$ echo $?
0
```

* `$!` devuelve el PID del último proceso ejecutado en background (en segundo plano)
``` bash
$ sleep 10 &
$ ps
  PID TTY          TIME CMD                                                
 5392 pts/0    00:00:00 bash                                               
 5531 pts/0    00:00:00 sleep                                              
 5532 pts/0    00:00:00 ps                                                 
$ echo $!
5531
```

* `$$` devuelve el PID del proceso actual (en el ejemplo es el PID de bash)
``` bash
$ echo $$
5392
$ ps
  PID TTY          TIME CMD                                                
 5392 pts/0    00:00:00 bash                                             
 5532 pts/0    00:00:00 ps 
```

* `$_` devuelve el último argumento de la última línea de comando ejecutada
``` bash
$ echo hola que tal
$ echo $_
tal
```
Otras variables mágicas en este [[link|https://tldp.org/LDP/abs/html/refcards.html]]

