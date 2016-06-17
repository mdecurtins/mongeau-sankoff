/* Sequence.cpp
 * Definition of the musical sequence data type. A musical sequence 
 * is defined as a vector of (pitch, IOI) pairs. The first element
 * has index 1.
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "Sequence.h"

#include <string>
#include <sstream>

using namespace std;

/* This constructor creates a new Sequence from a low-level representation
 * of the sequence consisting of an array of unsigned integers. In this 
 * representation the first element is the size in bytes of the array, including
 * this element. The second element is not used in this program and is equal to 
 * the size in bytes of the printed representation of the sequence. 
 * Each element from the third onwards represents a note of the sequence. 
 * The first 7 bits of the word store the pitch, and the remaining 25
 * bits, the duration in integer format. 
 */
Sequence::Sequence(unsigned int *binary_seq, unsigned int size) {
    size = (size / sizeof(int)) - 2;
    this->size = size;
    seq = new note[size];
    int max = -1;
    int min = 128;
    note* iter = seq;
    for (unsigned int i = 2; i < size + 2; i++) {
        note c;
        unsigned int duration = (binary_seq[i] & (unsigned int)DURATION);
        int pitch = (binary_seq[i] & (unsigned int)PITCH) >> 25;
        //c.duration = (float)duration / 240;
        c.duration = duration;
        c.pitch = pitch;
        *iter = c;
        if (c.pitch > max) {
            max = pitch; 
        }
        if (c.pitch < min) {
            min = pitch;
        }
        iter++;
    }
    min_pitch = min;
    max_pitch = max;
}

/* Multiply all the onset times of the sequence by a constant. */
void Sequence::scale(float a) {
    if (a == 1) {
        return;
    }
    for (Iterator iter = begin(); iter != end(); iter++) {
        (*iter).duration *= a;
    }
}

/* Transform pitches to one-octave representation (mod 12). */
void Sequence::mod12() {
    for (Iterator iter = begin(); iter != end(); iter++) {
        (*iter).pitch %= 12;
    }
}

void Sequence::quantize() {
    for (Iterator iter = begin(); iter != end(); iter++) {
        int c = durationClass(iter->duration);
        iter->duration = c;
    }
}

/* Returns the sum of durations of the notes of the sequence. */
int Sequence::totalDuration() {
    int d = 0;
    for (Iterator iter = begin(); iter != end(); iter++) {
        d += iter->duration;
    }
    return d;
}

string Sequence::print() const {
    stringstream s;
    for (int i = 0; i < size; i++) {
        s << seq[i].print() << " ";
    }
    return s.str();
}
