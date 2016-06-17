/* test.cpp
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "JCS.h"
#include "global.h"

#include <iostream>

using namespace std;

void enumerateStates();
void printState(State);
int stateToi(const State s);
int v_level;

int A, B, C, D, E;

int main() {
    int nScal = 9;
    int nTrans = 12;
    int L = 2;
    E = nScal;
    D = nTrans * E;
    C = L * D;
    B = (L - 1) * C;
    A = 3 * B;
    cout << "A = " << A << ", B = " << B << ", C = " << C << ", D = " << D 
         << ", E = " << E << "\n";
    enumerateStates();
    //cout << "Testing the State class...\n";
    cout << "duration classes\n";
    for (int d1 = 30; d1 < 3840; d1 *= 2) {
        int d2 = d1 * 2;
        float delta = (d2 - d1) / (float)4;
        for (float e = d1; e < d2; e += delta) {
            cout << e << " " << Sequence::durationClass(e) << ",\n";
        }
    }
    cout << 3840 << " " << Sequence::durationClass(3840) << "\n";
    return 0;
}

void enumerateStates() {
    for (int i = 0; i < 2; i++) {
        for (int edit_type = 0; edit_type < 3; edit_type++) {
            for (int edit_len = 2; edit_len <= 2; edit_len++) {
                for (int k = -5; k <= 6; k++) {
                    for (int s = -4; s <= 4; s++) {
                        if (edit_type == ELAB) {
                            for (int edit_pos = 0; edit_pos < edit_len; edit_pos++) {
                                cout << "k=" << k << " s=" << s << " e_i=" 
                                     << i << " e_type=" << edit_type << " e_len=" 
                                     << edit_len << " e_pos=" << edit_pos;
                                State state(k, s, i, edit_type, edit_len, edit_pos);
                                //states[stateToi(state)] = state;
                                //printState(state);
                                cout << "  " << stateToi(state);
                                cout << "  " << state.print() << "\n";
                            }
                        } else {
                            cout << "k=" << k << " s=" << s << " e_i=" 
                                 << i << " e_type=" << edit_type << " e_len=" 
                                 << edit_len;
                            State state(k, s, i, edit_type, edit_len, 0);
                            cout << "  " << stateToi(state);
                            cout << "  " << state.print() << "\n";
                            //states[stateToi(state)] = state;
                        }
                    }
                }
            }
        }
    }
    State end_state = State(6, 5, 1, 2, 2, 1); // s = 5 indicates the end state
    //states[stateToi(end_state)] = end_state;
    printState(end_state);
    cout << " " << stateToi(end_state) << "\n";
}


/* Converts a state to a 6-dimensional integer representation, where
 * the dimensions are (in this order): i, edit type, (l or m), j, K and S.
 */
int stateToi(const State s) {
    return A * s.e_i + B * s.editType() + /*C * s.editLength() +*/ D * s.editPos()
        + E * (s.k + 5) + s.s + 4;
}

void printState(State s) {
    cout << "k=" << (int)s.k << " s=" << (int)s.s << " e_i=" << (int)s.e_i << " e_type=" 
         << (int)s.editType() << " e_len=" << (int)s.editLength() << " e_pos=" 
         << (int)s.editPos();
}

void warning(char*) {}
