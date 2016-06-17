// musicseqmodule.c
// Módulo de Python para leer secuencias musicales.

#include <Python.h>

#define entero_t int
#define LNGTAM sizeof(int)

#include "funcion_secuencia.h"

static PyObject *musicseq_parse(PyObject*, PyObject*);

static PyMethodDef musicseq_methods[] = {
    {"parse", (PyCFunction)musicseq_parse, METH_VARARGS,
     "Crea una secuencia musical a partir de su representación de texto."},
    {NULL, NULL, 0, NULL} /* sentinel */
};

static PyObject *musicseq_parse(PyObject *self, PyObject *args) {
    char *txt;
    if (!PyArg_ParseTuple(args, "s", &txt)) {
        return NULL;
    }
    int num_notas = contarNotas(txt);
    size_t tam = sizeof(entero_t) + sizeof(unsigned int) 
        + sizeof(int) * num_notas;
    Secuencia *notas = (Secuencia*)malloc(tam);
    if (notas == 0) {
        return PyErr_NoMemory();
    }
    notas->tamano = tam;
    int ok = secuenciaIn(txt, notas);
    if (ok == -1) {
        free(notas);
        PyErr_SetString(PyExc_ValueError, "sintaxis incorrecta en la secuencia " \
                        "de notas");
        return NULL;
    } else if (ok == -2) {
        free(notas);
        PyErr_SetString(PyExc_OverflowError, "la secuencia de notas excede " \
                        "la longitud máxima");
        return NULL;
    } else {
        return PyBuffer_FromMemory(notas, tam);
    }
}

PyMODINIT_FUNC initmusicseq() {
    (void) Py_InitModule("musicseq", musicseq_methods);
}

#include "funcion_secuencia.c"
