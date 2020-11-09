#ifndef CLAUSE
#define CLAUSE

#include<algorithm>
#include "globals.hh"

typedef vector<LINT> LitVector;
typedef vector<LINT>::iterator Literator;
typedef vector<LINT>::reverse_iterator RLiterator;

class Clause {
  //friend class ClauseRegistry;
  //friend class ClauseSet;
public:

  explicit Clause(const vector<LINT>& lits) : clits(lits) {}

  //Clause(int nlits, const long int lits[]) :
  //clits(lits, lits+nlits), weight(0) {
  //sort_lits();
  ////N
  //DBG(cout << "Creating clause: " << *this << endl;);
  //}

  virtual ~Clause() { clits.clear(); }

public:

  /* ULINT size() { return clits.size(); } */
  size_t size() { return clits.size(); }

  Literator begin() { return clits.begin(); }

  Literator end() { return clits.end(); }

protected:
  vector<LINT> clits;
};
    
#endif

