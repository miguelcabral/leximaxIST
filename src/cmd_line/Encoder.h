
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

#ifndef Encoder_h
#define Encoder_h

#include <leximaxIST_types.h>
#include <leximaxIST_Solver.h>

// Encodings
#include <Enc_CNetworks.h>
#include <Enc_GTE.h>
// #include "Enc_IGTE.h"
#include <Enc_Ladder.h>
#include <Enc_MTotalizer.h>
#include <Enc_SWC.h>
#include <Enc_Totalizer.h>
#include <Enc_Adder.h>
#include <Enc_KPA.h>

#include <vector>

namespace leximaxIST {

//=================================================================================================
class Encoder {

public:
  Encoder(int incremental = _INCREMENTAL_NONE_,
          int cardinality = _CARD_TOTALIZER_, int amo = _AMO_LADDER_,
          int pb = _PB_SWC_, double eps = 1, int blockingMode = 0) {
    pb_encoding = pb;
    amo_encoding = amo;
    incremental_strategy = incremental;
    cardinality_encoding = cardinality;
    totalizer.setIncremental(incremental);
    epsilon = eps;
//     igte.setApproxRatio(eps);
    blockingMode = blockingMode;
  }

  ~Encoder() {}

  // TEMP
  std::vector<int> &lits();
  std::vector<int> &outputs();

  // At-most-one encodings:
  //
  // Encode exactly-one constraint into CNF.
  void encodeAMO(leximaxIST::Solver &solver, const std::vector<int> &lits);

  // Cardinality encodings:
  //
  // Encode cardinality constraint into CNF.
  void encodeCardinality(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs);

  // Update the rhs of an already existent cardinality constraint
  void updateCardinality(leximaxIST::Solver &solver, int64_t rhs);

  // Incremental cardinality encodings:
  //
  // Build a cardinality constraint that can count up to 'rhs'.
  // No restriction is made on the value of 'rhs'.
  // buildCardinality + updateCardinality is equivalent to encodeCardinality.
  // Useful for incremental encodings.
  void buildCardinality(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs);

  // Incremental update for cardinality constraints;
  void incUpdateCardinality(leximaxIST::Solver &solver, const std::vector<int> &join, const std::vector<int> &lits,
                            int64_t rhs, const std::vector<int> &assumptions);
  void incUpdateCardinality(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs,
                            const std::vector<int> &assumptions) {

    std::vector<int> empty;
    incUpdateCardinality(S, empty, lits, rhs, assumptions);
  }

  // Add two disjoint cardinality constraints
  void addCardinality(leximaxIST::Solver &solver, Encoder &enc, int64_t rhs);

  // PB encodings:
  //
  // Encode pseudo-Boolean constraint into CNF.
  void encodePB(leximaxIST::Solver &solver, std::vector<int> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs);
  // Update the rhs of an already existent pseudo-Boolean constraint.
  void updatePB(leximaxIST::Solver &solver, uint64_t rhs);
  // Predicts the number of clauses needed for the encoding
  int predictPB(leximaxIST::Solver &solver, const std::vector<int> &lits, const std::vector<uint64_t> &coeffs, uint64_t rhs);

  // Incremental PB encodings:
  //
  // Incremental PB encoding.
  void incEncodePB(leximaxIST::Solver &solver, const std::vector<int> &lits, const std::vector<uint64_t> &coeffs,
                   int64_t rhs, std::vector<int> &assumptions, int size);

  // Incremental update of PB encodings.
  void incUpdatePB(leximaxIST::Solver &solver, const std::vector<int> &lits, const std::vector<uint64_t> &coeffs,
                   int64_t rhs, std::vector<int> &assumptions);

  // Incremental update of assumptions.
  void incUpdatePBAssumptions(leximaxIST::Solver &solver, std::vector<int> &assumptions);

  // Incremental construction of the totalizer encoding.
  // Joins a set of new literals, x_1 + ... + x_i, to an existing encoding of
  // the type
  // y_1 + ... + y_j <= k. It also updates 'k' to 'rhs'.
  void joinEncoding(leximaxIST::Solver &solver, const std::vector<int> &lits, int64_t rhs);

  // Other:
  //
  // Returns true if an encoding has been built, false otherwise.
  bool hasCardEncoding();
  bool hasPBEncoding();

  // Controls the type of encoding to be used:
  //
  void setCardEncoding(int enc) { cardinality_encoding = enc; }
  int getCardEncoding() { return cardinality_encoding; }

  void setPBEncoding(int enc) { pb_encoding = enc; }
  int getPBEncoding() { return pb_encoding; }

  void setAMOEncoding(int enc) { amo_encoding = enc; }
  int getAMOEncoding() { return amo_encoding; }
  
  void setApproxRatio(leximaxIST::Solver &solver, double eps) {
      epsilon = eps; if(pb_encoding == _PB_KP_) kp.setApproxRatio(S, eps);
  }

//   void setBlockingMode(int bm) { blockingMode = bm; igte.setBlockingMode(bm); }
  

//   void setNameToIndex(std::map<std::string, int> &n2id){if(pb_encoding == _PB_KP_ || pb_encoding == _PB_KP_MINISATP_) kp.setNameToIndex(n2id); printf("SET NAMETOINDEX (size: %d)\n", n2id.size());}
    
    
  double getApproxRatio() { return epsilon; }

  // Controls the modulo value that is used in the modulo totalizer encoding.
  //
  void setModulo(int m) { mtotalizer.setModulo(m); }
  int getModulo() { return mtotalizer.getModulo(); }

  // Sets the incremental strategy for the totalizer encoding.
  //
  void setIncremental(int incremental) {
    incremental_strategy = incremental;
    totalizer.setIncremental(incremental);
  }
  
  std::vector<Lit>& getAssumptions();
  std::vector<Lit> assumptions;
  
  void getEncodeSizes(int *nvar, int *nclauses, int *nrootvars); //AG
  void kpa_fixed_vars(const std::set<Lit>& fv){kp.fixed_vars(fv);}
  
protected:
  int incremental_strategy;
  int cardinality_encoding;
  int pb_encoding;
  int amo_encoding;
  double epsilon;
  int blockingMode;

  // At-most-one encodings
  Ladder ladder;

  // Cardinality encodings
  CNetworks cnetworks;
  MTotalizer mtotalizer;
  Totalizer totalizer;
  Adder adder;

  // PB encodings
  SWC swc;
  GTE gte;
//   IGTE igte;
  KPA kp;
};
} // namespace openwbo

#endif
