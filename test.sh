#!/bin/bash

cd prueba

touch file1

echo "solo deberia estar file1"

ls

echo "====================================="

ls >file2

mkdir dir1

stat file2 >file3

echo "deberia imprimir los stats de file2 guardados en file3"

cat file3

echo "====================================="

seq 2500 >file4

stat file4 >>file3

seq 15 >file4

stat file4 >>file3

echo "deberia verse lo que tenia file3 antes y 2 bloques de stats de file4 cuando tenia seq 2500 y luego con seq 15"

cat file3

echo "====================================="

mkdir dir2

cd dir2

mkdir dir1

stat dir1 >file7

echo "se ven los stats de dir1 guardados en file7"

cat file7

cd dir1

touch file1

echo "string string" >file1

cd ..

stat dir1 >>file7

cd dir1

rm file1

cd ..

stat dir1 >>file7

echo "se ven los stats originales de dir1 luego los stats cuando se creo un archivo adentro y luego cuando se borro dicho archivo"

cat file7

echo "====================================="

cd ..

rm file3

touch file3

echo "se borro file3 y se volvio a crear, no deberia verse nada"

cat file3

echo "====================================="

rmdir dir2

mkdir dir2

cd dir2

echo "se borro todo el dir 2 con todos sus archivos y directorios dentro, luego se creo otro, se entro y al hacer ls no deberia verse nada"

ls

echo "====================================="

echo "seteo mas archivos para la prueba de persistencia"

echo "estoy en dir raiz"

cd ..

echo "entro al dir1"

cd dir1

echo "creo un archivo hola mundo"

echo "Hola mundo" > archivo_hola

echo "creo un archivo file con >"

echo "cosas de archivo" > file

echo "le agrego cosas a file con >>"

echo "mas cosas de archivo" >> file

echo "el archivo file en cuestion:"

cat file

echo "====================================="

echo "creo un dir llamado otro_dir"

mkdir otro_dir

echo "entro a otro_dir"

cd otro_dir

echo "creo un dir llamado dir final"

mkdir dir_final

echo "entro a dir_final"

cd dir_final

echo "creo un file guardandole un string"

echo "este archivo esta en /dir1/otro_dir/dir_final" >me_encontraste

echo "el file en cuestion"

cat me_encontraste

echo "====================================="

exit
