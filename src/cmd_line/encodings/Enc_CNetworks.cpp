/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2017, Ruben Martins, Vasco Manquinho, Ines Lynce
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <Enc_CNetworks.h>
#include <cassert>
#include <cmath>

using namespace leximaxIST;

/*_________________________________________________________________________________________________
  |
  |  encode : (S : Solver *) (lits : std::vector<Lit>&) (rhs : int64_t) ->  [void]
  |
  |  Description:
  |
  |     Encodes that at most 'rhs' literals can be assigned value true.
  |     Uses the Cardinality Network encoding for translating the cardinality
  |     constraint into CNF.
  |
  |  For further details see:
  |    * R. Asín, R. Nieuwenhuis, A. Oliveras, E. Rodríguez-Carbonell:
  |      Cardinality Networks: a theoretical and empirical study.
  |      Constraints 16(2): 195-221, 2011
  |    * The code for the Cardinality Network encoding was based on the source
  |      code of npSolver by Peter Steinke and Norbert Manthey
  |      (http://tools.computational-logic.org/content/npSolver.php)
  |
  |  Pre-conditions:
  |    * Assumes that 'lits' is not empty.
  |    * Assumes that 'rhs' is larger or equal to 0.
  |
  |  Post-conditions:
  |    * 'S' is updated with the clauses that encode the cardinality constraint.
  |
  |________________________________________________________________________________________________@*/
void CNetworks::encode(leximaxIST::Solver &S, const std::vector<Lit> &lits, int64_t rhs) {

  assert(rhs >= 0);
  assert(lits.size() > 0);

  current_cardinality_rhs = rhs;

  cardinality_outlits.clear();
  std::vector<Lit> lits_copy (lits);

  if (rhs == 0) {
    for (int i = 0; i < lits.size(); i++)
      addUnitClause(S, -lits[i]);
    return;
  }

  std::vector<Lit> units;
  int n = lits_copy.size();
  // Find the smallest power of 2 that is larger than rhs.
  int64_t new_k = std::pow(2, std::floor(std::log2(rhs)) + 1);

  int64_t m = std::ceil((double)n / new_k) * new_k - n;

  // The size of the input variables must be a multiple of rhs.
  for (int i = 0; i < m; ++i) {
    //Lit p = mkLit(S.nVars(), false);
    //newSATVariable(S);
      Lit p = S.fresh();
    lits_copy.push_back(p);
    units.push_back(-p);
  }

  for (int i = 0; i < new_k; ++i) {
    //cardinality_outlits.push_back(mkLit(S.nVars(), false));
    //newSATVariable(S);
      cardinality_outlits.push_back(S.fresh());
  }

  // Enforce that the cardinality constraint can take at most k true elements.
  // We can add as unit clauses all values between k and new_k.
  for (int i = rhs; i < new_k; i++)
    units.push_back(-cardinality_outlits[i]);

  CN_encode(S, lits_copy, cardinality_outlits, new_k);

  for (int i = 0; i < units.size(); i++)
    addUnitClause(S, units[i]);

  hasEncoding = true;
}

/*_________________________________________________________________________________________________
  |
  |  update : (S : Solver *) (rhs : int64_t) ->  [void]
  |
  |  Description:
  |
  |     Updates the 'rhs' of an already existent cardinality encoding.
  |     This method allows for all learned clauses from previous iterations to
  |     be kept in the next iteration.
  |     This procedure is denoted by incremental strengthening.
  |
  |  For further details see:
  |    * R. Asín, R. Nieuwenhuis, A. Oliveras, E. Rodríguez-Carbonell:
  |      Cardinality Networks: a theoretical and empirical study.
  |      Constraints 16(2): 195-221, 2011
  |
  |  Pre-conditions:
  |    * Only one cardinality constraint is being used.
  |    * Assumes that 'current_cardinality_rhs' is larger than -1, i.e. assumes
  |      a cardinality constraint has already been encoded.
  |
  |  Post-conditions:
  |    * 'S' is updated with unit clauses that update the 'rhs' of the
  |      cardinality encoding.
  |    * 'current_cardinality_rhs' is updated.
  |
  |________________________________________________________________________________________________@*/
