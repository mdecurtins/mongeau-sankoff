/* ProbVector.h
 * 
 * Code adapted from C++ FAQ Lite by Marshall Cline,
 * www.parashift.com/c++-faq-lite/
 */

#ifndef ProbVector_h
#define ProbVector_h

#include "global.h"

#include <string>

class ProbVector {
public:
    ProbVector(unsigned int length, int lbound = 0);
    class BadSize { }; // Exception
 
    ~ProbVector();
    ProbVector(const ProbVector& v);
    ProbVector& operator= (const ProbVector& v);
 
    inline float& operator() (int index);
    inline const float& operator() (int index) const;
    class BoundsViolation { };
    
    void normalize(float sigma);
    std::string print() const;

private:
    unsigned int length_;
    int lbound_;
    float *data_;
};

inline ProbVector::ProbVector(unsigned int length, int lbound)
    : length_ (length)
    , lbound_ (lbound)
{
    if (length == 0)
        throw BadSize();
    data_ = new float[length];
}

inline float& ProbVector::operator() (int index)
{
    if ((index < lbound_) || ((unsigned)(index - lbound_) >= length_))
        throw BoundsViolation();
    return data_[index - lbound_];
}

inline const float& ProbVector::operator() (int index) const
{
    if ((index < lbound_) || ((unsigned)(index - lbound_) >= length_))
        throw BoundsViolation();
    return data_[index - lbound_];
}

inline ProbVector::~ProbVector()
{
    delete[] data_;
}

#endif
