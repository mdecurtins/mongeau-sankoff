// funcion_secuencia.c
// Definición del tipo de datos secuencia musical. Define las funciones
// de entrada y salida de texto y de salida binaria. 
// Autor: Carlos Gómez

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

char* escala[] = {"do", "#do", "re", "#re", "mi", "fa", "#fa", "sol", "#sol", "la", "#la", "si"};

inline unsigned int crearNota(unsigned int nota, unsigned int duracion) {
    unsigned int n = (nota << 25) | duracion;
    return n;
}

inline int esNota(char c) {
    return (c == 'd') || (c == 'r') || (c == 'm') || (c == 'f') || (c == 's')
           || (c == 'l');
}

int contarNotas(char* s) {
    int espacio = 1;
    int num_notas = 0;
    char *i = s;
    while (*i != '\0') {
        if (isspace(*i)) {
            if (espacio == 0) {
                num_notas++;
                espacio = 1;
            }
        } else {
            espacio = 0;
        }
        i++;
    }
    if (espacio == 0) {
        num_notas++;
    }
    return num_notas;
}

// Devuelve la representación binaria de una secuencia de 
// notas dada como una cadena de texto. 
// *s     (in-out) cadena de texto que representa una secuencia musical
// *notas (in-out) espacio de memoria que contiene la representación binaria
// Al llamar a la función *notas debe ser un espacio de memoria reservado
// lo suficientemente grande para guardar la representación binaria. En caso de
// error, la función devuelve -1 y *s apunta a la posición del string que 
// no se pudo interpretar. 
int secuenciaIn(char *s, Secuencia *notas) {
    int estado = 0; // 0 = espacio, 1 = sostenido, 2 = nota, 3 = octava arriba, 
                    // 4 = actava abajo, 5 = duracion
    int j = 0; // recorre el arreglo notas->datos
    int sostenido, octava;
    unsigned int nota, duracion;
    octava = 0;
    sostenido = 0;
    unsigned long total_caracteres = 0;
    char  *i = s;
    while (*i != '\0') {
        if (estado == 0) {
            // espacio
            if (isspace(*i)) {
                i++;
            } else if (*i == '#') {
                estado = 1;
                i++;
            } else if (esNota(*i)) {
                estado = 2;
            } else {
                return -1;
            }
        } else if (estado == 1) {
            // sostenido
            sostenido = 1;
            total_caracteres++;
            if (esNota(*i)) {
                estado = 2;
            } else {
                return -1;
            }
        } else if (estado == 2) {
            // nota
            char snota[4];
            snota[3] = '\0';
            strncpy(snota, i, 3);
            if (strcasecmp("sol", snota) == 0) {
                nota = 67;
            } else {
                snota[2] = '\0';
                if (strcasecmp("do", snota) == 0) {
                    nota = 60;
                } else if (strcasecmp("re", snota) == 0) {
                    nota = 62;
                } else if (strcasecmp("mi", snota) == 0) {
                    nota = 64;
                } else if (strcasecmp("fa", snota) == 0) {
                    nota = 65;
                } else if (strcasecmp("sol", snota) == 0) {
                    nota = 67;
                } else if (strcasecmp("la", snota) == 0) {
                    nota = 69;
                } else if (strcasecmp("si", snota) == 0) {
                    nota = 71;
                } else {
                    return -1;
                }
            }
            if (nota == 67) { // if (nota == Sol)
                i += 3;
                total_caracteres += 3;
            } else {
                i += 2;
                total_caracteres += 2;
            }
            if (*i == '^') {
                estado = 3;
            } else if (*i == ',') {
                estado = 4;
            } else if (isdigit(*i)) {
                estado = 5;
            } else {
                return -1;
            }
        } else if (estado == 3) {
            // octava arriba
            octava += 1;
            i++;
            total_caracteres++;
            if (*i == '^') {
                // estado = 3
            } else if (isdigit(*i)) {
                estado = 5;
            } else {
                return -1;
            }
        } else if (estado == 4) {
            // octava abajo
            octava -= 1;
            i++;
            total_caracteres++;
            if (*i == ',') {
                // estado = 4
            } else if (isdigit(*i)) {
                estado = 5;
            } else {
                return -1;
            }
        } else if (estado == 5) {
            // duracion
            char *final;
            errno = 0;
            long l = strtol(i, &final, 10);
            if (isdigit(*final) || (errno != 0) || (l > DURACION_MAX)) {
                return -1;
            }
            duracion = l;
            total_caracteres += snprintf((char*)0, 0, "%ld", l);
            i = final;
            if (isspace(*i) || *i == '\0') {
                // agregar la nota a la secuencia
                nota = nota + octava * 12;
                if (sostenido) nota++;
                if ((nota > 127) || (nota < 0)) {
                    return -1;
                }
                notas->datos[j] = crearNota(nota, duracion);
                total_caracteres += 1; // espacio después de la nota
                j++;
                octava = 0;
                sostenido = 0;
                if (*i != '\0') i++;
                estado = 0;
            } else {
                return -1;
            }
        }
    }
    if (total_caracteres <= UINT_MAX) {
        notas->long_texto = (unsigned int)total_caracteres;
        return 0;
    } else {
        // el número de caracteres excede la longitud máxima
        return -2;
    }
}

int secuenciaOut(Secuencia *notas, char* salida) {
    int tam = notas->tamano - LNGTAM - sizeof(unsigned int);
    unsigned int max = notas->long_texto + 1; // texto | \0
    char *salida1 = salida;
    unsigned int total = 0;
    unsigned int *i = notas->datos;
    unsigned int *fin = notas->datos + (tam / sizeof(unsigned int));
    for (; i < fin; i++) {
        unsigned int nota = (*i);
        nota = nota & NOTA;
        unsigned int duracion = *i;
        duracion = duracion & DURACION;
        nota = nota >> 25;
        int octava = (nota / 12) - 5;
        nota = nota % 12;
        char s_octava[6];
        char simbolo;
        if (octava < 0) {
            simbolo = ',';
            octava = -octava;
        } else {
            simbolo = '^';
        }
        if ((octava > 5) || (octava < -5)) {
            *salida = '\0';
            return -2;
        }
        int k;
        for (k = 0; k < octava; k++) {
            s_octava[k] = simbolo;
        }
        s_octava[k] = '\0';
        int avance = snprintf(salida1, max - total, "%s%s%u ", escala[nota], s_octava, duracion);
        //if (avance >= max - total) {
        //    *salida = '\0';
        //    return -1;
        //}
        total += avance;
        salida1 += avance;
    }
    return 0;
}
