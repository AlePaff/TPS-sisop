# Lab fork

El objetivo de este lab es familiarizarse con las llamadas al sistema `fork(2)` (que crea una copia del proceso actual) y `pipe(2)` (que proporciona un mecanismo de comunicación unidireccional entre dos procesos).

A efectos de lo explicado en la [página de entregas](../entregas.md), el esqueleto para este lab se encuentra en el repositorio [`https://github.com/fisop/labs`][repolabs]{:.alert-link}, rama **_fork_** (la cual no necesita ninguna integración previa).
{:.alert .alert-primary}

**IMPORTANTE**: leer el archivo `README.md` que se encuentra en la raíz del proyecto. Contiene información sobre cómo realizar la compilación de los archivos, y cómo ejecutar el formateo de código.
{:.alert .alert-warning}

**REQUERIDO**: para las entregas es condición **necesaria** que el _check_ del **formato** de código esté en verde a la hora de realizar el PR (_pull request_). Para ello, se puede ejecutar `make format` localmente, comitear y subir esos cambios.
{:.alert .alert-danger}

[repolabs]: https://github.com/fisop/labs


## Índice
{:.no_toc}

* TOC
{:toc .sidetoc}


## Bibliografía sobre syscalls
{: #biblio}

Tanto en el caso de syscalls del sistema operativo, como funciones de la biblioteca estándar de C, se puede consultar su documentación mediante el comando `man`. Esto es particularmente recomendable en el caso de _syscalls_ como `stat(2)`, que son complejas y tienen muchos flags: `man 2 stat`. En las páginas de manual también se indican los _headers_ (a.k.a `#include ...`) necesarios para cada syscall. (Las páginas de manual también se pueden consultar online en sitios web como [dashsash.io] y [man7.org].)

[dashsash.io]: https://dashdash.io
[man7.org]: http://man7.org/linux/man-pages

En general, una buena referencia sobre sistemas POSIX es **[KERR]**. En particular, para este lab son relevantes:

  - Tareas **_pingpong_**, **_primes_**: §6.1, §6.2, §24.2, §44.2
  - Tarea **_find_**: §2.5, §5.4, §18.8, §18.11
  - Tarea **_xargs_**: §6.6, §24.1, §26.1, §27.1, §27.2

Opcionalmente se puede leer el capítulo 3 a modo de introducción.


## Tarea: pingpong
{: #pingpong}

Se pide escribir un programa en C que use `fork(2)` y `pipe(2)` para enviar y recibir (ping-pong) un determinado valor entero, entre dos procesos.  El valor se debe crear con `random(3)` **una vez ambos procesos existan**.

El programa debe imprimir por pantalla la secuencia de eventos de ambos procesos, en el formato **exacto** que se especifica a continuación:

```bash
$ ./pingpong
Hola, soy PID <x>:
  - primer pipe me devuelve: [3, 4]
  - segundo pipe me devuelve: [6, 7]

Donde fork me devuelve <y>:
  - getpid me devuelve: <?>
  - getppid me devuelve: <?>
  - random me devuelve: <v>
  - envío valor <v> a través de fd=?

Donde fork me devuelve 0:
  - getpid me devuelve: <?>
  - getppid me devuelve: <?>
  - recibo valor <v> vía fd=?
  - reenvío valor en fd=? y termino

Hola, de nuevo PID <x>:
  - recibí valor <v> vía fd=?
```

Ayuda:

  - Nótese que como las tuberías _—pipes—_ son unidireccionales, se necesitarán
    dos para poder transmitir el valor en una dirección y en otra.

  - Para obtener números aleatorios que varíen en cada ejecución del programa,
    se debe inicializar el _PRNG_ (generador de números pseudo-aleatorios)
    mediante la función `srandom(3)` (típicamente con el valor de `time(2)`).

  - Si `fork(2)` fallase, simplemente se imprime un mensaje por salida de error
    estándar (_stderr_), y el programa termina.

  - Tener en cuenta el tipo de los valores de retorno para cada una de las _syscalls_/funciones de _libc_ a utilizar (por ejemplo: `random(3)`)

Llamadas al sistema: `fork(2)`, `pipe(2)`.


## Tarea: primes
{: #primes}

La [criba de Eratóstenes][wpsieve-es] ([sieve of Eratosthenes][wpsieve-en] en inglés) es un algoritmo milenario para calcular todos los primos menores a un determinado número natural, _n_.

Si bien la visualización del algoritmo suele hacerse “tachando” en una grilla, el concepto de criba, o _sieve_ (literalmente: cedazo, tamiz, colador) debe hacernos pensar más en un filtro. En particular, puede pensarse en _n_ filtros apilados, donde el primero filtra los enteros múltiplos de 2, el segundo los múltiplos de 3, el tercero los múltiplos de 5, y así sucesivamente.

Si modelásemos cada filtro como un proceso, y la transición de un filtro al siguiente mediante tuberías _(pipes)_, se puede implementar el algoritmo con el siguiente pseudo-código (ver [fuente original][coxcsp], y en particular la imagen que allí se muestra):

```
p := <leer valor de pipe izquierdo>

imprimir p // asumiendo que es primo

mientras <pipe izquierdo no cerrado>:
    n = <leer siguiente valor de pipe izquierdo>
    si n % p != 0:
        escribir <n> en el pipe derecho
```

(El único proceso que es distinto, es el primero, que tiene que simplemente generar la secuencia de números naturales de 2 a _n_. No tiene lado izquierdo.)

La interfaz que se pide es:

```bash
$ ./primes <n>
```

donde _n_ será un número natural mayor o igual a 2. El código debe crear una estructura de procesos similar a la mostrada en la imagen, de tal manera que:

  - El primer proceso cree un proceso derecho, con el que se comunica mediante
    un _pipe_.

  - Ese primer proceso, escribe en el _pipe_ la secuencia de números de 2 a
    _n_, para a continuación cerrar el _pipe_ y esperar la finalización del
    proceso derecho.

  - Todos los procesos sucesivos aplican el pseudo-código mostrado
    anteriormente, con la salvedad de que son responsables de crear a su
    “hermano” derecho, y la tubería (_pipe_) que los comunica.

  - Se debería poder ejecutar correctamente el programa con un _N_ mayor o igual a 10000.

Ejemplo de uso:

```
$ ./primes 35
primo 2
primo 3
primo 5
primo 7
primo 11
primo 13
primo 17
primo 19
primo 23
primo 29
primo 31
```

Ayuda:

  - Conceptualmente esta tarea es la más difícil de las cuatro del lab, y no
    es prerrequisito para poder realizar las dos siguientes.

Llamadas al sistema: `fork(2)`, `pipe(2)`.

[coxcsp]: https://swtch.com/~rsc/thread/
[wpsieve-es]: https://es.wikipedia.org/wiki/Criba_de_Eratóstenes
[wpsieve-en]: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes


## Tarea: find
{: #find}

Se pide escribir una versión muy simplificada de la utilidad `find(1)`. Esta herramienta, tal y como se la encuentra en sistemas GNU/Linux, acepta una miríada de opciones (ver su [página de manual][find(1)], o un [resumen gráfico][evansfind]). No obstante, en este lab se implementará sólo una de ellas.

La sinopsis de nuestra implementación será:

```bash
$ ./find [-i] <cadena>
```

Invocado como `./find xyz`, el programa buscará y mostrará por pantalla todos los archivos del directorio actual (y subdirectorios) cuyo nombre contenga (o sea igual a) `xyz`. Si se invoca como `./find -i xyz`, se realizará la misma búsqueda, pero sin distinguir entre mayúsculas y minúsculas.

Por ejemplo, si en el directorio actual se tiene:

```
.
├── Makefile
├── find.c
├── xargs.c
├── antiguo
│   ├── find.c
│   ├── xargs.c
│   ├── pingpong.c
│   ├── basurarghh
│   │   ├── find0.c
│   │   ├── find1.c
│   │   ├── pongg.c
│   │   └── findddddddd.c
│   ├── planes.txt
│   └── pingpong2.c
├── antinoo.jpg
└── GNUmakefile
```

el resultado de las distintas invocaciones debe ser como sigue (**no importa el orden** en que se impriman los archivos de un mismo directorio):

```bash
$ ./find akefile
Makefile
GNUmakefile

$ ./find Makefile
Makefile

$ ./find -i Makefile
Makefile
GNUmakefile

$ ./find arg
xargs.c
antiguo/xargs.c
antiguo/basurarghh

$ ./find pong
antiguo/pingpong.c
antiguo/basurarghh/pongg.c
antiguo/pingpong2.c

$ ./find an
antiguo
antiguo/planes.txt
antinoo.jpg

$ ./find d.c
find.c
antiguo/find.c
antiguo/basurarghh/findddddddd.c
```

Ayuda:

  - Usar recursividad para descender a los distintos directorios.

  - Nunca descender los directorios especiales `.` y `..` (ambos son un “alias”;
    el primero al directorio actual, el segundo
    a su directorio inmediatamente superior).

  - No es necesario preocuparse por ciclos en enlaces simbólicos.

  - En el resultado de `readdir(3)`, asumir que el campo `d_type`
    siempre está presente, y es válido.

  - La implementación _case-sensitive_ vs. _case-insensitive_ (opción `-i`) se
    puede resolver limpiamente usando un puntero a función como abstracción.
    (Ver [`strstr(3)`][strstr(3)].)

Requisitos:

  - Llamar a la función `opendir(3)` **una sola vez**, al principio del programa
    (con argumento `"."`; no es necesario conseguir el _nombre_ del directorio
    actual, si tenemos su alias).

  - Para abrir sub-directorios, usar exclusivamente la función `openat(2)`
    (con el flag `O_DIRECTORY` como precaución). De esta manera, no es
    necesario realizar concatenación de cadenas para abrir subdirectorios.

      - Sí será necesario, no obstante, concatenar cadenas para mostrar por
        pantalla los resultados. No es necesario usar memoria dinámica; es suficiente un
        único buffer estático de longitud `PATH_MAX`.

      - Funciones que resultarán útiles como complemento a `openat()`:
        `dirfd(3)`, `fdopendir(3)`.

Llamadas al sistema: `openat(2)`, `readdir(3)`.

[find(1)]: https://dashdash.io/1/find
[strstr(3)]: https://dashdash.io/3/strstr
[evansfind]: https://twitter.com/b0rk/status/993862211964735488/photo/1


## Tarea: xargs
{: #xargs}

En su versión más sencilla, la utilidad `xargs(1)` permite:

  - Tomar un único argumento (`argv[1]`, un sólo comando).
  - Leer nombres de archivos de la entrada estándar (_stdin_), de a uno por línea.
  - Para cada nombre de archivo leído, invocar al comando especificado con el
    nombre de archivo como argumento.

Por ejemplo, si se invoca:

```bash
$ echo /home | xargs ls
```

Esto sería equivalente a realizar `ls /home`. Pero si se invoca:

```bash
$ printf "/home\n/var\n" | xargs ls
```

Esto sería equivalente a ejecutar `ls /home; ls/var`.

Variantes (soportadas por la utilidad):

  - Aceptar más de un argumento. Por ejemplo:

    ```bash
    $ printf "/home\n/var\n" | xargs ls -l
    ```

    En este caso, el comando ejecutado sobre el _input_ es `ls -l`.

  - Aceptar nombres de archivos separados por espacio, por ejemplo:

    ```bash
    $ echo /home /var | xargs ls
    ```

    (Esto impediría, no obstante, que se puedan pasar a _xargs_ nombres de
    archivos con espacios en sus nombres, como ser: `/home/user/Media Files`)

  - Aceptar el “empaquetado” de varios nombres de archivos en una sola invocación
    del comando. Por ejemplo, en el segundo ejemplo de arriba, que se ejecute
    `ls /home /var` (una sola vez) en lugar de `ls /home; ls /var` (dos
    veces).

Para ampliar y conocer el comportamiento de una implementación de  _xargs_ moderna, y por ejemplo las opciones `-r`, `-0`, `-n`, `-I` y `-P`, consultar [la página de manual][xargs(1)].

Se pide implementar la siguiente versión modificada de _xargs_.

Requisitos:

  - Leer los nombres de archivo **línea a línea**, **nunca** separados **por espacios** (se recomienda usar la
    función [`getline(3)`][getline(3)]). También, es necesario eliminar el caracter `'\n'` para obtener el nombre del
    archivo.

  - El “empaquetado” vendrá definido por un valor entero positivo disponible en
    la macro `NARGS` (la cual tiene efecto durante el proceso de compilación.
    Para más información se puede consultar la siguiente [documentación](https://dashdash.io/1/gcc#-D_1)).
    El programa debe funcionar de manera tal que siempre se
    pasen `NARGS` argumentos al comando ejecutado (excepto en su última
    ejecución, que pueden ser menos). **Si no se encuentra definida `NARGS`,
    se debe definir a 4.** 
    Se puede hacer algo como:

    ```c
    #ifndef NARGS
    #define NARGS 4
    #endif
    ```

  - Se debe esperar **siempre** a que termine la ejecución del comando actual.

    - **Mejora o funcionalidad opcional:** si el primer argumento a _xargs_ es
      `-P`, emplear hasta 4 ejecuciones del comando en paralelo (pero **nunca**
      más de 4 a la vez).

Llamadas al sistema: `fork(2)`, `execvp(3)`, `wait(2)`.

[xargs(1)]: https://dashdash.io/1/xargs
[getline(3)]: https://dashdash.io/3/getline

## Desafíos
{: #Desafíos}

Las tareas listadas aquí no son obligatorias, pero suman para el régimen de [final alternativo](../regimen.md#final).
{:.alert .alert-warning}

### Más utilidades Unix
{: #utilidades-unix}

Implementar, a elección, dos o más de los siguientes comandos. Cada comando
tiene una cantidad de puntos basado en la dificultad relativa de su
implementación. Como mínimo para cumplir el desafío se deben alcanzar los 10
puntos.

Las opciones son:

* `ps` (5pts): el comando _process status_ muestra información básica de los procesos
  que están corriendo en el sistema. Se pide **como mínimo** una implementación
que muestre el pid y comando (i.e. argv) de cada proceso (esto es equivalente a
hacer `ps -eo pid,comm`, se recomienda compararlo con tal comando).  Para más
información, leer la [sección `ps0`][ps0], de uno de los labs anteriores. Ayuda:
leer `proc(5)` para información sobre el directorio `/proc`.

* `ls` (2pts):  el comando _list_ permite listar los contenidos de un
  directorio, brindando información extra de cada una de sus entradas.
Se pide implementar una versión simplificada de `ls -al` donde cada entrada del
directorio se imprima en su propia línea, indicando _nombre_, _tipo_ (con la
misma simbología que usa `ls`), _permisos_ (se pueden mostrar numéricamente) y
_usuario_ (aquí nuevamente alcanza con mostrar el uid). Si se trata de un
_enlace simbólico_, se debe mostrar a qué archivo/directorio apunta el mismo.
Ayuda: es similar a `find`, pero deben incorporar `stat(2)` y `readlink(2)`.

* `cp` (3pts): implementar el comando _copy_ de forma eficiente, haciendo uso de
`mmap` tal y como se describe en la [sección `cp1`][cp1] de uno de los labs
anteriores. Solo se pide soportar el caso básico de copiar un archivo regular a
otro (i.e. `cp src dst` donde `src` es un archivo regular y `dst` no existe). Si
el archivo destino existe, debe fallar; y si el archivo fuente no existe
también. Ayuda: `mmap(2)` y `memcpy(3)`. Se recomienda implementar
primero una versión simplificada con `write` y `read`; y luego optimizarlo con
`mmap`.

* `timeout` (5pts): el comando _timeout_ realiza una ejecución de un segundo
proceso, y espera una cantidad de tiempo prefijada (e.g. `timeout duración
comando`. Si se excede ese tiempo y el proceso sigue en ejecución, lo termina
enviándole SIGTERM. Se pide implementar una versión simplificada del comando
`timeout`. La implementación debe usar _señales_. Ayuda: `timer_create(2)`,
`timer_settime(2)`, `kill(2)`.

[ps0]: ../unix/#ps0-
[cp1]: ../unix/#cp1-

### strace(1)
{: #strace}

La utilidad `strace` (de _system-call trace_) permite ejecutar un proceso y a la
vez registrar todas las llamadas al sistema que éste realiza, junto con los
parámetros y valores de retorno. Se trata de un programa complejo si se quiere
seguir procesos que están compuestos por threads o realizan fork. Sin embargo,
en su versión más sencilla (con un único proceso) es bastante fácil de seguir.

Un ejemplo del uso de strace para inspeccionar el proceso `echo` podría ser:

```
$ strace -e trace=read,write,open,execve echo hi
execve("/bin/echo", ["echo", "hi"], 0x7ffcbeba00b8 /* 27 vars */) = 0
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\20\35\2\0\0\0\0\0"...,
832) = 832
read(3, "# Locale name alias data base.\n#"..., 4096) = 2995
read(3, "", 4096)                       = 0
write(1, "hi\n", 3hi
)                     = 3
+++ exited with 0 +++
```

#### Parte 1

Leer la documentación sobre la utilidad `strace`. Eligiendo al menos otros tres
comandos de Unix (que no sean _echo_), mostrar las syscalls que usan,
identificando aquellas que conozcan. Explicar lo que strace imprime por
pantalla, y si aparecen más llamadas de las esperadas. ¿De dónde provienen?.

Correr `strace` sobre `strace`, e inspeccionar qué syscalls utiliza. Prestar
especial atención en `wait(2)` y `ptrace(2)`.

#### Parte 2

Luego de familiarizarse con la herramienta, se pide _programar_ una versión
sencilla de `strace`. Los requisitos mínimos de tal implementación son:
* Debe imprimir _todas_ las llamadas al sistema, junto con su valor de
  retorno. Alcanza con indicar el número de syscall, en x86\_64 pueden usar
[ésta tabla][syscalls-table] para encontrar el _nombre_ de la syscall. No hace falta
imprimir los argumentos de las syscalls.
* Debe soportar la ejecución de un único proceso, del que asumiremos no realiza
  `fork` ni `exec`; no maneja threads ni señales especiales.
* No es necesario que soporte flags adicionales, y el formato de salida es libre
  mientras esté la información esperada.
* Se debe incluir una breve explicación en prosa comentando cada parte del
  código.

[syscalls-table]: https://gitlab.com/strace/strace/-/blob/master/src/linux/x86_64/syscallent.h

Se debe, además de proveer el código funcionando; incluir evidencia del
funcionamiento (e.g. corridas con programas de prueba, comparaciones con
`strace` original, etc).

Ayuda: leer detenidamente `ptrace(2)`, ya que el funcionamiento de `strace` se
basa fuertemente en esa syscall. Las macros descritas en `wait(2)` (como
`WIFSTOPPED`) son de utilidad.

{% include footnotes.html %}
