/* Algorithm.h
 *
 * Copyright (C) 2006 Carlos G�mez
 * Universidad Sim�n Bol�var
 */

#ifndef Algorithm_h
#define Algorithm_h

#include "Sequence.h"
#include "Result.h"
#include "global.h"

// alignment types
typedef enum {
    Global,
    Start,
    VarLenStart,
    AllToSome,
    Overlap,
    Local
} Alignment;

class Algorithm {
public:

    virtual Result compare(Sequence& q, Sequence& t) = 0;

    virtual ~Algorithm() {};

};

#endif
