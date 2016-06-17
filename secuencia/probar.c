// probar.c

#define entero_t int
#define LNGTAM sizeof(int)

#include <stdlib.h>
#include <stdio.h>

#include "funcion_secuencia.h"

Secuencia* secuencia_in_c(char* s) {
    int num_notas = contarNotas(s);
    Secuencia *notas = (Secuencia*)malloc(2 * sizeof(unsigned int) + 
                                          sizeof(int) * num_notas);
    if (notas <= 0) {
        return 0;
    }
    notas->tamano = 2 * sizeof(unsigned int) + sizeof(int) * num_notas;
    int error = secuenciaIn(s, notas);
    if (error != 0) {
        printf("error %d interpretando la secuencia\n", error);
        return 0;
    }
    return notas;
}

char* secuencia_out_c(Secuencia *notas) {
    char *salida = (char*)malloc(1000);
    int error = secuenciaOut(notas, salida);
    if (error != 0) {
        printf("error %d imprimiendo la secuencia\n", error);
        return 0;
    }
    return salida;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("\nUso: probar <secuencia>\n\n");
        return 0;
    }
    Secuencia* sec = secuencia_in_c(argv[1]);
    if (sec == 0) {
        return -1;
    }
    char* texto = secuencia_out_c(sec);
    if (texto == 0) {
        return -1;
    }
    printf("%s\n", texto);
    return 0;
}

#include "funcion_secuencia.c"
