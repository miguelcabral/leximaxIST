/*!
 * \author Vasco Manquinho - vmm@sat.inesc-id.pt
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

#ifndef FormulaPB_h
#define FormulaPB_h

#include <leximaxIST_types.h>
#include <vector>
#include <string>
#include <map>
#include <cmath>

namespace leximaxIST {
    
typedef std::map<int, std::string> indexMap;
    
// Cardinality constraint of the form atMostK
class Card {

public:
  Card(const std::vector<Lit> &lits, int64_t rhs, bool sign = false) {
      _lits = lits;
      _rhs = rhs;
    if (sign) {
      int s = 0;
      for (int i = 0; i < _lits.size(); i++) {
        s += 1;
        _lits[i] = -_lits[i];
      }
      _rhs = s - _rhs;
    }
//     print();
  }

  Card() { _rhs = 0; }
  ~Card() {}

  void print() {
    printf("Card: ");

    for (int i = 0; i < _lits.size(); i++) {
      if (sign(_lits[i]))
        printf("~");
      printf("%d ", std::abs(_lits[i]) + 1);
    }
    printf(" <= %d\n", (int)_rhs);
  }
  
  
    void my_print(indexMap indexToName, bool original_vars = true) {
    // Assume _sign == false...
    printf("Card: ");

    for (int i = 0; i < _lits.size(); i++) {
      if (sign(_lits[i]))
        printf("~");
      if(!original_vars)
        printf("X%d ", std::abs(_lits[i]) + 1);
      else
          if(indexToName.find(std::abs(_lits[i])) != indexToName.end())
            printf("%s ", indexToName.at(std::abs(_lits[i])).c_str());
          else
            printf("X%d ", std::abs(_lits[i]) + 1);
    }
    printf(" <= %d\n", (int)_rhs);
  }

  std::vector<Lit> _lits;
  int64_t _rhs;
};

// PB constraint. The constraint sign is encoded in the structure.
class PB {

public:
    PB(std::vector<Lit> &lits, std::vector<uint64_t> &coeffs, int64_t rhs, bool s = false) {
        _lits = lits;
        _coeffs = coeffs;
        _rhs = rhs;
        _sign = s;
        for(int i = 0; i< coeffs.size(); i++) ub += coeffs[i];

        //     for(int i = 0; i< coeffs.size(); i++) printf("%lu\n", coeffs[i]);
        //     printf("%d\n" % ((s) ? 1 : 0));
        //     printf("rhs: %lu\n", rhs);
    }

  PB() {
    _rhs = 0;
    _sign = false;
    ub = 0;
  }
  ~PB() {}

  void addProduct(Lit l, int64_t c) {
    if (c >= 0) {
      _coeffs.push_back(c);
      _lits.push_back(l);
      ub += c;
    } else {
      _coeffs.push_back(-c);
      _lits.push_back(-l);
      _rhs += -c;
      ub += -c;
    }
  }

  void addRHS(int64_t rhs) { _rhs += rhs; }

  void changeSign() {
    int s = 0;
    for (int i = 0; i < _coeffs.size(); i++) {
      s += _coeffs[i];
      _lits[i] = -(_lits[i]);
    }
    _rhs = s - _rhs;
    _sign = !(_sign);
  }

  bool isClause() {
    // Assume _sign == false...
    bool sign = _sign;
    if (_sign)
      changeSign();
    if (_rhs != 1) {
      if (_sign != sign)
        changeSign();
      return false;
    }
    for (int i = 0; i < _coeffs.size(); i++) {
      if (_coeffs[i] != 1) {
        if (_sign != sign)
          changeSign();
        return false;
      }
    }
    return true;
  }

  bool isCardinality() {
    // Assume _sign == false...
    bool sign = _sign;
    if (_sign)
      changeSign();
    for (int i = 0; i < _coeffs.size(); i++) {
      if (_coeffs[i] != 1) {
        if (_sign != sign)
          changeSign();
        return false;
      }
    }
    return true;
  }

  void print() {
    // Assume _sign == false...
    if (isClause())
      printf("Clause: ");
    else if (isCardinality())
      printf("Card: ");
    else
      printf("PB: ");

    for (int i = 0; i < _coeffs.size(); i++) {
      printf("%d ", (int)_coeffs[i]);
      if (sign(_lits[i]))
        printf("~");
      printf("%d ", std::abs(_lits[i]) + 1);
    }
    if(!_sign)
        printf(" >= %d\n", (int)_rhs);
    else
        printf(" <= %d\n", (int)_rhs);
  }

  void my_print(indexMap indexToName, bool original_vars = true) {
    // Assume _sign == false...
    if (isClause())
      printf("c\tClause:\t");
    else if (isCardinality())
      printf("c\tCard:\t");
    else
      printf("c\tPB:\t");

    for (int i = 0; i < _coeffs.size(); i++) {
      printf("%d ", (int)_coeffs[i]);
      if (sign(_lits[i]))
        printf("~");
      if(!original_vars)
        printf("y%d ", std::abs(_lits[i]) + 1);
      else
        printf("%s ", indexToName.at(std::abs(_lits[i])).c_str());
    }
    
    
    if(!_sign)
        printf(" >= %d\n", (int)_rhs);
    else
        printf(" <= %d\n", (int)_rhs);
  }
  
  
  uint64_t getUB(){ return ub;}
  
  std::vector<uint64_t> _coeffs;
  std::vector<Lit> _lits;
  int64_t _rhs;
  bool _sign; // atLeast: false; atMost: true
  uint64_t ub;
};

class PBObjFunction {

    public:
    PBObjFunction(const std::vector<Lit> &lits, const std::vector<uint64_t> &coeffs, int64_t c = 0) {
        _lits = lits;
        _coeffs = coeffs;
        _const = c;
        ub = 0;
        lb = 0;
        for(int i = 0; i < coeffs.size(); i++){
            if(coeffs[i] > 0) ub += coeffs[i];
            else lb += coeffs[i];
        }
    }

    PBObjFunction() { _const = 0; ub = 0;}
    ~PBObjFunction() {}

    void addProduct(Lit l, int64_t c) {
        if (c >= 0) {
        _coeffs.push_back(c);
        _lits.push_back(l);
        } else {
        _coeffs.push_back(-c);
        _lits.push_back(-l);
        _const += c;
        }
//         printf("update _const: %ld\n", _const);
    }

    std::vector<uint64_t> _coeffs;
    std::vector<Lit> _lits;
    int64_t _const;
    uint64_t ub;
    uint64_t lb;
    
    uint64_t getUB(){ return ub; }
    uint64_t getLB(){ return lb; }

    void my_print(indexMap indexToName, bool original_vars = true, int maxsize=20) {
        // Assume _sign == false...
        printf("c\tmin (%d)\t", _coeffs.size());
        if(maxsize < 0)
            maxsize = _coeffs.size();
        

        for (int i = 0; i < maxsize && i < _coeffs.size(); i++) {
            printf("%lu ", _coeffs[i]);
            if (sign(_lits[i]))
                printf("~");
            if(!original_vars)
                printf("y%d ", std::abs(_lits[i]) + 1);
            else
                if(indexToName.find(std::abs(_lits[i])) != indexToName.end())
                    printf("%s ", indexToName.at(std::abs(_lits[i])).c_str());
                else
                    printf("Y%d ", std::abs(_lits[i]) + 1);
        }
        if(_coeffs.size() > maxsize)
            printf("... (%d lits)", _coeffs.size());
        printf("\n");
        
        double tweight = 0;
        for (int i = 0; i < _coeffs.size(); i++)
            tweight += _coeffs[i];
        printf("c max obj value: %.0f\n", tweight);
        printf("c _const: %ld\n", _const);
    }
    
    };

} // namespace leximaxIST

#endif
