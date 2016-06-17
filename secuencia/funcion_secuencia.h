// funcion_secuencia.h
// Declaraciones para el tipo de datos secuencia musical.

#ifndef funcion_secuencia_h
#define funcion_secuencia_h

#define DURACION_MAX 33554431 // 2^25 - 1
#define NOTA 4261412864U // 11111110 00000000 00000000 ...
#define DURACION 33554431 // 00000001 11111111 11111111 ...

typedef struct {
    entero_t tamano;
    unsigned int long_texto;
    unsigned int datos[1];
} Secuencia;

int secuenciaIn(char*, Secuencia*);
int secuenciaOut(Secuencia*, char*);
int contarNotas(char*);

#endif
