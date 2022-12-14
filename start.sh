#!/bin/bash



make clean

rm myfilesystem.fisopfs

make  

mkdir prueba

./fisopfs -f prueba


