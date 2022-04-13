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

#ifndef Enc_SWC_h
#define Enc_SWC_h

#include <Encodings.h>
#include <leximaxIST_Solver.h>

namespace leximaxIST {

class SWC : public Encodings {

public:
  SWC() {
    current_pb_rhs = -1; // -1 corresponds to an unitialized value
    previous_lit_blocking = lit_Undef;
    current_lit_blocking = lit_Undef;
    nb_clauses = 0;
    nb_variables = 0;
  }
  ~SWC() {}

  // Encode constraint.
  void encode(leximaxIST::Solver &S, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs);
  void encode(leximaxIST::Solver &S, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs,
              std::vector<Lit> &assumptions, int size);
  // Update constraint.
  void update(leximaxIST::Solver &S, uint64_t rhs);
  void update(leximaxIST::Solver &S, uint64_t rhs, std::vector<Lit> &assumptions);

  // Update assumptions.
  void updateAssumptions(leximaxIST::Solver &S, std::vector<Lit> &assumptions) {
    assumptions.push_back(-current_lit_blocking);

    for (int i = 0; i < unit_lits.size(); i++)
      assumptions.push_back(-unit_lits[i]);
  }

  // Join encodings.
  void join(leximaxIST::Solver &S, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs,
            std::vector<Lit> &assumptions);

  // Returns true if the encoding was built, otherwise returns false;
  bool hasCreatedEncoding() { return hasEncoding; }

protected:
  std::vector<Lit> pb_outlits;    // Stores the outputs of the pseudo-Boolean constraint
                          // encoding for incremental solving.
  int64_t current_pb_rhs; // Stores the current value of the rhs of the
                          // pseudo-Boolean constraint.

  Lit previous_lit_blocking; // Blocking literal.
  Lit current_lit_blocking;  // Previous blocking literal.

  // Stores unit lits. Used for lits that have a coeff larger than rhs.
  std::vector<Lit> unit_lits;
  std::vector<uint64_t> unit_coeffs;

  // Stores the matrix with the auxiliary variables.
  std::vector<std::vector<Lit>> seq_auxiliary_inc;

  // Temporary copy of lits and coeffs for incremental approach.
  std::vector<Lit> lits_inc;
  std::vector<uint64_t> coeffs_inc;

  // Number of variables and clauses for statistics.
  int nb_variables;
  int nb_clauses;
};
} // namespace openwbo

#endif
