/* State.h
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "global.h"

#include <assert.h>
#include <string>

class State {
public:
    char k;            // transposition
    char s;            // tempo scaling
    unsigned char e_i; // target position that corresponds to the edit state

private:
    unsigned char e_type;   // edit type
    unsigned char e_len;      // edit length
    unsigned char e_pos;    // edit position

    // lower and upper bounds for each parameter
    static const int l_edit_type = 0;
    static const int u_edit_type = 2;
    static const int l_edit_len = 2;
    static const int u_edit_len = 2;
    static const int l_edit_pos = 0;
    static const int u_edit_pos = 1;
    static const int l_k = -5;
    static const int u_k = 6;
    static const int l_s = -4;
    static const int u_s = 5; // 5 = end state

public:    
    State() {};
    State(char k_, char s_, unsigned char e_i_, unsigned char edit_type, 
          unsigned char edit_len, unsigned char edit_pos);
    unsigned char editType() const;
    unsigned char editLength() const;
    unsigned char editPos() const;
    //void editPos(unsigned char pos);
    void incEditPos();
    std::string print() const;
};

inline State::State(char k_, char s_, unsigned char e_i_, unsigned char edit_type,
                    unsigned char edit_len, unsigned char edit_pos)
    : k(k_),
      s(s_),
      e_i(e_i_),
      e_type(edit_type),
      e_len(edit_len), 
      e_pos(edit_pos)
{
    assert(k_ >= l_k && k_ <= u_k);
    assert(s_ >= l_s && s_ <= u_s);
    assert(edit_type <= u_edit_type);
    assert(edit_len >= l_edit_len && edit_len <= u_edit_len);
    assert(edit_pos <= u_edit_pos);
}

inline unsigned char State::editType() const {
    return e_type;
}

inline unsigned char State::editLength() const {
    return e_len;
}

inline unsigned char State::editPos() const {
    return e_pos;
}

//inline void State::editPos(unsigned char pos) {
//    assert(pos <= u_edit_pos);
//    e_pos = pos;
//}

inline void State::incEditPos() {
    assert(editPos() < u_edit_pos);
    e_pos++;
}
