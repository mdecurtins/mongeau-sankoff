/* MS.cpp
 * Implementation of the generalized Mongeau-Sankoff algorithm for comparison 
 * of musical sequences. This implementation is largely based on
 * 
 * Meek, C. J. and W. P. Birmingham (Chair) (2004). Modelling error in 
 * query-by-humming applications. Doctoral thesis, University of Michigan. 
 * 
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "MS.h"

#include <iostream>
#include <list>
#include <assert.h>

float MS::weight[] = {0, 2.6, 0.9, 1, 0.2, 0.5, 1.8, 0.1, 1.3, 0.35, 2.2,
                      0.8, 2.5, 0, 3};

using namespace std;

/* Calculates the matching score of two sequences
 * using the Mongeau-Sankoff algorithm.
 * q  query
 * d  target document
 */
Result MS::compare(Sequence& q, Sequence& d) {
    int m, n;
    m = q.length();
    n = d.length();
    if ((m == 0) || (n == 0)) {
        throw NullLengthSequence();
    }
    float s;
    float max_s = ninfinity;
    Matrix<char> O_1(m + 1, n + 1); // stores the operation applied at each
    Matrix<char> O_2(m + 1, n + 1); //  position
    Matrix<char> *current_o = &O_1;
    Matrix<char> *max_o = &O_2;
    int i_max = 0;
    int j_max = 0;
    int min_trans, max_trans;
    if (modulo12) {
        min_trans = 0;
        max_trans = 11;
        q.mod12();
        d.mod12();
    } else {
        min_trans = d.min_pitch - q.max_pitch;
        max_trans = d.max_pitch - q.min_pitch;
    }
    float a = min_scal;
    float step1 = (1 - min_scal) / steps;
    float step2 = (max_scal - 1) / steps;
    float step = step1;
    do {
        Sequence q1(q);
        q1.scale(a);
        for (int t = min_trans; t <= max_trans; t++) {
            if (verb(2)) cout << "\nt = " << t << ", s = " << a << "\n";
            int i, j; // end position of the alignment
            s = ms(q1, d, m, n, t, *current_o, &i, &j);
            if (s > max_s) {
                max_s = s;
                swap(&max_o, &current_o);
                i_max = i;
                j_max = j;
            }
        }
        if (a == 1) {
            step = step2;
        }
        a += step;
    } while ((a <= max_scal) && (step != 0));
    int start_i, start_j;
    list<char> solution = traceback(*max_o, i_max, j_max, &start_i, &start_j);
    return Result(max_s, solution, start_i, start_j);
}

/* This function is the actual M-S algorithm. 
 * q  query, 1 <= q_i <= m
 * d  document (target), 1 <= d_j <= n
 * m  query length
 * n  document length
 * t  transposition that should be applied to the query
 * O  matrix to store the applied operations
 * bi (out) end position of the alignment (i)
 * bj (out) end position of the alignment (j)
 */
