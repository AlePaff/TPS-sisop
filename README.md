# fisopfs

Sistema de archivos tipo FUSE.

## Pruebas y sus salidas

#### Creación de directorios (con mkdir)
```bash
$ mkdir mi_carpeta
$ ls
mi_carpeta
```


#### Creación de archivos (touch, redirección de escritura)
```bash
$ touch archivo_hola
$ ls
archivo_hola
$ echo "Hola mundo" > archivo_hola
$ cat archivo_hola
Hola mundo
```


#### Lectura de directorios, incluyendo los pseudo-directorios . y .. (con ls)
```bash
$ touch archivo.txt
$ ls
archivo_hola.txt
$ ls -a
. .. archivo_hola.txt 
```

#### Lectura de archivos (con cat, more, less, etc)
```bash
$ echo "este archivo tiene contenido" > file
$ cat file
este archivo tiene contenido
```
comandos de lectura de archivos grandes usando more, less, head, tail
```bash
#uso de more
$ seq 20 > file
$ more file
1
2
...
14
--More--(64%)

# uso de less
$ less file
1
2
...
14
file

# uso de head
$ head file
1
2
...
10

# uso de tail
$ tail file
11
12
...
20
```

#### Escritura de archivos (sobre-escritura y append con redirecciones)
```bash
# append
$ echo "cosas de archivo" > file
$ cat file
cosas de archivo
$ echo "mas cosas de archivo" >> file
$ cat file
cosas de archivo
mas cosas de archivo

# sobre-escritura
$ echo "sobre-escritura" > file
$ cat file
sobre-escritura
```


#### Acceder a las estadísticas de los archivos (con stat)
```bash
$ touch archivo.txt
$ ls
archivo.txt
$ stat archivo.txt
# salida similar a la siguiente
  File: archivo.txt
  Size: 0         	Blocks: 4          IO Block: 4096   regular empty file
Device: 2eh/46d	Inode: 2           Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/     ale)   Gid: (    0/    root)
Access: 2022-12-10 14:27:53.000000000 -0300
Modify: 2022-12-10 14:27:53.000000000 -0300
Change: 2022-12-10 14:27:53.000000000 -0300
 Birth: -
```
#### Incluir y mantener fecha de modificación y creación
```bash
$ touch file
$ stat file
    File: file
    ...
    Access: 2022-12-10 14:37:06.000000000 -0300
    Modify: 2022-12-10 14:37:06.000000000 -0300
# cerrar el sistema de archivos con 'sudo umount directorio_raiz'
# volver a montar el sistema de archivos
$ cd directorio_raiz
$ ls
file
$ stat file
    File: file
    ...
    Access: 2022-12-10 14:37:06.000000000 -0300
    Modify: 2022-12-10 14:37:06.000000000 -0300
    # misma salida que la anterior
```

#### Borrado de un archivo (con rm o unlink)
```bash
$ touch archivo
$ touch ejemplo.txt
$ ls
archivo ejemplo.txt
$ rm archivo
$ ls
ejemplo.txt
```
#### Borrado de un directorio (con rmdir)
Eliminar un directorio vacío
```bash
$ mkdir ejemplo
$ mkdir TPS
$ ls
ejemplo TPS
$ rmdir ejemplo
$ ls
TPS
```
Directorio no vacío
```bash
$ mkdir dir
$ touch dir/file
$ cd dir
$ ls
file
$ cd ..
$ rmdir dir
$ ls
```

## pruebas semiautomaticas
hay 4 archivos .sh para correr test de forma automatica. Para usarlos se deben tener 2 terminales abiertas, una para el filesystem (terminal A)
y otra para las pruebas sobre el filesystem (terminal B). Estos son los pasos

en terminal A:
./start.sh

en terminal B:
./test.sh >test1

en terminal A:
interrumpir el filesystem con ctrl+c
./restart.sh

en terminal B:
./test2.sh >test2

