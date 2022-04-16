/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2018, Ruben Martins, Vasco Manquinho, Ines Lynce
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

#include <Encoder.h>
#include <leximaxIST_printing.h>
#include <stdlib.h>
#include <string>
#include <cassert>

using namespace leximaxIST;

/************************************************************************************************
 //
 // Encoding of exactly-one constraints
 //
 ************************************************************************************************/
void Encoder::encodeAMO(leximaxIST::Solver &solver, std::vector<Lit> &lits) {

  switch (amo_encoding) {
  // Currently only the ladder encoding is used for AMO constraints.
  case _AMO_LADDER_:
    ladder.encode(solver, lits);
    break;

  default:
    print_error_msg("Invalid at-most-one encoding");
    exit(EXIT_FAILURE);
  }
}

/************************************************************************************************
 //
 // Encoding of cardinality constraints
 //
 ************************************************************************************************/
//
// Manages the encoding of cardinality encodings.
void Encoder::encodeCardinality(leximaxIST::Solver &solver, std::vector<Lit> &lits, int64_t rhs) {

  switch (cardinality_encoding) {
  case _CARD_TOTALIZER_:
    totalizer.build(solver, lits, rhs);
    if (totalizer.hasCreatedEncoding())
      totalizer.update(solver, rhs);
    break;

  case _CARD_MTOTALIZER_:
    mtotalizer.encode(solver, lits, rhs);
    break;

  case _CARD_CNETWORKS_:
    cnetworks.encode(solver, lits, rhs);
    break;

  default:
    print_error_msg("Invalid cardinality encoding");
    exit(EXIT_FAILURE);
  }
}