void CNetworks::update(leximaxIST::Solver &S, int64_t rhs) {

  assert(current_cardinality_rhs != -1);
  assert(cardinality_outlits.size() != 0 && rhs < cardinality_outlits.size());

  for (int i = rhs; i < current_cardinality_rhs; i++)
    addUnitClause(S, -cardinality_outlits[i]);

  current_cardinality_rhs = rhs;
}

/************************************************************************************************
//
// Auxiliary methods for the cardinality encoding
//
************************************************************************************************/

void CNetworks::CN_hmerge(leximaxIST::Solver &S, std::vector<Lit> &a_s, std::vector<Lit> &b_s,
                          std::vector<Lit> &c_s) {

  assert(a_s.size() == b_s.size());
  assert(a_s.size() * 2 == c_s.size());

  if (a_s.size() == 1) {
    Lit c1 = c_s[0];
    Lit c2 = c_s[1];

    Lit a = a_s[0];
    Lit b = b_s[0];

    addTernaryClause(S, -a, -b, c2);
    addBinaryClause(S, -a, c1);
    addBinaryClause(S, -b, c1);
  } else {

    std::vector<Lit> odd_a_s;
    std::vector<Lit> even_a_s;
    std::vector<Lit> odd_b_s;
    std::vector<Lit> even_b_s;

    for (int i = 0; i < a_s.size(); i = i + 2)
      odd_a_s.push_back(a_s[i]);

    for (int i = 1; i < a_s.size(); i = i + 2)
      even_a_s.push_back(a_s[i]);

    for (int i = 0; i < b_s.size(); i = i + 2)
      odd_b_s.push_back(b_s[i]);

    for (int i = 1; i < b_s.size(); i = i + 2)
      even_b_s.push_back(b_s[i]);

    std::vector<Lit> d_s;
    std::vector<Lit> e_s;

    d_s.push_back(c_s[0]);

    for (int i = 1; i < a_s.size(); ++i) {
      //d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
        d_s.push_back(S.fresh());
    }

    for (int i = 0; i < a_s.size() - 1; ++i) {
      //e_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
        e_s.push_back(S.fresh());
    }

    e_s.push_back(c_s[c_s.size() - 1]);

    for (int i = 1; i < a_s.size(); ++i) {
      addTernaryClause(S, -d_s[i], -e_s[i - 1], c_s[2 * i]);
      addBinaryClause(S, -d_s[i], c_s[2 * i - 1]);
      addBinaryClause(S, -e_s[i - 1], c_s[2 * i - 1]);
    }

    CN_hmerge(S, odd_a_s, odd_b_s, d_s);
    CN_hmerge(S, even_a_s, even_b_s, e_s);
  }
}

void CNetworks::CN_hsort(leximaxIST::Solver &S, std::vector<Lit> &a_s, std::vector<Lit> &c_s) {
  assert(a_s.size() == c_s.size());

  if (a_s.size() == 2) {
    assert(a_s[0] != a_s[1]);
    std::vector<Lit> a;
    std::vector<Lit> b;
    a.push_back(a_s[0]);
    b.push_back(a_s[1]);
    CN_hmerge(S, a, b, c_s);
  } else {

    std::vector<Lit> upper_a_s;
    std::vector<Lit> lower_a_s;
    std::vector<Lit> upper_d_s;
    std::vector<Lit> lower_d_s;

    for (int i = 0; i < (a_s.size() / 2); i++) {
      lower_a_s.push_back(a_s[i]);
      //upper_d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
      upper_d_s.push_back(S.fresh());
      //lower_d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
      lower_d_s.push_back(S.fresh());
    }

    for (int i = (a_s.size() / 2); i < a_s.size(); i++)
      upper_a_s.push_back(a_s[i]);

    CN_hsort(S, lower_a_s, lower_d_s);
    CN_hsort(S, upper_a_s, upper_d_s);
    CN_hmerge(S, lower_d_s, upper_d_s, c_s);
  }
}

