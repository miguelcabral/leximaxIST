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

#ifndef Enc_Totalizer_h
#define Enc_Totalizer_h

#include <Encodings.h>
#include <leximaxIST_Solver.h>
#include <vector>

namespace leximaxIST {

class Totalizer : public Encodings {

public:
  Totalizer(int strategy = _INCREMENTAL_NONE_) {
    blocking = lit_Undef;
    hasEncoding = false;
    joinMode = false;
    current_cardinality_rhs = -1; // -1 corresponds to an unitialized value
    incremental_strategy = strategy;

    n_clauses = 0;
    n_variables = 0;
  }
  ~Totalizer() {}

  void build(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs);
  void join(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs);
  void update(leximaxIST::Solver &solver, int64_t rhs, const std::vector<int> &lits, std::vector<int> &assumptions);
  void update(leximaxIST::Solver &solver, int64_t rhs) {
    std::vector<int> lits;
    std::vector<int> assumptions;
    update(solver, rhs, lits, assumptions);
  }
  void add(leximaxIST::Solver &solver, Totalizer &tot, int64_t rhs);

  bool hasCreatedEncoding() { return hasEncoding; }
  void setIncremental(int incremental) { incremental_strategy = incremental; }
  int getIncremental() { return incremental_strategy; }

  // void enableConstraintBlocker(Solver* S)
  // {
  //   if (incremental_strategy == _INCREMENTAL_BLOCKING_)
  //   {
  //     printf("Error: Blocking incremental strategy does not support the "
  //            "use of constraint blockers.\n");
  //     printf("s UNKNOWN\n");
  //     exit(_ERROR_);
  //   }
  //   assert(!hasEncoding);
  //   if (blocking == lit_Undef)
  //   {
  //     Lit p = mkLit(S->nVars(), false);
  //     newSATVariable(S);
  //     blocking = p;
  //   }
  // }
  // Lit getConstraintBlocker() { return blocking; } // returns 'lit_Undef' if
  // constraint blockers are not enabled

  int getNbClauses() { return n_clauses; }
  int getNbVariables() { return n_variables; }
  void resetCounters() {
    n_clauses = 0;
    n_variables = 0;
  }

  std::vector<int> &lits() { return ilits; }
  std::vector<int> &outputs() { return cardinality_outlits; }

protected:
  void encode(leximaxIST::Solver &solver, std::vector<int> &lits);
  void adder(leximaxIST::Solver &solver, std::vector<int> &left, std::vector<int> &right, std::vector<int> &output);
  void incremental(leximaxIST::Solver &solver, int64_t rhs);
  void toCNF(leximaxIST::Solver &solver, std::vector<int> &lits);

  std::vector<std::vector<int>> totalizerIterative_left;
  std::vector<std::vector<int>> totalizerIterative_right;
  std::vector<std::vector<int>> totalizerIterative_output;
  std::vector<int64_t> totalizerIterative_rhs;

  Lit blocking; // Controls the blocking literal for the incremental blocking.
  bool hasEncoding;

  // TEMP
  std::vector<int> ilits;
  std::vector<int> olits;

  std::vector<int> cardinality_inlits; // Stores the inputs of the cardinality
                               // constraint encoding for the totalizer encoding
  std::vector<int> cardinality_outlits; // Stores the outputs of the cardinality
                                // constraint encoding for incremental solving

  int incremental_strategy;
  int64_t current_cardinality_rhs;

  std::vector<int> disable_lits; // Contains a vector with a list of blocking literals.
  bool joinMode;

  int n_clauses;
  int n_variables;
};
} // namespace leximaxIST

#endif
