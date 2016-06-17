/* MS.h
 * This class implements the generalized Mongeau-Sankoff algorithm for 
 * comparison of musical sequences. This implementation is largely based on 
 * 
 * Meek, C. J. and W. P. Birmingham (Chair) (2004). Modelling error in 
 * query-by-humming applications. Doctoral thesis, University of Michigan. 
 * 
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef MS_h
#define MS_h

#include "Matrix.h"
#include "Sequence.h"
#include "Result.h"
#include "Algorithm.h"
#include "global.h"

#include <limits>
#include <list>
#include <cmath>

// operations
const char Insertion = 1;
static const char Deletion = 2;
static const char Replacement = 3;
static const char StartAlignment = 4;
static const char Fragmentation = 20;
static const char Consolidation = 40;

class MS : public Algorithm {
    Alignment alignment;
    float k1, k2, k3;
    int MaxInterval; // |weight| = MaxInterval
    static float weight[]; // cost of a substitution by interval
    bool weights; // use weights or a distance metric for interval costs?
    bool ins_del; // allow insertions and deletions?
    int F, C; // 1 <= F,C <= 20
    float min_scal, max_scal;
    float steps; // scaling steps
    bool modulo12;
    bool smaller_distance;
    const float ninfinity;

    float ms(const Sequence& q, const Sequence& d, int m, int n, int t, 
             Matrix<char>& O, int *bi, int *bj);
    float sFragment(note b, const Sequence& q, int i, int k, int t);
    float sConsolid(note a, const Sequence& d, int j, int k, int t);
    std::list<char> traceback(const Matrix<char>& O, int end_i, int end_j, 
                               int *start_i, int *start_j);
    void swap(Matrix<char> **a, Matrix<char> **b);
    float min(float a, float b);
    note rmod(note a);
    int iabs(int a);

    float sDeletion(note b);
    float sInsertion(note a);
    float sReplacement(note a, note b, int t);
    float wInterval(note a, note b);

public:

    /*
    MS(Alignment a = AllToSome) :
        alignment(a), 
        k1(0.348), 
        k2(0.05),
        k3(0.348),
        MaxInterval(15),
        weights(false),
        ins_del(true),
        F(2),
        C(2),
        min_scal(1),
        max_scal(1),
        steps(4),
        ninfinity(-std::numeric_limits<float>::max())
    {};
    */

    MS(float k1_, float k2_, float k3_, int F_, int C_, bool ins_del_,
       Alignment a = AllToSome, bool mod12 = false, bool smaller_dist = false) :
        alignment(a),
        k1(k1_), 
        k2(k2_),
        k3(k3_),
        MaxInterval(15),
        weights(false),
        ins_del(ins_del_),
        F(F_),
        C(C_),
        min_scal(1),
        max_scal(1),
        steps(4),
        modulo12(mod12),
        smaller_distance(smaller_dist),
        ninfinity(-std::numeric_limits<float>::max())
    {};

    ~MS() {};

    Result compare(Sequence& q, Sequence& t);

    class NullLengthSequence {}; // Exception
};

/* Returns the score of a deletion, that is, skipping a symbol in
 * the document.
 * b  deleted note
 */
inline float MS::sDeletion(note b) {
    return (k2 - k3) * (float)b.duration; // k2 * b.duration - k3 * b.duration
}

/* Returns the score of an insertion, that is, skipping a symbol in
 * the query.
 * a  inserted note
 */
inline float MS::sInsertion(note a) {
    return (k2 - k3) * (float)a.duration; // k2 * a.duration - k3 * a.duration
}

/* Returns the score of a fragmentation, that is, replacing a note
 * in the document by two or more notes in the query.
 * b  document note
 * q  query
 * i  position of the last note of the fragmentation in the query
 * k  number of query notes in the fragmentation, k <= F
 */
inline float MS::sFragment(note b, const Sequence& q, int i, int k, int t) {
    float w_interval = 0;
    float w_length;
    float sum = 0;
    for (int l = i; l >= i - k + 1; l--) {
        w_interval += wInterval(b, rmod(q(l) + t));
        sum += q(l).duration;
    }
    w_length = fabs(sum - b.duration);
    sum += b.duration; // total length of the notes involved in the fragmentation
    return k2 * sum - w_interval - k1 * w_length;
}

/* Returns the score of a consolidation, that is, replacing two or more notes
 * in the document by a single note in the query.
 * a  query note
 * d  document
 * j  position of the last note of the consolidation in the document
 * k  number of document notes in the consolidation, k <= C
 */
inline float MS::sConsolid(note a, const Sequence& d, int j, int k, int t) {
    float w_interval = 0;
    float w_length;
    float sum = 0;
    for (int l = j; l >= j - k + 1; l--) {
        w_interval += wInterval(rmod(a + t), d(l));
        sum += d(l).duration;
    }
    w_length = fabs(sum - a.duration);
    sum += a.duration; // total length of the notes involved in the consolidation
    return k2 * sum - w_interval - k1 * w_length;
}

/* Returns the score of a simple substitution. */
inline float MS::sReplacement(note a, note b, int t) {
    return k2 * (a.duration + b.duration) - wInterval(rmod(a + t), b) - 
        k1 * fabs(a.duration - b.duration);
}

/* Returns the weight contribution of the interval between two given notes. */
inline float MS::wInterval(note a, note b) {
    int inter = iabs(a.pitch - b.pitch);
    float w;
    if (weights) {
        // use weights as in the original M-S article
        if (inter > MaxInterval) {
            inter = MaxInterval;
        }
        w = weight[inter];
    } else {
        // use a distance metric
        w = inter;
        if (modulo12 && smaller_distance && (w > 6)) {
            w = 12 - w;
        }
    }
    float minl = min((float)a.duration, (float)b.duration);
    return minl * w;
}

inline float MS::min(float a, float b) {
    if (b < a)
        return b;
    else
        return a;
}

/* Swaps the contents of two pointers.
 * a  (in-out) pointer to a Matrix
 * b  (in-out) pointer to a Matrix
 */ 
inline void MS::swap(Matrix<char> **a, Matrix<char> **b) {
    Matrix<char> *temp = *a;
    *a = *b;
    *b = temp;
}

// pre: a.pitch >= 0
inline note MS::rmod(note a) {
    if (modulo12 == false) {
        return a;
    }
    return note(a.pitch % 12, a.duration);
}

inline int MS::iabs(int a) {
    if (a < 0)
        return -a;
    else
        return a;
}

#endif