void CNetworks::CN_smerge(leximaxIST::Solver &S, std::vector<Lit> &a_s, std::vector<Lit> &b_s,
                          std::vector<Lit> &c_s) {

  assert(a_s.size() == b_s.size());
  assert(a_s.size() + 1 == c_s.size());

  if (a_s.size() == 1) {
    Lit c1 = c_s[0];
    Lit c2 = c_s[1];

    Lit a = a_s[0];
    Lit b = b_s[0];

    addTernaryClause(S, -a, -b, c2);
    addBinaryClause(S, -a, c1);
    addBinaryClause(S, -b, c1);
  } else {

    std::vector<Lit> odd_a_s;
    std::vector<Lit> even_a_s;
    std::vector<Lit> odd_b_s;
    std::vector<Lit> even_b_s;

    for (int i = 0; i < a_s.size(); i = i + 2)
      odd_a_s.push_back(a_s[i]);

    for (int i = 1; i < a_s.size(); i = i + 2)
      even_a_s.push_back(a_s[i]);

    for (int i = 0; i < b_s.size(); i = i + 2)
      odd_b_s.push_back(b_s[i]);

    for (int i = 1; i < b_s.size(); i = i + 2)
      even_b_s.push_back(b_s[i]);

    std::vector<Lit> d_s;
    std::vector<Lit> e_s;

    d_s.push_back(c_s[0]);

    for (int i = 1; i < (a_s.size() / 2) + 1; ++i) {
      //d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
        d_s.push_back(S.fresh());
    }

    for (int i = 0; i < (a_s.size() / 2) + 1; ++i) {
      //e_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
        e_s.push_back(S.fresh());
    }

    for (int i = 1; i <= (a_s.size() / 2); ++i) {
      addTernaryClause(S, -d_s[i], -e_s[i - 1], c_s[2 * i]);
      addBinaryClause(S, -d_s[i], c_s[2 * i - 1]);
      addBinaryClause(S, -e_s[i - 1], c_s[2 * i - 1]);
    }

    CN_smerge(S, odd_a_s, odd_b_s, d_s);
    CN_smerge(S, even_a_s, even_b_s, e_s);
  }
}

void CNetworks::CN_encode(leximaxIST::Solver &S, std::vector<Lit> &a_s, std::vector<Lit> &c_s,
                          int64_t rhs) {
  assert(a_s.size() % rhs == 0);
  assert(c_s.size() == rhs);

  if (a_s.size() == rhs) {
    CN_hsort(S, a_s, c_s);
  } else {
    std::vector<Lit> upper_a_s;
    std::vector<Lit> lower_a_s;
    std::vector<Lit> upper_d_s;
    std::vector<Lit> lower_d_s;

    for (int i = 0; i < rhs; i++) {
      lower_a_s.push_back(a_s[i]);
      //lower_d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
      lower_d_s.push_back(S.fresh());
      //upper_d_s.push_back(mkLit(S.nVars(), false));
      //newSATVariable(S);
      upper_d_s.push_back(S.fresh());
    }

    for (int i = rhs; i < a_s.size(); i++) {
      upper_a_s.push_back(a_s[i]);
    }

    std::vector<Lit> next_c_s;
    for (int i = 0; i < c_s.size(); i++)
      next_c_s.push_back(c_s[i]);

    //next_c_s.push_back(mkLit(S.nVars(), false));
    //newSATVariable(S);
    next_c_s.push_back(S.fresh());

    CN_encode(S, lower_a_s, lower_d_s, rhs);
    CN_encode(S, upper_a_s, upper_d_s, rhs);
    CN_smerge(S, lower_d_s, upper_d_s, next_c_s);
  }
}
