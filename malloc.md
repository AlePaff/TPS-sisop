# TP1: Manejo del _heap_

## Índice
{:.no_toc}

* TOC
{:toc .sidetoc}

## Introducción

**AVISO**: antes de comenzar, verificar que se tiene instalado [el software necesario](../kit.md#tools){:.alert-link}.
{:.alert .alert-warning}

En este trabajo se desarrollará una librería de usuario que implementará las funciones [`malloc(3)`](https://dashdash.io/3/malloc), [`calloc(3)`](https://dashdash.io/3/malloc), [`realloc(3)`](https://dashdash.io/3/realloc) y [`free(3)`](https://dashdash.io/3/malloc).
La librería se encargará de solicitar la memoria que requiera, y la administrará de forma transparente para el usuario.

Podrá utilizarse de forma normal en cualquier programa de C, ya sea de forma estática o dinámica (como se verá más adelante).

<div class="alert alert-success" markdown="1">
**Casos reales**

Aunque se podría discutir el propósito de querer definir nuestra propia librería de _manejo de memoria_, resulta que es algo bastante común, conforme las necesidades del contexto así lo ameritan. Por ejemplo, aquí se listan algunas implementaciones:

- [dlmalloc](https://gee.cs.oswego.edu/dl/html/malloc.html) utilizado por _GNU_
- [jemalloc](https://github.com/jemalloc/jemalloc) utilizada por _Firefox_
- [tcmalloc](https://github.com/google/tcmalloc) creado por _Google_
- [mimalloc](https://github.com/microsoft/mimalloc) creado por _Microsoft_
</div>

## Esqueleto y compilación

Se provee un esqueleto mínimo con una implementación funcional utilizando `sbrk(2)` la cual puede ser compilada tanto estática como dinámicamente contra cualquier programa en _C_ que utilice la librería estándar.

Para compilar la librería se puede ejecutar:

```bash
make libmalloc.so
```

El resultado es una _librería_ dinámica `libmalloc.so`. Por su parte, se pueden generar y ejecutar, dos versiones diferentes de un programa de pruebas, de la siguiente forma:

- Estática

```bash
make run-s
```

Esta versión compila el programa `test.c` conjuntamente con los `.o` de la librería `malloc.c`, y luego corre el binario `test-s`.

- Dinámica

```bash
make run-d
```

En esta versión se compila el mismo programa `test.c`, pero esta vez no se incluye la librería. En su lugar, se utiliza la variable de entorno `LD_PRELOAD`, la cual le indica al _loader_ del sistema operativo que tiene que sustituir ciertos _símbolos_ (las funciones de nuestra librería), por las que se indican en el binario.
El resultado de `test-d` debería ser idéntico.

Además, se puede hacer la prueba de intentar ejecutar el binario `test-d` de forma aislada `./test-d`, verificando que efectivamente se utiliza la implementación de la librería estándar de _C_.

## Implementación

Llamaremos **bloque** a una sección contigua de memoria virtual que es administrada por la librería (puede haber más de un _bloque_). Asimismo, llamaremos **región** a aquella sección contigua de memoria virtual que es _devuelta_ por la librería para el usuario; o bien que se encuentra disponible para ser _fraccionada_ o devuelta. Entonces, la librería administrará _bloques_ dividiéndolos en _regiones_.

Al final de este trabajo, la librería tendrá las siguientes funcionalidades:
  - Las firmas de `malloc()`, `calloc()`, `realloc` y `free()` han de ser las mismas que las de la librería estándar de C.
  - Se debe poder _reutilizar_ la memoria liberada en tanto sea posible.
  - Se debe poder _escalar_ en la cantidad de memoria que se le pide al kernel.
  - Se podrá compilar la librería para utilizar diferentes estrategias de _búsqueda de espacio libre_.
  - La implementación debe estar protegida contra _double free_ y _invalid free_.

Para simplificar, tendremos en cuenta las siguientes limitaciones y sugerencias:
  - usar [`mmap(2)`](https://dashdash.io/2/mmap) para la obtención de memoria, **no** [`brk(2)`](https://dashdash.io/2/brk) **ni** [`sbrk(2)`](https://dashdash.io/2/sbrk)
  - definir un tamaño mínimo que siempre se va a devolver, incluso aunque el usuario pida menos memoria (máximo 256 bytes)

**IMPORTANTE**: registrar todas los respuestas a las preguntas y cualquier explicación adicional sobre los detalles del diseño en el archivo `malloc.md`
{:.alert .alert-primary}

### Parte 1: Administrador de bloques

En esta parte se van a implementar las estructuras de datos, y aplicar a _un único bloque_ de memoria solicitado al kernel.

Será necesario definir un _header_ para la _abstracción_ de la _región_, incorporando una forma de marcarlas como ocupadas y libres. Conceptualmente, se debería tener algo como lo siguiente:

```
 --------------------------------------------------------------------
 |   header0   |   ...   |   header1   |   ...        |   header2   |
 | - free: 1   |         | - free: 0   |              | - free: 1   |
 --------------------------------------------------------------------
 <------------->         <-------------->             <------------->
  sizeof(Header)          sizeof(Header)               sizeof(Header) 
```

La librería reservará un **bloque** de 16Kib utilizando `mmap(2)`, durante el primer llamado a `malloc()`, y mantendrá ese bloque hasta que finalice el programa, administrándolo eficientemente. Si en algún momento llega un pedido que supera la cantidad de memoria disponible en el bloque, se recomienda fallar (para esta primera parte).

Además, que en ningún momento (incluso cuando se puedan tener más de un **bloque**) se podrá solicitar una gran cantidad de memoria y administrarla desde el comienzo. La memoria debe pedirse de forma _escalonada_ y a medida que sea necesaria.

También habrá que asegurarse que los pedidos de memoria se registran correctamente, y que la lista de regiones libres siempre está actualizada, introduciendo lógica necesaria para combinar regiones libres (_coalescing_).

<div class="alert alert-primary" markdown="1">
**Tareas**
  - Soportar las funciones: `malloc(3)` y `free(3)`
  - Implementar `first fit` como estrategia de búsqueda de **región** libre
  - Definir la estructura de datos correspondiente a un **bloque** con varias **regiones**
  - Lógica para la administración de un único **bloque** de tamaño fijo
  - Soportar _coalescing_: combinar **regiones** de memoria libres contiguas
  - Soportar _splitting_: separar en dos **regiones** (si la nueva región es lo _suficientemente_ grande [^split])
</div>

[^split]: Es decir, si el tamaño restante es igual o mayor que `sizeof(Header) + MIN_SIZE`

### Parte 2: Agregando más bloques

Extender la lógica anterior para que la librería pueda administrar _más de un bloque_ de memoria a la vez. Se definirán _bloques de distintos tamaños_ y así permitiremos alocamientos de mayor tamaño.

Se definirán tres tamaños de bloque: pequeño (16Kib), mediano (1Mib) y grande (32Mib); y la librería deberá administrar múltiples instancias de cada tamaño de bloque.

El procedimiento debería pasar a ser el siguiente: empezar, como en la parte 1, con un único bloque (el más pequeño). Mientras sea posible, trabajar con éste devolviendo regiones contenidas en él. Si nos quedamos sin memoria en ese bloque, alocar _otro_ bloque, cuyo tamaño sea tan pequeño como sea posible, dentro de las opciones definidas. Es decir, buscar el tamaño de bloque más pequeño en el cual cabe el pedido del usuario.

Si se pidiera más memoria que el bloque de tamaño más grande; entonces la librería debería fallar. Se puede además definir un tamaño máximo para toda la librería (i.e. la suma de todos los bloques administrados nunca podrá exceder tal valor).

A la hora de recibir un pedido de `malloc()`, la librería debería entonces iterar sobre todos los bloques que esté administrando, comenzando por los más pequeños hasta encontrar uno que posea una región apropiada.

Cuando se reciba un `free()` que libere completamente un bloque (i.e. termine conteniendo una única región que englobe el bloque completo), debería ocasionar un `munmap()` de tal bloque; devolviendo así la memoria al usuario.

<div class="alert alert-primary" markdown="1">
**Tareas**
  - Modificar `malloc(3)` y `free(3)` para que utilicen múltiples bloques
  - Definir tres tamaños de **bloque**: pequeño, mediano y grande
  - Agregar el soporte para mantener una cantidad arbitraria de cada tipo de **bloque**
</div>

### Parte 3: Mejorando la búsqueda de regiones libres

En esta sección, se implementará una nueva estrategia de búsqueda denominada "_best fit_".
La misma consiste en poder encontrar la "mejor" región disponible para el tamaño pedido.

Para facilitar la corrección y poder compilar contra ambas implementaciones, el esqueleto y el `Makefile` proveen lo siguiente:

```c
#ifdef FIRST_FIT
   // implementación "First Fit"
#endif

#ifdef BEST_FIT
   // implementación "Best Fit"
#endif
```

<div class="alert alert-primary" markdown="1">
**Tareas**
  - Agregar `best-fit` como estrategia de búsqueda de **región** libre
  - Soportar la función `calloc(3)`
</div>

### Parte 4: Agrandar/Achicar regiones

En esta parte, finalmente, se agregará el soporte para la función `realloc(3)` la cual permite, agrandar o reducir una **región** previamente reservada.

Conceptualmente, el algoritmo para esta función es:

```c
void* realloc(void* ptr, size_t size)
{
    void* res = malloc(size);

    memcpy(res, ptr, old_size);

    free(ptr);

    return res;
}
```

Aunque esta implementación debería funcionar correctamente, no es eficiente en cuanto a la administración de las regiones y bloques libres.
Por ejemplo, podría ser que la región actual, tuviera un tamaño real compatible con el _nuevo tamaño_, siendo la solución óptima, la de reutilizar dicha región. Pero, siguiendo el _pseudocódigo_ se estaría utilizando una nueva región (incluso hasta un nuevo bloque.

<div class="alert alert-primary" markdown="1">
**Tareas**
  - Soportar la función `realloc(3)`
  - El contenido de la región existente **no** debe alterarse (teniendo en cuenta que el _nuvo tamaño_ podría ser menor que el actual)
  - Si el _nuevo tamaño_ es más grande, la nueva memoria **no** debe inicializarse
  - Si `ptr` es igual a `NULL`, el comportamiento es igual a `malloc(size)`
  - Si `size` es igual a _cero_ (y `ptr` no es `NULL`) debería ser equivalente a `free(ptr)`
  - Si falla, la región original **no** debe ser modificada ni liberada
  - Verificar que la región suministrada fue previamente pedida con `malloc(3)`
</div>


## Manejo de errores

El manejo de errores para las funciones a implementar, debería ser consistente con lo detallando en las páginas de manual. En este sentido, se tiene lo siguiente:

- `malloc(3)`, `calloc(3)` y `realloc(3)` devuelven `NULL`
- `free(3)` no devuelve nada

Todas **deben** setear la variable global `errno(3)` con el valor `ENOMEM`.

## Bibliografía útil

A continuación se presentan algunos enlaces y bibliografía útiles como referencia.
  - OSTEP, capítulo 17: [_Free-Space Management_][ostep-malloc] (PDF)
  - The Linux Prgramming Interface, capítulo 7: Memory Allocation
  - Doug Lea: [A Memory Allocator][dlea]
  - Dan Luu: [A Malloc tutorial][danluu]
  - Marwan Burelle: [A Malloc Tutorial][marwan] (PDF)

[dlea]: https://gee.cs.oswego.edu/dl/html/malloc.html
[danluu]: https://danluu.com/malloc-tutorial/
[ostep-malloc]: https://pages.cs.wisc.edu/~remzi/OSTEP/vm-freespace.pdf
[marwan]: https://wiki-prog.infoprepa.epita.fr/images/0/04/Malloc_tutorial.pdf

{% include anchors.html %}
{% include footnotes.html %}
