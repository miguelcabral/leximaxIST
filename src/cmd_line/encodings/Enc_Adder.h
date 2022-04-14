/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2018, Ruben Martins, Vasco Manquinho, Ines Lynce
 * PBLib,    Copyright (c) 2012-2013  Peter Steinke
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

#ifndef Enc_Adder_h
#define Enc_Adder_h

#include <Encodings.h>
#include <leximaxIST_types.h>
#include <leximaxIST_Solver.h>
#include <map>
#include <utility>
#include <vector>
#include <queue>

namespace leximaxIST {
class Adder : public Encodings {

public:
  Adder() {
    hasEncoding = false;
  }
  ~Adder() {}

  // Encode constraint.
  void encode(leximaxIST::Solver &S, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs);

  // Update constraint.
  void update(leximaxIST::Solver &S, uint64_t rhs);

  // Returns true if the encoding was built, otherwise returns false;
  bool hasCreatedEncoding() { return hasEncoding; }

  void encodeInc(leximaxIST::Solver &S, std::vector<Lit> &lits, std::vector<uint64_t> &coeffs, uint64_t rhs, std::vector<Lit> &assumptions);
  void updateInc(leximaxIST::Solver &S, uint64_t rhs, std::vector<Lit>& assumptions);


protected:

  std::vector<Lit> _output;
  std::vector<Lit> clause;
  std::vector<std::queue<Lit> > _buckets;

  void FA_extra ( leximaxIST::Solver &S, Lit xc, Lit xs, Lit a, Lit b, Lit c );
  Lit FA_carry ( leximaxIST::Solver &S, Lit a, Lit b, Lit c );
  Lit FA_sum ( leximaxIST::Solver &S, Lit a, Lit b, Lit c );
  Lit HA_carry ( leximaxIST::Solver &S, Lit a, Lit b);
  Lit HA_sum ( leximaxIST::Solver &S, Lit a, Lit b );
  void adderTree (leximaxIST::Solver &S, std::vector< std::queue< Lit > > & buckets, std::vector< Lit >& result );
  void lessThanOrEqual (leximaxIST::Solver &S, std::vector< Lit > & xs, std::vector< uint64_t > & ys);
  void numToBits ( std::vector<uint64_t> & bits, uint64_t n, uint64_t number );
  uint64_t ld64(const uint64_t x);

  void lessThanOrEqualInc (leximaxIST::Solver &S, std::vector< Lit > & xs, std::vector< uint64_t > & ys, std::vector<Lit>& assumptions);


	#define wbsplit(half,wL,wR, ws,bs, wsL,bsL, wsR,bsR) \
  wsL.clear(); bsL.clear(); wsR.clear(); bsR.clear(); \
  int ii = 0; \
  int wsSizeHalf = ws.size()/2; \
  for(; ii < wsSizeHalf; ii++) { \
    wsL.push_back(ws[ii]); \
    bsL.push_back(bs[ii]); \
    wL += ws[ii]; \
  } \
  for(; ii < ws.size(); ii++) { \
    wsR.push_back(ws[ii]); \
    bsR.push_back(bs[ii]); \
    wR += ws[ii]; \
  }

	void genWarnersFull(Lit& a, Lit& b, Lit& c, Lit& carry, Lit& sum, int comp,
		       leximaxIST::Solver &S, std::vector<Lit>& lits);

	void genWarnersHalf(Lit& a, Lit& b, Lit& carry, Lit& sum, int comp,
		       leximaxIST::Solver &S, std::vector<Lit>& lits);

	void genWarners(std::vector<uint64_t>& weights, std::vector<Lit>& blockings,
		uint64_t max, int k,
		int comp, leximaxIST::Solver &S, const Lit zero,
		std::vector<Lit>& lits, std::vector<Lit>& linkingVar);

	void genWarners0(std::vector<uint64_t>& weights, std::vector<Lit>& blockings,
		 uint64_t max,uint64_t k, int comp, leximaxIST::Solver &S,
		  std::vector<Lit>& lits, std::vector<Lit>& linkingVar);

	void lessthan(std::vector<Lit>& linking, uint64_t k, std::vector<uint64_t>& cc, leximaxIST::Solver &S, std::vector<Lit>& lits);

	std::vector<uint64_t> cc;
	std::vector<Lit> linkingVar;

	void wbSort(std::vector<uint64_t>& weights, std::vector<Lit>& blockings,
	    std::vector<uint64_t>& sweights, std::vector<Lit>& sblockings) {
  sweights.clear(); sblockings.clear();
  for(int i = 0; i < weights.size(); i++) {
    sweights.push_back(weights[i]);
    sblockings.push_back(blockings[i]);
  }
}

// koshi 20140121
void wbFilter(uint64_t UB, leximaxIST::Solver &S,std::vector<Lit>& lits,
	      std::vector<uint64_t>& weights, std::vector<Lit>& blockings,
	      std::vector<uint64_t>& sweights, std::vector<Lit>& sblockings) {
  sweights.clear(); sblockings.clear();

  for(int i = 0; i < weights.size(); i++) {
    if (weights[i] < UB) {
      sweights.push_back(weights[i]);
      sblockings.push_back(blockings[i]);
    } else {
      lits.clear();
      lits.push_back(-blockings[i]);
      Clause c (lits);
      S.add_hard_clause(c);
    }
  }
}


};
}

#endif
