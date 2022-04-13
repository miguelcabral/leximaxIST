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

#ifndef Enc_MTotalizer_h
#define Enc_MTotalizer_h

#include <Encodings.h>
#include <leximaxIST_Solver.h>
#include <leximaxIST_types.h>


namespace leximaxIST {

class MTotalizer : public Encodings {

public:
  MTotalizer() {
    h0 = lit_Undef;
    modulo = -1;
    current_cardinality_rhs = -1; // -1 corresponds to an unitialized value.
  }
  ~MTotalizer() {}

  void encode(leximaxIST::Solver &S, const std::vector<Lit> &lits, int64_t rhs);
  void update(leximaxIST::Solver &S, int64_t rhs);
  void setModulo(int m) { modulo = m; }

  int getModulo() { return modulo; }
  bool hasCreatedEncoding() { return hasEncoding; }

protected:
  // Auxiliary methods for the cardinality encoding:
  //
  void toCNF(leximaxIST::Solver &S, int mod, std::vector<Lit> &ublits, std::vector<Lit> &lwlits,
             int64_t rhs);
  void adder(leximaxIST::Solver &S, int mod, std::vector<Lit> &upper, std::vector<Lit> &lower,
             std::vector<Lit> &lupper, std::vector<Lit> &llower, std::vector<Lit> &rupper,
             std::vector<Lit> &rlower);
  void encode_output(leximaxIST::Solver &S, int64_t rhs);

  Lit h0;     // Temporary literal for the construction of the encoding.
  int modulo; // Stores the modulo value for the encoding.

  // Stores temporary inputs for the construction of the encoding.
  std::vector<Lit> cardinality_inlits;
  // Stores the outputs of the cardinality encoding for incremental solving.
  std::vector<Lit> cardinality_upoutlits;
  std::vector<Lit> cardinality_lwoutlits;

  // Stores the current value of the rhs of the cardinality constraint.
  int64_t current_cardinality_rhs;
};
} // namespace openwbo

#endif
