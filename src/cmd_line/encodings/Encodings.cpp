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

#include <Encodings.h>
#include <cmath>
#include <cassert>

using namespace leximaxIST;

// Miguel Cabral - my implementation for leximaxIST
// this used to be implemented by the SAT solver, such as glucose or minisat
// False means I want the variable, true means I want the negation of the variable
Lit Encodings::mkLit(int var_id, bool sign = false)
{
    if (!sign) {
        Lit l (var_id);
        return l;
    }
    else {
        Lit l (-var_id);
        return l;
    }
}

// Creates an unit clause in the SAT solver
void Encodings::addUnitClause(leximaxIST::Solver &S, Lit a, Lit blocking) {
  assert(clause.size() == 0);
  assert(a != lit_Undef);
  assert(std::abs(a) < S.nVars());
  clause.push_back(a);
  if (blocking != lit_Undef)
    clause.push_back(blocking);
  S.add_hard_clause(clause);
  clause.clear();
}

// Creates a binary clause in the SAT solver
void Encodings::addBinaryClause(leximaxIST::Solver &S, Lit a, Lit b, Lit blocking) {
  assert(clause.size() == 0);
  assert(a != lit_Undef && b != lit_Undef);
  assert(std::abs(a) < S.nVars() && std::abs(b) < S.nVars());
  clause.push_back(a);
  clause.push_back(b);
  if (blocking != lit_Undef)
    clause.push_back(blocking);
  S.add_hard_clause(clause);
  clause.clear();
}

// Creates a ternary clause in the SAT solver
void Encodings::addTernaryClause(leximaxIST::Solver &S, Lit a, Lit b, Lit c, Lit blocking) {
  assert(clause.size() == 0);
  assert(a != lit_Undef && b != lit_Undef && c != lit_Undef);
  assert(std::abs(a) < S.nVars() && std::abs(b) < S.nVars() && std::abs(c) < S.nVars());
  clause.push_back(a);
  clause.push_back(b);
  clause.push_back(c);
  if (blocking != lit_Undef)
    clause.push_back(blocking);
  S.add_hard_clause(clause);
  clause.clear();
}

// Creates a quaternary clause in the SAT solver
void Encodings::addQuaternaryClause(leximaxIST::Solver &S, Lit a, Lit b, Lit c, Lit d,
                                    Lit blocking) {
  assert(clause.size() == 0);
  assert(a != lit_Undef && b != lit_Undef && c != lit_Undef && d != lit_Undef);
  assert(std::abs(a) < S.nVars() && std::abs(b) < S.nVars() && std::abs(c) < S.nVars() &&
         std::abs(d) < S.nVars());
  clause.push_back(a);
  clause.push_back(b);
  clause.push_back(c);
  clause.push_back(d);
  if (blocking != lit_Undef)
    clause.push_back(blocking);
  S.add_hard_clause(clause);
  clause.clear();
}
