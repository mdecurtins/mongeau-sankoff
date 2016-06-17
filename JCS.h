/* JCS.h
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef JCS_h
#define JCS_h

#include "Sequence.h"
#include "Matrix.h"
#include "Algorithm.h"
#include "Result.h"
#include "State.h"
#include "ProbVector.h"
#include "gauss.h"
#include "global.h"

#include <vector>
#include <assert.h>

// operations
/*
const char Insertion = 1;
static const char Deletion = 2;
static const char Replacement = 3;
static const char StartAlignment = 4;
static const char Fragmentation = 20;
static const char Consolidation = 40;
*/

#define SAME 0
#define JOIN 1
#define ELAB 2

#define APROX_N_STATES 800

class JCS : public Algorithm {
    State* states;
    std::vector<int>* q;
    std::vector<int>* next_q;
    int nStates;
    static const int L = 2;// = M  do not just change this value, as for
                           //      increasing the allowed edit length
                           //      it's also necessary to add another nested
                           //      loop to the forward variable function and
                           //      change other things. 
    static const int nTrans = 12;
    static const int nScal = 9;
    int A, B, C, D, E;
    State end_state;
    static float edit_probabilities[3];
    static float duration_classes[29];
    float sigmaInitScaling, sigmaTransp, sigmaScaling, sigmaPitch, sigmaRhythm;
    Alignment alignment;
    ProbVector init_scaling_probabilities;
    ProbVector transp_probabilities;
    ProbVector scaling_probabilities;
    ProbVector pitch_error_probabilities;
    ProbVector rhythm_error_probabilities;
    
    void enumerateStates(int n);
    float forward(const Sequence& query, const Sequence& doc, int m, int n);
    float initial(int edit_type, int delta_s);
    float transition(int edit_type, int delta_k, int delta_s);
    float observation(const State s, int t, const Sequence& query,
                      const Sequence& doc, int m, int n);
    float PErrorProb(int delta);
    float RErrorProb(int delta);
    void normalize(float *probabilities, int n);
    int stateToi(const State s) const;
    bool isEndState(State state);
    void swapq();
    int min(int a, int b);
    int max(int a, int b);
    int rmod(int a, int b);
    int iabs(int a);
    //float gaussian_pdf(const float x, const float sigma);
    float gaussianP(const float x1, const float x2, const float sigma);
    float errorP(float x, const float sigma);
    std::string printArray(float *array, int length);
    bool belongs(std::vector<int>*, int);

public:
    JCS(float sInitScaling = 1.5, float sTransp = 0.28, 
        float sScaling = 0.263, float sPitch = 0.422, float sRhythm = 0.55, 
        float p_same = 0.95, float p_join = 0.03, Alignment a = Start) :
        states(NULL),
        q(NULL),
        next_q(NULL),
        sigmaInitScaling(sInitScaling),
        sigmaTransp(sTransp),
        sigmaScaling(sScaling),
        sigmaPitch(sPitch),
        sigmaRhythm(sRhythm),
        alignment(a),
        init_scaling_probabilities(17, -8),
        transp_probabilities(23, -11),
        scaling_probabilities(17, -8),
        pitch_error_probabilities(23, -11),
        rhythm_error_probabilities(65, -32)
    {
        assert((a == AllToSome) || (a == Start));
        E = nScal;
        D = nTrans * E;
        C = L * D;
        B = (L - 1) * C;
        A = 3 * B;
        assert(p_same + p_join <= 1);
        edit_probabilities[0] = p_same;
        edit_probabilities[1] = p_join;
        edit_probabilities[2] = 1 - p_same - p_join;
    };

    Result compare(Sequence& q, Sequence& t);
    ~JCS();
    class NullLengthSequence {}; // Exception
};

#endif
