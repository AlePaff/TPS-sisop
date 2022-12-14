
Decidimos utilizar un diseño parecido al que usa Linux para representar los archivos, en nuestro caso tenemos:
* Superbloque
* Inodos
* Bloques de datos
* Bitmaps


## Superbloque
Es la estructura que contiene información del file system. Contiene: 

* <u>Número mágico</u>: El cual sirve para identificar el filesystem (aunque no se use dentro del mismo)

* <u>Cantidad de bloques de datos y cantidad de bloques de inodo</u>:
Decidimos que cada inodo podría tener hasta 4 bloques de datos por lo que la cantidad de bloque de datos va a ser 4 veces más que la cantidad de inodos, elegimos 100 inodos y 400 bloques de datos.


* <u>Un bitmap para los bloques de datos y otro bitmap para los inodos</u>:
Aunque los llamamos bitmaps en realidad son array de int. Decidimos manejarnos de esta forma para evitar bitwise operations y simplificar su uso sin perder la idea original del bitmat. Dentro de los bitmaps de datos e inodos decidimos representar los bloques libres con un `0` y los ocupados como un `1`.


## Inodo
Un inodo es un struct que guarda la metadata tanto de los archivos regulares como de los directorios. Entre estos conserva varios campos como su inumero, el tamaño de un archivo, el pid, los permisos que tenga, su fecha de creación, su fecha de modificacion, el nombre del archivo que va a ser el path completo, el uid y un arreglo con los índices de los bloques de datos que correspondan al inodo.

Esto último fue hecho para serializar más facilmente, ya que guardar solamente los punteros a los bloques no nos serviría de nada al ser distintos cada vez que se monta el filesystem. 
El arreglo tiene un máximo de `4 bloques de datos por inodo`, fue una decisión arbritraria que nos pareció razonable. También decidimos que si el arreglo tiene un `-1` se entendería que no hay nada guardado en ese bloque de datos.

A su vez, inodo, también tiene un arreglo que apunta a los bloques para poder acceder a los datos y una variable tipo `int` que indica si es un archivo `regular` o un `directorio`, elegimos que los directorios sean `0` y los archivos `1`.

## Bloque de datos
Un bloque de datos es un array de 4096 caracteres. Decidimos este número por ser el tamaño de una page en linux (hay más tendencia a haber muchos archivos pequeños que pocos grandes). Se guarda como struct para abstraernos del array.


## Uso de constantes
|Constante|Valor|Descripción|
|-|-|-|
`CANT_INODOS`|100|Es la cantidad de inodos que tendrá el sistema, elegida arbitrariamente
`BLOCK_DATA_NUMBER`|400|Es la cantidad de bloques de datos que tendrá el sistema, elegida arbitrariamente
`DATABLOCKMAX`|4|cantidad de bloques de datos que puede tener un inodo.
`BLOCKSIZE`|4096|tamaño de cada bloque, elegido arbitrariamente.
`MAX_NOMBRE`|60|largo de nombre de archivo, path incluido
`MAX_NOMBRE_DIR`|55|largo de nombre para directorio, path incluido. Es menor a `MAX_NOMBRE` para dejar espacio en el path a los archivos que se vayan a crear dentro
`OCUPADO`|1|constante booleana para los bitmap
`LIBRE`|0|constante booleana para los bitmap
`DIR_T`|0|tipo de archivo directorio
`REG_T`|1|tipo de archivo regular


## Aclaraciones
* Los directorios no tienen límite en cuanto a archivos que soportan que no sea otro que el número de archivos que soporta todo el filesystem.

* El filesystem soporta directorios anidados. El límite de cuántos directorios anidados se pueden tener está dado por el límite de `MAX_NOMBRE_DIR`, depende de cuán largo es el nombre.

* La forma en la que se encuentra un archivo especificado por un path lo hace la función `buscar_inodo()` que recibe un path y devuelve la posición donde se encuentra el inodo correspondiente en el array de inodos. Si no se encuentra devuelve un `-1`.