/* Matrix.h
 * Generic two-dimensional array.
 * 
 * Code adapted from C++ FAQ Lite by Marshall Cline,
 * www.parashift.com/c++-faq-lite/
 */

#ifndef Matrix_h
#define Matrix_h

#include "global.h"

#include <string>
#include <memory>

template<typename T>
class Matrix {
public:
    Matrix(unsigned nrows, unsigned ncols);
    Matrix(unsigned nrows, unsigned ncols, T initializer);
    // Throws a BadSize object if either size is zero
    class BadSize { };
 
    ~Matrix();
    Matrix(const Matrix<T>& m);
    Matrix<T>& operator= (const Matrix<T>& m);
 
    // Access methods to get the (i,j) element:
    inline T&       operator() (unsigned i, unsigned j);
    inline const T& operator() (unsigned i, unsigned j) const;
    // These throw a BoundsViolation object if i or j is too big
    class BoundsViolation { };
    
    std::string print() const;

private:
    unsigned nrows_, ncols_;
    T* data_;
};

template<typename T>
inline Matrix<T>::Matrix(unsigned nrows, unsigned ncols)
    : nrows_ (nrows)
    , ncols_ (ncols)
{
    if (nrows == 0 || ncols == 0)
        throw BadSize();
    data_ = new T[nrows * ncols];
}

template<typename T>
inline Matrix<T>::Matrix(unsigned nrows, unsigned ncols, T initializer)
    : nrows_ (nrows)
    , ncols_ (ncols)
{
    if (nrows == 0 || ncols == 0)
        throw BadSize();
    data_ = new T[nrows * ncols];
    std::uninitialized_fill_n(data_, nrows_ * ncols_, initializer);
}

template<typename T>
inline T& Matrix<T>::operator() (unsigned row, unsigned col)
{
    if (row >= nrows_ || col >= ncols_)
        throw BoundsViolation();
    return data_[row*ncols_ + col];
}

template<typename T>
inline const T& Matrix<T>::operator() (unsigned row, unsigned col) const
{
    if (row >= nrows_ || col >= ncols_)
        throw BoundsViolation();
    return data_[row*ncols_ + col];
}

template<typename T>
inline Matrix<T>::~Matrix()
{
    delete[] data_;
}

#endif
