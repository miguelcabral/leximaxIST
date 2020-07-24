/* 
 * File:   ReadCNF.cc
 * Author: mikolas
 * 
 * Created on November 11, 2010, 1:19 PM
 */

#include "ReadCNF.hh"
#include <vector>
#include <assert.h>
#define _abs(x) (  ( (x) < 0 ) ? (-(x)): (x) )


ReadCNF::ReadCNF(gzFile& input_file)
: input_file(input_file), mxid(0)
{}

ReadCNF::~ReadCNF() {}

void ReadCNF::read_cnf_clause(StreamBuffer& in, vector<LINT>& lits) {
  lits.clear();
  for (;;){
    LINT parsed_lit = parseInt(in);
    if (parsed_lit == 0) break;
    LINT var = _abs(parsed_lit);
    if (var > mxid) mxid = var;
    lits.push_back(parsed_lit);
  }
}


void ReadCNF::read()
{
  StreamBuffer in(input_file);
  std::vector<LINT> literals;
  for (;;){
    skipWhitespace(in);
    if (*in == EOF)
      break;
    else if (*in == 'c' || *in == 'p')
      skipLine(in);
    else {
        literals.clear();
        read_cnf_clause(in, literals);
        clause_vector.push_back(clauses.create_clause(literals));
    }
  }
}
