/* global.h
 * Global program definitions.
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef global_h
#define global_h

extern int v_level; // verbosity level

inline bool verb(int v) {
    return (v <= v_level);
}

void warning(char* msg);

#endif
