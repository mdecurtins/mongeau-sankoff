/* JCS.cpp
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "JCS.h"

#include <iostream>
#include <list>
#include <string>
#include <sstream>

using namespace std;

float JCS::edit_probabilities[3] = {0.95, 0.03, 0.02};

float JCS::duration_classes[29] = {30, 37.5, 45, 52.5, 60, 75, 90, 105, 120, 
                                   150, 180, 210, 240, 300, 360, 420, 480, 600, 
                                   720, 840, 960, 1200, 1440, 1680, 1920, 2400, 
                                   2880, 3360, 3840};

/* Calculates the probability that a query matches a given
 * document, using the JCS model. 
 * q  query
 * d  target document
 */
Result JCS::compare(Sequence& q, Sequence& d) {
    int m, n;
    m = q.length();
    n = d.length();
    if ((m == 0) || (n == 0)) {
        throw NullLengthSequence();
    }
    if (m > 255) {
        warning("the query has more than 255 notes, so it won't be compared");
        return Result(0, list<char>(), 0, 0);
    }
    if (n > 255) {
        warning("the document has more than 255 notes, so it won't be compared");
        return Result(0, list<char>(), 0, 0);
    }
    if (verb(2)) cout << "quantized query: " << q.print() << "\n";
    if (verb(2)) cout << "quantized document: " << d.print() << "\n";
    nStates = n * 3 * (L - 1) * L * nTrans * nScal + 1; // + 1 because of the end state
    delete[] states;
    states = new State[nStates];
    enumerateStates(n);
    if (verb(2)) cout << "number of states: " << nStates << "\n";
    for (int i = -11; i <= 11; i++) {
        transp_probabilities(i) = errorP(i, sigmaTransp);
    }
    for (int i = -8; i <= 8; i++) {
        scaling_probabilities(i) = errorP(i, sigmaScaling);
        init_scaling_probabilities(i) = errorP(i, sigmaInitScaling);
    }
    for (int i = -11; i <= 11; i++) {
        pitch_error_probabilities(i) = errorP(i, sigmaPitch);
    }
    for (int i = -32; i <= 32; i++) {
        rhythm_error_probabilities(i) = errorP(i, sigmaRhythm);
    }
    transp_probabilities.normalize(sigmaTransp);
    scaling_probabilities.normalize(sigmaScaling);
    init_scaling_probabilities.normalize(sigmaInitScaling);
    pitch_error_probabilities.normalize(sigmaPitch);
    rhythm_error_probabilities.normalize(sigmaRhythm);
    if (verb(2)) cout << "Transposition probabilities:\n"
                      << transp_probabilities.print()
                      << "\nScaling probabilities:\n"
                      << scaling_probabilities.print()
                      << "\nInitial scaling probabilities:\n"
                      << init_scaling_probabilities.print()
                      << "\nPitch error probabilities:\n"
                      << pitch_error_probabilities.print()
                      << "\nRhythm error probabilities:\n"
                      << rhythm_error_probabilities.print()
                      << "\nEdit probabilities:\n"
                      << printArray(edit_probabilities, 3)
                      << "\n\n";
    if (verb(2)) {
        for (int i = -5; i <= 17; i++)
            cout << i << " % 12" << " = " << rmod(i, 12) << "  ";
        cout << "\n";
    }
    float p = forward(q, d, m, n);
    return Result(p, list<char>(), 0, 0);
}

void JCS::enumerateStates(int n) {
    for (int i = 0; i < n; i++) {
        for (int edit_type = 0; edit_type < 3; edit_type++) {
            for (int edit_len = 2; edit_len <= L; edit_len++) {
                for (int k = -5; k <= 6; k++) {
                    for (int s = -4; s <= 4; s++) {
                        if (edit_type == ELAB) {
                            for (int edit_pos = 0; edit_pos < edit_len; edit_pos++) {
                                State state(k, s, i, edit_type, edit_len, edit_pos);
                                states[stateToi(state)] = state;
                            }
                        } else {
                            State state(k, s, i, edit_type, edit_len, 0);
                            states[stateToi(state)] = state;
                        }
                    }
                }
            }
        }
    }
    end_state = State(6, 5, n - 1, 2, 2, 1); // s = 5 indicates the end state
    assert(stateToi(end_state) == nStates - 1);
    states[stateToi(end_state)] = end_state;
}

