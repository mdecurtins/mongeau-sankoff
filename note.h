/* note.h
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef note_h
#define note_h

#include "global.h"

#include <string>

class note {
    static char* solfa[]; // array of note names used for printing

public:
    int pitch;
    int duration;
    
    note() {};
    note(int p, int d) : pitch(p), duration(d) {};
    note operator+ (int t) const {
        return note(pitch + t, duration);
    };
    note operator- (int t) const {
        return note(pitch - t, duration);
    };
    std::string print() const;
};

#endif