float MS::ms(const Sequence& q, const Sequence& d, int m, int n, int t, 
             Matrix<char>& O, int *bi, int *bj) {
    Matrix<float> G(m + 1, n + 1); // alignment matrix
    // initialize the alignment matrix according to the alignment type
    if ((alignment == Global) || (alignment == Start) || 
        (alignment == VarLenStart)) {
        for (int i = 0; i <= m; i++)
            for (int j = 0; j <= n; j++)
                G(i, j) = ninfinity;
        G(0, 0) = 0;
    } else if (alignment == AllToSome) {
        for (int j = 0; j <= n; j++)
            G(0, j) = 0;
        for (int i = 1; i <= m; i++)
            for (int j = 0; j <= n; j++)
                G(i, j) = ninfinity;
    } else if (alignment == Overlap) {
        for (int j = 0; j <= n; j++)
            G(0, j) = 0;
        for (int i = 1; i <= m; i++)
            G(i, 0) = 0;
        for (int i = 1; i <= m; i++)
            for (int j = 1; j <= n; j++)
                G(i, j) = ninfinity;
    } else if (alignment == Local) {
        for (int i = 0; i <= m; i++)
            for (int j = 0; j <= n; j++)
                G(i, j) = 0;
    }
    O(0, 0) = StartAlignment;
    if (verb(2)) cout << G.print() << "\n";
    // fill the alignment and operation matrices
    float s, max_s;
    char max_op;
    //   initial row
    for (int j = 1; j <= n; j++) {
        max_s = G(0, j - 1) + sDeletion(d(j));
        if (G(0, j) > max_s) {
            O(0, j) = StartAlignment;
        } else {
            G(0, j) = max_s;
            O(0, j) = Deletion;
        }
    }
    if (verb(2)) cout << G.print() << "\n";
    //   initial column
    for (int i = 1; i <= m; i++) {
        max_s = G(i - 1, 0) + sInsertion(q(i));
        if (G(i, 0) > max_s) {
            O(i, 0) = StartAlignment;
        } else {
            G(i, 0) = max_s;
            O(i, 0) = Insertion;
        }
    }
    if (verb(2)) cout << G.print() << "\n";
    //   all the other positions
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            // replacement
            max_s = G(i - 1, j - 1) + sReplacement(q(i), d(j), t);
            max_op = Replacement;
            if (verb(2)) cout << "max(" << max_s << ", ";
            if (ins_del) {
                // insertion
                s = G(i - 1, j) + sInsertion(q(i));
                if (verb(2)) cout << s << ", ";
                if (s > max_s) {
                    max_s = s;
                    max_op = Insertion;
                }
                // deletion
                s = G(i, j - 1) + sDeletion(d(j));
                if (verb(2)) cout << s << ", ";
                if (s > max_s) {
                    max_s = s;
                    max_op = Deletion;
                }
            }
            // fragmentation
            for (int k = 2; (k <= F) && (k <= i); k++) {
                s = G(i - k, j - 1) + sFragment(d(j), q, i, k, t);
                if (verb(2)) cout << s << ", ";
                if (s > max_s) {
                    max_s = s;
                    max_op = Fragmentation + k;
                }
            }
            // consolidation
            for (int k = 2; (k <= C) && (k <= j); k++) {
                s = G(i - 1, j - k) + sConsolid(q(i), d, j, k, t);
                if (verb(2)) cout << s << ", ";
                if (s > max_s) {
                    max_s = s;
                    max_op = Consolidation + k;
                }
            }
            if (verb(2)) cout << G(i, j) << ")\n";
            if (G(i, j) > max_s) {
                // start new alignment
                O(i, j) = StartAlignment;
            } else {
                G(i, j) = max_s;
                O(i, j) = max_op;
            }
            if (verb(2)) cout << G.print() << "\n";
            // TODO ver cual IOI se toma en cuenta
        }
    }
    // find the highest score at an end position
    int best_i, best_j;
    max_s = G(m, n);
    best_i = m;
    best_j = n;
    if (alignment != Global) {
        for (int j = 0; j < n; j++) { // South row
            if (G(m, j) > max_s) {
                max_s = G(m, j);
                best_i = m;
                best_j = j;
            }
        }
    }
    if ((alignment == VarLenStart) || (alignment == Overlap) || 
               (alignment == Local)) {
        for (int i = 0; i < m; i++) { // East column
            if (G(i, n) > max_s) {
                max_s = G(i, n);
                best_i = i;
                best_j = n;
            }
        }
    }
    if (alignment == Local) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (G(i, j) > max_s) {
                    max_s = G(i, j);
                    best_i = i;
                    best_j = j;
                }
            }
        }
    }
    *bi = best_i;
    *bj = best_j;
    return max_s;
}

/* Reconstructs the sequence of operations that produce the optimal 
 * alignment. 
 * O       operations matrix filled by the algorithm
 * end_i   end position of the alignment (i)
 * end_j   end position of the alignment (j)
 * start_i (out) start position of the alignment (i)
 * start_j (out) start position of the alignment (j)
 */
list<char> MS::traceback(const Matrix<char>& O, int end_i, int end_j, 
                          int *start_i, int *start_j) {
    list<char> solution;
    int i = end_i;
    int j = end_j;
    char edit = O(i, j);
    //cout << O.print() << "\n\n\n";
    while (edit != StartAlignment) {
        solution.push_front(edit);
        if (edit == Insertion) {
            i--;
        } else if (edit == Deletion) {
            j--;
        } else if (edit == Replacement) {
            i--;
            j--;
        } else if ((edit >= Fragmentation) && (edit < Consolidation)) {
            i -= edit - Fragmentation;
            j--;
        } else if ((edit >= Consolidation) && (edit < 60)) {
            j -= edit - Consolidation;
            i--;
        } else {
            assert(false);
        }
        edit = O(i, j);
    }
    *start_i = i;
    *start_j = j;
    return solution;
}
