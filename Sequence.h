/* Sequence.h
 * Definition of the musical sequence data type. A musical sequence 
 * is defined as a vector of (pitch, IOI) pairs. The first element
 * has index 1.
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef Sequence_h
#define Sequence_h

#include "note.h"
#include "global.h"

#include <string>
#include <cmath>

#define PITCH 4261412864U
#define DURATION 33554431

static const float log_30(1.4771212547196623799);
static const float log_3840(3.5843312243675309325);

class Sequence {
    note *seq;
    int size;
    static char *solfa[]; // array of note names used for printing
    
public:
    int max_pitch;
    int min_pitch;

    Sequence(int size);
    Sequence(unsigned int *binary_seq, unsigned int size);
    Sequence(const Sequence& s1);
    int length() const;
    note operator() (int);
    const note operator() (int) const;
    void scale(float a);
    void mod12();
    void quantize();
    static int durationClass(float duration);
    int totalDuration();
    std::string print() const;
    ~Sequence();
    
    typedef note* Iterator;
    Iterator begin() const {return seq;};
    Iterator end() const {return seq + size;};

    class IndexOutOfBounds {}; // out of bounds exception
};

/* Creates a new sequence of the specified size. */
inline Sequence::Sequence(int size_) : size(size_) {
    seq = new note[size];
}

/* Copy constructor. */
inline Sequence::Sequence(const Sequence& s1) : 
    size(s1.length()), max_pitch(s1.max_pitch), min_pitch(s1.min_pitch)
{
    seq = new note[size];
    std::copy(s1.begin(), s1.end(), seq);
}

/* Returns the length of the sequence. */
inline int Sequence::length() const {
    return size;
}

/* Returns the i-th element of the sequence. The first element has
 * index 1.
 */
inline note Sequence::operator() (int i) {
    if ((i <= 0) || (i > size)) {
        throw IndexOutOfBounds();
    }
    return seq[i - 1];
}

/* Returns the i-th element of the sequence as a const. The first 
 * element has index 1.
 */
inline const note Sequence::operator() (int i) const {
    if ((i <= 0) || (i > size)) {
        throw IndexOutOfBounds();
    }
    return seq[i - 1];
}

inline int Sequence::durationClass(float duration) {
    if (duration < 30) {
        return 0;
    }
    float c = (log10(duration) - log_30) / (log_3840 - log_30) * 28;
    c = floor(c + 0.5);
    if (c > 28) { 
        c = 28;
    }
    return (int)c;
}

inline Sequence::~Sequence() {
    delete[] seq;
}

#endif
