/* State.cpp
 * 
 * Copyright (C) 2006 Carlos G�mez
 * Universidad Sim�n Bol�var
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
