// parsemusic.c

#define entero_t unsigned int
#define LNGTAM sizeof(unsigned int)

#include <stdlib.h>

#include "parsemusic.h"
#include "funcion_secuencia.h"

int* parse(char* s) {
    int num_notas = contarNotas(s);
    Secuencia *notas = (Secuencia*)malloc(2 * sizeof(unsigned int) + 
                                          sizeof(int) * num_notas);
    if (notas <= 0) {
        return 0;
    }
    notas->tamano = 2 * sizeof(unsigned int) + sizeof(int) * num_notas;
    int error = secuenciaIn(s, notas);
    if (error != 0) {
        return 0;
    }
    return (int*)notas;
}

#include "funcion_secuencia.c"
