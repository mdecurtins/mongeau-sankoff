/* MSResult.h
 * Mongeau-Sankoff search result, which adds information to a result particular
 * to M-S, such as the number of operations of each type that was applied. 
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#ifndef MSResult_h
#define MSResult_h

#include "Result.h"
#include "global.h"

#include <string>
#include <list>

class MSResult : public Result {

public:
    int replacements;
    int insertions;
    int deletions;
    int fragmentations;
    int consolidations;
    
    MSResult(const MSResult&);

    MSResult(float s, std::list<char> sol, int i, int j, int repl, int ins,
             int del, int frag, int cons) :
        Result(s, sol, i, j),
        replacements(repl),
        insertions(ins),
        deletions(del),
        fragmentations(frag),
        consolidations(cons)
        {} ;

  //MSResult(float s, std::string filename_, int id_, std::string composer_ = "",
  //         std::string title_ = "")
  //    : Result(s, filename_, id_, composer_, title_) {};
};

#endif