float JCS::forward(const Sequence& query, const Sequence& doc, int m, int n)
{
    typedef vector<int>::iterator Iterator;
    Matrix<float> alpha(nStates, m + 1, -1);
    delete q;
    delete next_q;
    q = new vector<int>();
    next_q = new vector<int>();
    q->reserve(APROX_N_STATES);
    next_q->reserve(APROX_N_STATES);

    // initialization
    //if (verb(3)) cout << "initial\tobservation\n";
    int max_start_position;
    if (alignment == Start) {
        max_start_position = 1;
    } else { // if (alignment == AllToSome)
        max_start_position = n;
    }
    for (int edit_type = 0; edit_type < 3; edit_type++) {
        for (int k = -5; k <= 6; k++) {
            for (int s = -4; s <= 4; s++) {
                for (int i = 0; i < max_start_position; i++) {
                    State s_ini(k, s, i, edit_type, 2, 0);
                    int pos = stateToi(s_ini);
                    q->push_back(pos);
                    if (verb(3)) cout << "pos = " << pos << "\n";
                    alpha(pos, 0) = initial(edit_type, s) * 
                        observation(s_ini, 0, query, doc, m, n);
                    //alpha(pos, 0) = (float)0.1;
                    //cout << pos << "\n";
                    //cout << "value=" << (float)alpha(pos, 0) << "\n";
                    //assert((float)alpha(pos, 0) == (float)0.1);
                    //if (verb(3)) cout << initial(edit_type, s) << "\t" << 
                    //                 observation(s_ini, 0, query, doc, m, n) << "\n";
                    //if (verb(3)) cout << (initial(edit_type, s) *
                    //                 observation(s_ini, 0, query, doc, m, n)) << "\n";
                }
            }
        }
        //if (verb(2)) cout << alpha.print() << "\n\n";
    }

    if (verb(3)) cout << alpha.print() << "\n\n";

    alpha(nStates - 1, m) = 0; // initialize the probability of the end state

    // induction
    for (int t = 0; t < m; t++) {
        //float sum_join = 0;
        //float sum_elab = 0;
        //float sum_same = 0;
        for (Iterator iter = q->begin(); iter != q->end(); iter++) {
            int pos = *iter;
            State state = states[pos];
            //if (state.editType() == JOIN) {
            //    //cout << "plot_join " <<  alpha(pos, t) << "\n";
            //    sum_join += alpha(pos, t);
            //} else if (state.editType() == ELAB && state.editPos() == 0) {
            //    //cout << "plot_elab " <<  alpha(pos, t) << "\n";
            //    sum_elab += alpha(pos, t);
            //} else if (state.editType() == SAME) {
            //    sum_same += alpha(pos, t);
            //    //    cout << "plot_same " <<  alpha(pos, t) << "\n";
            //}
            assert(alpha(pos, t) >= 0);
            if (alpha(pos, t) == 0) {
                //cout << "a" << pos << "\n";
                // discard past-the-end join & elab states
                continue;
            }
            if (isEndState(state)) {
                assert(false);
            } else if ((state.editType() == ELAB) && (state.editPos() < state.editLength() - 1)) {
                //cout << "c" << pos << "\n";
                assert(t < m - 1); // if L>2 this condition must be rewrited
                state.incEditPos();
                int pos1 = stateToi(state);
                if (alpha(pos1, t + 1) == -1) {
                    next_q->push_back(pos1);
                    alpha(pos1, t + 1) = alpha(pos, t);
                } else {
                    assert(false);
                }
            } else {
                // general case
                //cout << "d" << pos << "\n";
                int i = state.e_i + 1;
                if (state.editType() == JOIN) {
                    i += state.editLength() - 1;
                }
                assert(i <= n);
                if ((t == m - 1) || (i == n)) {
                    // the only succesor is the end state
                    alpha(nStates - 1, m) += alpha(pos, t);
                } else {
                    int kp = state.k; // previous k
                    int sp = state.s;
                    for (int edit_type = 0; edit_type < 3; edit_type++) {
                        for (int delta_k = -5; delta_k <= 6; delta_k++) {
                            for (int s = max(sp - 4, -4); s <= min(sp + 4, 4); s++) {
                                int k = rmod(kp + 5 + delta_k, 12) - 5;
                                int delta_s = s - sp;
                                State suc(k, s, i, edit_type, 2, 0);
                                int pos1 = stateToi(suc);
                                if (alpha(pos1, t + 1) == -1) {
                                    next_q->push_back(pos1);
                                    alpha(pos1, t + 1) = alpha(pos, t) * 
                                        transition(edit_type, delta_k, delta_s);
                                } else {
                                    alpha(pos1, t + 1) += alpha(pos, t) * 
                                        transition(edit_type, delta_k, delta_s);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (verb(3)) cout << alpha.print() << "\n\n";
        for (Iterator iter = next_q->begin(); iter != next_q->end(); iter++) {
            int pos = *iter;
            State state = states[pos];
            alpha(pos, t + 1) *= observation(state, t + 1, query, doc, m, n);
        }
        swapq();
        //cout << sum_join << " " << sum_elab << " " << sum_same <<"\n";
    }

    // termination
    assert(q->empty());
    return alpha(nStates - 1, m);
}

inline float JCS::initial(int edit_type, int s) {
    float p = 1/(float)12;
    return edit_probabilities[edit_type] * p * 
        init_scaling_probabilities(s);
}

inline float JCS::transition(int edit_type, int delta_k, int delta_s) {
    return edit_probabilities[edit_type] * transp_probabilities(delta_k) *
        scaling_probabilities(delta_s);
}

float JCS::observation(const State state, int t, const Sequence& query,
                       const Sequence& doc, int m, int n) {
    if (isEndState(state)) {
        assert(t == m);
        return 1;
    }
    int edit_type = state.editType();
    int k = state.k;
    int s = state.s;
    int i = state.e_i;
    int pos = state.editPos();
    int delta_ioi;
    float pitch_error_p;
    if (edit_type == SAME) {
        int delta_pitch = query(t + 1).pitch - rmod(doc(i + 1).pitch + k, 12);
        //int delta_pitch = query(t + 1).pitch - doc(i + 1).pitch - k;
        if (delta_pitch > 6) {
            delta_pitch = 12 - delta_pitch;
        }
        delta_ioi = query(t + 1).duration - doc(i + 1).duration - s;
        pitch_error_p = PErrorProb(delta_pitch);
        //cout << query(t + 1).pitch << " " << doc(i + 1).pitch << " " << k << " " << pitch_error_p << "\n";
        //cout << "plotsame " << (float)(pitch_error_p * RErrorProb(delta_ioi)) << "\n";
    } else if (edit_type == JOIN) {
        int l = state.editLength();
        if (i + l > n) { // (i + l - 1 > n - 1)
            return 0;
        }
        pitch_error_p = 1;
        for (int j = i; j < i + l; j++) {
            //cout << query(t + 1).pitch << " " << doc(j + 1).pitch << " " << k << "\n";
            int delta_pitch = query(t + 1).pitch - rmod(doc(j + 1).pitch + k, 12);
            //int delta_pitch = query(t + 1).pitch - doc(j + 1).pitch - k;
            if (delta_pitch > 6) {
                delta_pitch = 12 - delta_pitch;
            }
            pitch_error_p *= PErrorProb(delta_pitch);
        }
        float sum = 0;
        for (int j = i; j < i + l; j++) {
            sum += duration_classes[doc(j + 1).duration];
        }
        int log_sum = Sequence::durationClass(sum);
        delta_ioi = query(t + 1).duration - log_sum - s;
        //cout << "plotjoin " <<  (float)(pitch_error_p * RErrorProb(delta_ioi)) << "\n";
    } else if ((edit_type == ELAB) && (pos == 0)) {
        int l = state.editLength();
        if (t + l > m) {
            return 0;
        }
        pitch_error_p = 1;
        for (int j = t; j < t + l; j++) {
            //cout << doc(i + 1).pitch << " " << query(j + 1).pitch << " " << k << "\n";
            int delta_pitch = query(j + 1).pitch - rmod(doc(i + 1).pitch + k, 12);
            //int delta_pitch = query(j + 1).pitch - doc(i + 1).pitch - k;
            if (delta_pitch > 6) {
                delta_pitch = 12 - delta_pitch;
            }
            pitch_error_p *= PErrorProb(delta_pitch);
        }
        //cout << "elab ";
        float sum = 0;
        for (int j = t; j < t + l; j++) {
            sum += duration_classes[query(j + 1).duration];
            //cout << query(j + 1).duration << " ";
        }
        int log_sum = Sequence::durationClass(sum);
        delta_ioi = log_sum - doc(i + 1).duration - s;
        //cout << doc(i + 1).duration << " sum: " << sum << " log_sum:" << log_sum << " s: " << s << " " << RErrorProb(delta_ioi) << "\n";
        //cout << "plotelab " << doc(i + 1).duration << " " << sum  << " " << s << "\n";
        //cout << "plotelab " <<  (float)(pitch_error_p * RErrorProb(delta_ioi)) << "\n";
    } else { // if ((edit_type == ELAB) && (pos != 0))
        return 1;
    }
    return pitch_error_p * RErrorProb(delta_ioi);
}

inline float JCS::PErrorProb(int delta) {
    return pitch_error_probabilities(delta);
}

inline float JCS::RErrorProb(int delta) {
    if ((delta >= -32) && (delta <= 32)) {
        return rhythm_error_probabilities(delta);
    } else {
        return rhythm_error_probabilities(32);
    }
}

/* Converts a state to a 6-dimensional integer representation, where
 * the dimensions are (in this order): i, edit type, (l or m), j, K and S.
 */
inline int JCS::stateToi(const State s) const {
    //return A * s.e_i + B * s.editType() + C * s.editLength() + D * s.editPos()
    //    + E * (s.k + 5) + s.s + 4;
    return A * s.e_i + B * s.editType() + D * s.editPos()
        + E * (s.k + 5) + s.s + 4;
}

inline bool JCS::isEndState(const State state) {
    return state.s == 5;
}

inline void JCS::swapq() {
    delete q;
    q = next_q;
    next_q = new vector<int>();
    next_q->reserve(APROX_N_STATES);
}

inline int JCS::min(int a, int b) {
    if (b < a)
        return b;
    else
        return a;
}

inline int JCS::max(int a, int b) {
    if (b > a)
        return b;
    else
        return a;
}

// pre: b > 0
inline int JCS::rmod(int a, int b) {
    int m = iabs(a) % b;
    if ((a < 0) && (m != 0)) {
        return b - m;
    } else {
        return m;
    }
}

inline int JCS::iabs(int a) {
    if (a < 0)
        return -a;
    else
        return a;
}

/* 
 * Adapted from the GNU Scientific Library.
 * Copyright (C) 1996-2000 James Theiler, Brian Gough
 */
/*
float JCS::gaussian_pdf(const float x, const float sigma) {
  float u = x / fabsf(sigma);
  float p = (1 / (sqrtf(2 * M_PI) * fabsf(sigma))) * expf(-u * u / 2);
  return p;
}
*/

//f(x) = 1/(sqrt(2 pi) sigma) e^-((x - mu)^2/(2 sigma^2))

/*
float JCS::gaussian_pdf(const float x, const float sigma) {
    assert(sigma > 0);
    float p = 1 / (sqrt(2 * M_PI) * sigma) * exp(-(x * x / (2 * sigma * sigma)));
    return p;
}
*/

float JCS::gaussianP(const float x1, const float x2, const float sigma) {
    return gsl_cdf_gaussian_P(x2, sigma) - gsl_cdf_gaussian_P(x1, sigma);
}

float JCS::errorP(float x, const float sigma) {
    if (x > 0) {
        x = -x;
    }
    return gaussianP(x - 0.5, x + 0.5, sigma);
}

string JCS::printArray(float *array, int length) {
    stringstream s;
    for (int i = 0; i < length; i++) {
        s.width(6);
        s << right << array[i];
    }
    return s.str();
}

JCS::~JCS() {
    delete[] states;
    delete q;
    delete next_q;
}

/*
bool JCS::belongs(vector<int>* q, int a) {
    typedef vector<int>::iterator Iterator;
    for (Iterator iter = q->begin(); iter != q->end(); iter++) {
        if (a == *iter) {
            cout << (*iter) << "\n";
            return true;
        }
    }
    //cout << "\n\n";
    return false;
}
*/
