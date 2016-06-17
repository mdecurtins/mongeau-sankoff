/* State.cpp
 * 
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "State.h"

#include <string>
#include <sstream>

using namespace std;

string State::print() const {
    stringstream ss;
    ss << (int)k << "|" << (int)s << "|" << (int)e_i << "|" << (int)editType() <<
        "|" << (int)editLength() << "|" << (int)editPos();
    return ss.str();
}
