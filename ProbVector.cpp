/* ProbVector.cpp
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "ProbVector.h" 
#include "gauss.h" 

#include <sstream>
#include <iostream>
#include <string>

using namespace std;

string ProbVector::print() const {
    stringstream s;
    s.precision(3);
    for (unsigned int i = 0; i < length_; i++) {
        s.width(10);
        s << right << data_[i];
    }
    s << "\n";
    return s.str();
}

void ProbVector::normalize(float sigma) {
    double sum = 1 - 2 * gsl_cdf_gaussian_P(lbound_ - 0.5, sigma);
    double debug_sum = 0;
    for (unsigned int i = 0; i < length_; i++) {
        data_[i] /= sum;
        debug_sum += data_[i];
    }
    if (verb(2)) cout << "sum(pnorm(x)) = " << debug_sum << "\n";
}