void Encoder::addCardinality(leximaxIST::Solver &solver, Encoder &enc, int64_t rhs) {
  if (cardinality_encoding == _CARD_TOTALIZER_ &&
      enc.cardinality_encoding == _CARD_TOTALIZER_) {
    totalizer.add(solver, enc.totalizer, rhs);
  } else {
    print_error_msg("Cardinality encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

// Manages the update of cardinality constraints.
void Encoder::updateCardinality(leximaxIST::Solver &solver, int64_t rhs) {

  switch (cardinality_encoding) {
  case _CARD_TOTALIZER_:
    totalizer.update(solver, rhs);
    break;

  case _CARD_MTOTALIZER_:
    mtotalizer.update(solver, rhs);
    break;

  case _CARD_CNETWORKS_:
    cnetworks.update(solver, rhs);
    break;

  default:
    print_error_msg("Invalid cardinality encoding");
    exit(EXIT_FAILURE);
  }
}

// Incremental methods for cardinality encodings:
//
// Manages the building of cardinality encodings.
// Currently is only used for incremental solving.
void Encoder::buildCardinality(leximaxIST::Solver &solver, const std::vector<Lit> &lits, int64_t rhs) {
  assert(incremental_strategy != _INCREMENTAL_NONE_);

  switch (cardinality_encoding) {
  case _CARD_TOTALIZER_:
    totalizer.build(solver, lits, rhs);
    break;

  default:
    print_error_msg("Cardinality encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

// Manages the incremental update of cardinality constraints.
void Encoder::incUpdateCardinality(leximaxIST::Solver &solver, const std::vector<Lit> &join, const std::vector<Lit> &lits,
                                   int64_t rhs, std::vector<Lit> &assumptions) {
  assert(incremental_strategy == _INCREMENTAL_ITERATIVE_ ||
         incremental_strategy == _INCREMENTAL_WEAKENING_);
  // Note: the assumption vector will be updated in this procedure

  switch (cardinality_encoding) {
  case _CARD_TOTALIZER_:
    if (join.size() > 0)
      totalizer.join(solver, join, rhs);

    assert(lits.size() > 0);
    totalizer.update(solver, rhs, lits, assumptions);
    break;

  default:
    print_error_msg("Cardinality encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

void Encoder::joinEncoding(leximaxIST::Solver &solver, const std::vector<Lit> &lits, int64_t rhs) {

  switch (cardinality_encoding) {
  case _CARD_TOTALIZER_:
    totalizer.join(solver, lits, rhs);
    break;

  default:
    print_error_msg("Cardinality encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

/************************************************************************************************
 //
 // Encoding of pseudo-Boolean constraints
 //
 ************************************************************************************************/
//
// Manages the encoding of PB encodings.
void Encoder::encodePB(leximaxIST::Solver &solver, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs,
                       uint64_t rhs) {
    
//   printf("c [Encoder:encodePB] encoding: %d\n", pb_encoding);

  switch (pb_encoding) {
  case _PB_SWC_:
    swc.encode(solver, lits, coeffs, rhs);
    break;

  case _PB_GTE_:
    gte.encode(solver, lits, coeffs, rhs);
    break;

  case _PB_ADDER_:
    adder.encode(solver, lits, coeffs, rhs);
    break;
    
//   case _PB_IGTE_:
//     igte.encode(solver, lits, coeffs, rhs);
//     break;
    /*
    case _PB_KP_:
    kp.encode(solver, lits, coeffs, rhs, _kp_sat4j_, _inc_); //inc
    break;

    case _PB_KP_MINISATP_:
    kp.encode(solver, lits, coeffs, rhs, 1); //_kp_minisatp_ (modo) -- full
    break;*/
    
    //TODO: Fazer as versoes incrementais do KP
  default:
    print_error_msg("Invalid PB encoding : " + std::to_string(pb_encoding));
    exit(EXIT_FAILURE);
  }
}

int Encoder::predictPB(leximaxIST::Solver &solver, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs,
                       uint64_t rhs) {

  switch (pb_encoding) {
  case _PB_SWC_:
    return -1;
    break;

  case _PB_GTE_:
    return gte.predict(solver, lits, coeffs, rhs);
    break;

  case _PB_ADDER_:
    return -1;
    break;
    
//   case _PB_IGTE_:
//     return igte.predict(solver, lits, coeffs, rhs);
//     break;

  default:
    print_error_msg("Invalid PB encoding : " + std::to_string(pb_encoding));
    exit(EXIT_FAILURE);
  }
}


// Manages the update of PB encodings.
void Encoder::updatePB(leximaxIST::Solver &solver, uint64_t rhs) {

  switch (pb_encoding) {
  case _PB_SWC_:
    swc.update(solver, rhs);
    break;

  case _PB_GTE_:
    gte.update(solver, rhs);
    break;

  case _PB_ADDER_:
    adder.update(solver, rhs);
    break;
        
//   case _PB_IGTE_:
//     igte.update(solver, rhs);
//     break;
    /*
  case _PB_KP_:
    kp.update(solver, rhs);
    break;

  case _PB_KP_MINISATP_:
    kp.update(solver, rhs);
    break;
*/
  default:
    print_error_msg("Invalid PB encoding : " + std::to_string(pb_encoding));
    exit(EXIT_FAILURE);
  }
}

/*
std::vector<Lit>& Encoder::getAssumptions(){
    std::vector<Lit> assumpts;
    if(pb_encoding == _PB_IGTE_)
        return igte.getAssumptions();
    
    if(pb_encoding == _PB_KP_)
        return kp.getAssumptions();
    
    assumpts.clear();
    return assumpts;
}*/


// Incremental methods for PB encodings:
//
// Manages the incremental encode of PB encodings.
void Encoder::incEncodePB(leximaxIST::Solver &solver, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs,
                          int64_t rhs, std::vector<Lit> &assumptions, int size) {
  assert(incremental_strategy == _INCREMENTAL_ITERATIVE_);
  
  // Note: the assumption vector will be updated in this procedure

  switch (pb_encoding) {
  case _PB_SWC_:
    swc.encode(solver, lits, coeffs, rhs, assumptions, size);
    break;

  default:
    print_error_msg("PB encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

// Manages the incremental update of PB encodings.
void Encoder::incUpdatePB(leximaxIST::Solver &solver, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs,
                          int64_t rhs, std::vector<Lit> &assumptions) {
  assert(incremental_strategy == _INCREMENTAL_ITERATIVE_);

  // Note: the assumption vector will be updated in this procedure

  switch (pb_encoding) {
  case _PB_SWC_:
    swc.update(solver, rhs, assumptions);
    swc.join(solver, lits, coeffs, assumptions);
    break;

  default:
    print_error_msg("PB encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

// Manages the incremental update of assumptions.
// Currently only used for the iterative encoding with SWC.
void Encoder::incUpdatePBAssumptions(leximaxIST::Solver &solver, std::vector<Lit> &assumptions) {
  assert(incremental_strategy == _INCREMENTAL_ITERATIVE_);

  switch (pb_encoding) {
  case _PB_SWC_:
    swc.updateAssumptions(solver, assumptions);
    break;

  default:
    print_error_msg("PB encoding does not support incrementality");
    exit(EXIT_FAILURE);
  }
}

std::vector<Lit> &Encoder::lits() {
  assert(cardinality_encoding == _CARD_TOTALIZER_ &&
         incremental_strategy == _INCREMENTAL_ITERATIVE_);

  return totalizer.lits();
}

std::vector<Lit> &Encoder::outputs() {
  assert(cardinality_encoding == _CARD_TOTALIZER_ &&
         incremental_strategy == _INCREMENTAL_ITERATIVE_);

  return totalizer.outputs();
}

/************************************************************************************************
 //
 // Other
 //
 ************************************************************************************************/
// Returns true if the cardinality encoding was built, false otherwise.
bool Encoder::hasCardEncoding() {

  if (cardinality_encoding == _CARD_TOTALIZER_)
    return totalizer.hasCreatedEncoding();
  else if (cardinality_encoding == _CARD_MTOTALIZER_)
    return mtotalizer.hasCreatedEncoding();
  else if (cardinality_encoding == _CARD_CNETWORKS_)
    return cnetworks.hasCreatedEncoding();

  return false;
}

// Returns true if the PB encoding was built, false otherwise.
bool Encoder::hasPBEncoding() {
  if (pb_encoding == _PB_SWC_)
    return swc.hasCreatedEncoding();
  else if (pb_encoding == _PB_GTE_)
    return gte.hasCreatedEncoding();
  /*else if (pb_encoding == _PB_KP_)
    return kp.hasCreatedEncoding();
  else if (pb_encoding == _PB_KP_MINISATP_)
    return kp.hasCreatedEncoding();*/

  return false;
}

// //AG
void Encoder::getEncodeSizes(int *nvar, int *nclauses, int *nrootvars){
  if (pb_encoding == _PB_GTE_)
    return gte.getEncodeSizes(nvar, nclauses, nrootvars);
  /*
  else if (pb_encoding == _PB_KP_)
    return kp.getEncodeSizes(nvar, nclauses, nrootvars);
  else if (pb_encoding == _PB_KP_MINISATP_)
    return kp.getEncodeSizes(nvar, nclauses, nrootvars);*/
}
