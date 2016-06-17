// Funciones del tipo de datos secuencia musical
// para PostgreSQL.
// Autor: Carlos Gómez

#include <postgres.h>
#include <fmgr.h> 
#include <libpq/pqformat.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define entero_t int4
#define LNGTAM VARHDRSZ

#include "funcion_secuencia.h"

Datum secuencia_in(PG_FUNCTION_ARGS);
Datum secuencia_out(PG_FUNCTION_ARGS);
Datum secuencia_send(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(secuencia_in);

Datum secuencia_in(PG_FUNCTION_ARGS) {
    char *s = PG_GETARG_CSTRING(0);
    int num_notas = contarNotas(s);
    Secuencia *notas = (Secuencia*)palloc(VARHDRSZ + sizeof(unsigned int) + 
                                          sizeof(int) * num_notas);
    if (notas == 0) {
        PG_RETURN_POINTER(notas);
        ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR)));
    }
    notas->tamano = VARHDRSZ + sizeof(unsigned int) + sizeof(int) * num_notas;
    int error = secuenciaIn(s, notas);
    if (error == -1) {
        pfree(notas);
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("sintaxis de entrada incorrecta para secuencia: \"%s\"",
                        s)));
        PG_RETURN_POINTER(0);
    } else if (error == -2) {
        pfree(notas);
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
                 errmsg("la secuencia de notas excede la longitud máxima")));
        PG_RETURN_POINTER(0);
    } else {
        PG_RETURN_POINTER(notas);
    }
}

PG_FUNCTION_INFO_V1(secuencia_out);

Datum secuencia_out(PG_FUNCTION_ARGS) {
    Secuencia *notas = (Secuencia*)PG_GETARG_POINTER(0);
    char *salida = (char*)palloc(15000); // TODO
    int ok = secuenciaOut(notas, salida);
    if (ok == -1) {
        pfree(salida);
        ereport(ERROR,
                (errcode(ERRCODE_STRING_DATA_LENGTH_MISMATCH),
                 errmsg("la longitud de la secuencia excede la " \
                        "longitud esperada")));
    } else if (ok == -2) {
        pfree(salida);
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_BINARY_REPRESENTATION)));
    }
    PG_RETURN_CSTRING(salida);
}

PG_FUNCTION_INFO_V1(secuencia_send);

Datum secuencia_send(PG_FUNCTION_ARGS) {
    Secuencia *notas = (Secuencia *)PG_GETARG_POINTER(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendbytes(&buf, (char *)notas, notas->tamano);
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

#include "funcion_secuencia.c"
