/* note.cpp
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "note.h"

#include <string>
#include <sstream>

using namespace std;

char *note::solfa[] = {"do", "#do", "re", "#re", "mi", "fa", "#fa", "sol",
                       "#sol", "la", "#la", "ti"};

string note::print() const {
    stringstream s;
    int degree = pitch % 12;
    int octave = pitch / 12 - 5;
    s << solfa[degree];
    string symbol = "^";
    if (octave < 0) {
        symbol = ",";
        octave = -octave;
    }
    for (int k = 0; k < octave; k++) {
        s << symbol;
    }
    s << duration;
    return s.str();
}
