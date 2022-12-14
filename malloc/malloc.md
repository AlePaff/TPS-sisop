# TP: malloc



## Diseño

La administración de la memoria está compuesta por tres listas globales de bloques de distintos tamaños: pequeño (con espacio `SMALLSIZE`), mediano (con espacio `MEDIUMSIZE`) y grande (con espacio `LARGESIZE`).

Dentro de cada lista de bloques hay n bloques de tamaño fijo, y dentro de cada bloque, las regiones de datos de tamaño variable que serán devueltas al usuario.

Tanto bloques como regiones son listas enlazadas.

### Diseño de Malloc:

Cuando se llama a un `malloc(size)` primero se verifica que `size` cumpla con ciertos requisitos, siendo estos:
- Mayor a 0 (si no lo es devuelve null).
- Menor a nuestro `LARGESIZE` (`LARGESIZE` menos los tamaños de nuestros headers. Si no lo es devuelve null y setea errno).
- Mayor o igual a `MINSIZE` (espacio mínimo de memoria a reservar: 256 bytes). Si no lo cumple, se setea size a `MINSIZE`. 
- Alineado a 32 bytes, ya que nuestros headers tienen ese tamaño y por lo tanto cada header debe quedar alineado a su tamaño. [^align]

Una vez hecho esto, se fija si ya existe un bloque donde se pueda guardar (haciendo uso de `find_free_region(size)`). En caso que todavía no exista un bloque que pueda alocar la memoria pedida, se elige el tamaño del nuevo bloque a crear dependendiedo de su relación con las constantes `SMALLSIZE`, `MEDIUMSIZE` y `LARGESIZE`.

#### Encontrar una región libre: Politicas first fit y best fit

Si se considera la política **first fit**, se va a recorrer la lista de bloques hasta encontrar una región libre que sea lo suficientemente grande como para contener el tamaño pedido. Se empezará por la lista de bloques de menor tamaño posible (teniendo en cuenta el tamaño pedido). De no encontrarse se pasa a la siguinte lista de bloques de menor tamaño y se repite. 

Si se considera la politica **best fit**, se deberán recorrer las tres listas de bloques hasta encontrar el mejor fit, que es la región más pequeña dentro de las que cumplen con el tamaño pedido. 

Una vez encontrado el lugar se verifica si a continuación de éste hay suficiente espacio para un splitting, y de ser así se hace. Caso contrario se toma todo el espacio disponible.

De no encontrar una región libre disponible, sea la política que sea, se crea un nuevo bloque del menor tamaño posible y se aloja en el mismo. 

Si es la primera vez que se llama a malloc y por lo tanto todavía no existe ningún bloque o regiones, caerá directamente en el último caso mencionado. 

Diagrama de malloc para un bloque pequeño:

![malloc](https://i.imgur.com/bo0p60b.png)


### Diseño de Free:

Lo primero que hace free es verificar si se le pasó un puntero nulo, en cuyo caso no hace nada y termina la ejecución de la función.

En caso contrario, se intenta leer la metadata de la región. Si se le pasó un puntero que no fue reservado con malloc da _segfault_.

Si no ocurre ninguno de los dos casos anteriores se busca la región a donde pertenece dicho puntero para liberarlo, es decir, marcarlo como free (en su bloque correspondiente).
Luego se verifica dentro del bloque si hay que hacer **coalescing**. Coalescing consiste en recorrer el bloque al que pertenece la región liberada, revisando una por una cuáles están libres y, en caso de encontrar una libre, se verifica si la siguiente también lo está. De ser así, la primera región modifica su header para abarcar el espacio de ambas, "uniéndolas" en una sola región.

Una vez hecho esto se verifica si el bloque con el que nos quedamos está completamente libre. De ser así se lo separa de la lista de bloques, verificando que ésta no quede rota, y se devuelve la memoria que ocupaba el bloque con `munmap()`.

### Diseño de Calloc:

Calloc llama a malloc recibiendo el puntero y tamaño correspondientes, seteando todo su contenido a 0 antes de devolver el puntero.

### Diseño de Realloc:

De forma similar a `malloc()`, cuando se llama a un `realloc(ptr,size)` primero se verifica que `size` cumpla con ciertos requisitos, siendo estos:
- mayor a 0 (si no lo es se hace free del puntero).
- menor a nuestro `LARGESIZE` (`LARGESIZE` menos los tamaños de nuestros headers, si no lo es devuelve null y setea `errno`).
- Mayor o igual a `MINSIZE` (espacio mínimo de memoria a reservar: 256 bytes). Si no lo cumple, se setea size a `MINSIZE`. 
- Alineado a 32 bytes (ya que nuestros headers tienen ese tamaño y por lo tanto cada header debe quedar alineado a su tamaño). De no ser así, se alinea. [^align]

Si el nuevo tamaño es **menor** al original pueden pasar varias cosas:
- <u>La siguiente región está libre</u>: se puede achicar la región que se pasó y darle la parte libre "sobrante" a la siguiente región vía coalescing. 
- <u>La siguiente región no está libre</u>: pueden ocurrir dos cosas:
    - Al achicar la región queda suficiente espacio entre mi región y la siguinte como para hacer splitting, por lo que se hace.
	- No hay suficiente espacio para hacer un splitting, no se hace nada.

Si el nuevo tamaño es **mayor** pueden pasar varias cosas:
- <u>La siguente región está libre</u>: en este caso pueden ocurrir dos cosas:
	- si la siguiente región tiene tamaño mayor o igual al espacio extra que se pide, se toma dicha parte y se hace splitting del resto (si hay espacio suficiente para hacer splitting) o se toma todo el espacio disponible (si el remanente no alcanza para un splitting).
	- Si no es lo suficientemente grande para alocar al espacio extra, es como si estuviera ocupada y por lo tanto se hace lo mismo que en ese caso.
- <u>La siguiente región no está libre</u>: en este caso habrá que hacer un malloc por el nuevo tamaño, copiar en la nueva región todo lo que tenía antes y hacer free de la vieja región.

El resto de las funciones son autoexplicativas y se encuentran en el código.


## Tests
* `amount_of_mallocs`, `amount_of_frees` y `requested_memory` son contadores globales que se incrementan cada vez que se llaman a las funciones, son utilizados para llevar un registro para poder testear la correcta ejecución de las funciones.

* `print_test()` es una función que imprime por pantalla el resultado de un test, y devuelve `OK` si el test fue exitoso, o `ERROR` si no lo fue. Dentro lleva un contador de cuántas pruebas fallaron y cuantas tuvieron éxito.



## Observación sobre los warnings

Los warnings que van a ocurrir durante la compilación son los siguientes:
* `[-Wunused-but-set-variable]`, para la variable `tam` en la linea `148` de `malloc.c`. Debido a las cláusuras de `#ifdef` para FF y BF. Entonces en tiempo de compilacion esta variable no es utilizada, pero si lo es en tiempo de ejecución.
* `[-Walloc-size-larger-than=]`, ya que en las pruebas estamos mandando el valor `-1` a malloc, realloc, etc. quienes reciben un `size_t`, cuyo valor no puede ser negativo. Esta misma advertencia aparecería con la librería estandar


[^align]: Este tamaño ya es fijo por lo tanto no vimos la necesidad de hacerlo de forma genérica.
