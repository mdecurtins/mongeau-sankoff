/* Result.h
 * Search result, which consists of the score of a document as well as its 
 * filename, composer and title. 
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef Result_h
#define Result_h

#include "global.h"

#include <string>
#include <list>

class Result {
public:
    float score;
    std::string filename;
    int id;
    std::string composer;
    std::string title;
    std::list<char> solution;  // operations that produce the alignment
    int queryStart;            // start position of the alignment in the query
    int documentStart;         // start position of the alignment in the document

    Result(float s, std::list<char> sol, int i, int j) : 
        score(s), solution(sol), queryStart(i), documentStart(j) {};
  //Result(float s, std::string f, int m, std::string c = "", std::string t = "")
  //    : score(s), filename(f), id(m), composer(c), title(t) {};
    int operator< (const Result& r2) const {
        return this->score > r2.score;
    };
};

#endif
