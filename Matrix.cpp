/* Matrix.cpp
 * Generic two-dimensional array.
 * 
 * Code adapted from C++ FAQ Lite by Marshall Cline,
 * www.parashift.com/c++-faq-lite/
 */

#include "Matrix.h" 

#include <sstream>
#include <iostream>
#include <string>

using namespace std;

template<typename T>
string Matrix<T>::print() const {
    stringstream s;
    s.precision(4);
    for (unsigned int i = 0; i < nrows_; i++) {
        for (unsigned int j = 0; j < ncols_; j++) {
            s.width(6);
            s << right << operator()(i, j);
        }
        s << "\n";
    }
    return s.str();
}

template<> string Matrix<char>::print() const {
    stringstream s;
    s.precision(4);
    for (unsigned int i = 0; i < nrows_; i++) {
        for (unsigned int j = 0; j < ncols_; j++) {
            s.width(6);
            s << right << (int)operator()(i, j);
        }
        s << "\n";
    }
    return s.str();
}

template std::string Matrix<float>::print() const;
//template std::string Matrix<char>::print() const;
//template std::string Matrix<double>::print() const;
//template float Matrix<float>::operator() (unsigned, unsigned);
//template const float Matrix<float>::operator() (unsigned, unsigned) const;
template class Matrix<float>;
template class Matrix<char>;
//template class Matrix<float>;

//template float Matrix<float>::operator() (unsigned, unsigned);
//template const float Matrix<float>::operator() (unsigned, unsigned) const;
//template char Matrix<char>::operator() (unsigned, unsigned);
//template const char Matrix<char>::operator() (unsigned, unsigned) const;
