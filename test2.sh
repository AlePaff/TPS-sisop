#!/bin/bash


cd prueba

echo "deberiamos tener dir1, dir2, file1, file2, file3 y file4 al hacer ls"

ls

echo "====================================="

echo "al hacer cat file2 deberia tener file1 y file2"

cat file2

echo "====================================="

echo "cat file4 deberia tener la seq 15"

cat file4

echo "====================================="

echo "entro a dir1 que tenia cosas"

cd dir1

echo "ls deberia mostrar archivo_hola, file y el directorio otro_dir"

ls

echo "====================================="

echo "veamos los stats de archivo_hola"

stat archivo_hola

echo "y su contenido"

cat archivo_hola

echo "====================================="

echo "entramos a otro_dir y al hacer ls deberia verse dir_final"

cd otro_dir

ls

echo "====================================="

echo "entro a dir_final y deberia verse el archivo me_encontraste al hacer ls"

cd dir_final

ls

echo "====================================="

echo "y al hacer cat deberia tener el string que se guardo"

cat me_encontraste

echo "====================================="


