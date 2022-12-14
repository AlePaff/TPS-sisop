# malloc

Repositorio para el esqueleto del [TP malloc](https://fisop.github.io/website/tps/malloc/) del curso Mendez-Fresia de **Sistemas Operativos (7508) - FIUBA**

## Compilar

```bash
$ make
```

## Compilar (usando FF o BF)
Para First fit
```bash
$ make -B -e USE_FF=true
```
Para Best fit
```bash
$ make -B -e USE_BB=true
```

## Compilar la librería

```bash
$ make libmalloc.so
```

## Ejecutar prueba (estática)

```bash
$ make USE_FF=true run-s        # First fit, usar USE_BB para Best fit
$ ./test-s
```

## Ejecutar prueba (dinámica)

```bash
$ make USE_FF=true run-d        # First fit, usar USE_BB para Best fit
$ ./test-d
```

## Linter

```bash
$ make format
```

