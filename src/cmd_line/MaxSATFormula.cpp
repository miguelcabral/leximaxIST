/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2015, Ruben Martins, Vasco Manquinho, Ines Lynce
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

#include <iostream>
#include <MaxSATFormula.h>
#include <cassert>

using namespace leximaxIST;
/*
MaxSATFormula *MaxSATFormula::copyMaxSATFormula() {
  assert(format == _FORMAT_MAXSAT_);

  MaxSATFormula *copymx = new MaxSATFormula();
  copymx->setInitialVars(nVars());

  for (int i = 0; i < nVars(); i++)
    copymx->newVar();

  for (int i = 0; i < nSoft(); i++)
    copymx->addSoftClause(getSoftClause(i).weight, getSoftClause(i).clause);

  for (int i = 0; i < nHard(); i++)
    copymx->addHardClause(getHardClause(i).clause);

  copymx->setProblemType(getProblemType());
  copymx->updateSumWeights(getSumWeights());
  copymx->setMaximumWeight(getMaximumWeight());
  copymx->setHardWeight(getHardWeight());

  return copymx;
}
*/
// Adds a new hard clause to the hard clause database.
void MaxSATFormula::addHardClause(const std::vector<Lit> &lits) {
  hard_clauses.push_back(lits);
  n_hard++;
}

// Adds a new soft clause to the hard clause database.
/*
void MaxSATFormula::addSoftClause(uint64_t weight, const std::vector<Lit> &lits) {
  std::vector<Lit> vars;
  Lit assump = lit_Undef;
  Soft sc (lits, weight, assump, vars);
  soft_clauses.push_back(sc);
  n_soft++;
}
*/
// Adds a new soft clause to the hard clause database with predefined relaxation
// variables.
/*
void MaxSATFormula::addSoftClause(uint64_t weight, const std::vector<Lit> &lits,
                                  const std::vector<Lit> &vars) {
  Lit assump = lit_Undef;
  Soft sc (lits, weight, assump, vars);
  soft_clauses.push_back(sc);
  n_soft++;
}
*/
int MaxSATFormula::nInitialVars() const {
  return n_initial_vars;
} // Returns the number of variables in the working MaxSAT formula.

void MaxSATFormula::setInitialVars(int vars) { n_initial_vars = vars; }

int MaxSATFormula::nVars() const {
  return n_vars;
} // Returns the number of variables in the working MaxSAT formula.
/*
int MaxSATFormula::nSoft() const {
  return n_soft;
} // Returns the number of soft clauses in the working MaxSAT formula.
*/
int MaxSATFormula::nHard() const {
  return n_hard;
} // Returns the number of hard clauses in the working MaxSAT formula.

void MaxSATFormula::newVar(int v) {
  if(v == -1) n_vars++;
  else if(v > n_vars) n_vars = v;
} // Increases the number of variables in the working MaxSAT formula.

// Makes a new literal to be used in the working MaxSAT formula.
/*
Lit MaxSATFormula::newLiteral(bool sign) {
    Lit p = sign ? -nVars() : nVars();
    newVar();
    return p;
}*/
/*
void MaxSATFormula::setProblemType(int type) {
  problem_type = type;
} // Sets the problem type.

int MaxSATFormula::getProblemType() {
  return problem_type; // Return the problem type.
}
*/
// 'ubCost' is initialized to the sum of weights of the soft clauses.
void MaxSATFormula::updateSumWeights(uint64_t weight) {
  if (weight != hard_weight)
    sum_soft_weight += weight;
}

// The initial 'currentWeight' corresponds to the maximum weight of the soft
// clauses.
void MaxSATFormula::setMaximumWeight(uint64_t weight) {
  if (weight > max_soft_weight && weight != hard_weight)
    max_soft_weight = weight;
}

uint64_t MaxSATFormula::getMaximumWeight() { return max_soft_weight; }

void MaxSATFormula::setHardWeight(uint64_t weight) {
  hard_weight = weight;
} // Sets the weight of hard clauses.

Soft &MaxSATFormula::getSoftClause(int pos) {
  assert(pos < nSoft());
  return soft_clauses[pos];
}

Hard &MaxSATFormula::getHardClause(int pos) {
  assert(pos < nHard());
  return hard_clauses[pos];
}

void MaxSATFormula::addPBConstraint(PB &p) {

  // Add constraint to formula data structure.
  if (p.isClause()) {
    addHardClause(p._lits);
  } else if (p.isCardinality()) {
    if (!p._sign) {
      p.changeSign();
    }
    Card c (p._lits, p._rhs);
    cardinality_constraints.push_back(c);

  } else {
    if (!p._sign) {
      p.changeSign();
    }
    pb_constraints.push_back(p);
  }
}

int MaxSATFormula::newVarName(char *varName) {
  int id = varID(varName);
  if (id == var_Undef) {
    id = nVars() + 1;
    newVar();
    std::string s(varName);
    std::pair<std::string, int> nv(s, id);
    std::pair<int, std::string> ni(id, s);
    _nameToIndex.insert(nv);
    _indexToName.insert(ni);
  }
  return id;
}

int MaxSATFormula::varID(char *varName) {
  std::string s(varName);

  nameMap::const_iterator iter = _nameToIndex.find(s);
  if (iter != _nameToIndex.end()) {
    return iter->second;
  }
  return var_Undef;
}
/*
void MaxSATFormula::convertPBtoMaxSAT() {
// void MaxSATFormula::convertPBtoMaxSAT(Solver * S = NULL) { //AG
    
//   printf("MaxSATFormula::convertPBtoMaxSAT n_objf: %d\n", n_objf);
  assert(objective_functions != NULL);
  std::vector<Lit> unit_soft(1);

  if(n_objf == 1){
    // Convert objective function to soft clauses
    for (int i = 0; i < objective_functions[0]->_lits.size(); i++) {
        assert(objective_functions[0]->_coeffs[i] > 0);
        unit_soft[0] = ~objective_functions[0]->_lits[i];
        addSoftClause(objective_functions[0]->_coeffs[i], unit_soft);

        // Updates the maximum weight of soft clauses.
        setMaximumWeight(objective_functions[0]->_coeffs[i]);
        // Updates the sum of the weights of soft clauses.
        updateSumWeights(objective_functions[0]->_coeffs[i]);
    }
  }else{ //AG - n_objf > 1 (Multiobjective case)
      //TODO: converter fobj para GTE
      // objective_function->_lits[i] ou ~objective_function->_lits[i]?
      for(int j = 0; j < n_objf; j++){
          int totalw = 0;
            for (int i = 0; i < objective_functions[j]->_lits.size(); i++, totalw += objective_functions[j]->_coeffs[i]);
            
//TODO: Fazer versao standalone to GTE (recebe pbc e devolve conjunto de clausulas e literais
//             na raiz)
//         encode(this, objective_functions[j]->_lits, objective_functions[j]->_coeffs, totalw);
      }
      //      obter os literais do no raiz, R

      //      usar o order encoding nos literais de R (adicionar hard clauses)
      
      //      adicionar os literais em R como soft clauses
      
  }
  
  
  if (getMaximumWeight() == 1)
    setProblemType(_UNWEIGHTED_);
  else
    setProblemType(_WEIGHTED_);
  
  
  //AG
//   printf("MaxSATFormula::convertPBtoMaxSAT -- formula after conversion\n");
//   my_print();
  
}
*/
/*
void Hard:: my_print(indexMap indexToName){   
    printf("c\t");
    for(int i = 0; i < clause.size(); i++)
//         printf(" %s%d", ((sign(clause[i])) ? "~" : ""), std::abs(clause[i]));
        if(indexToName.find(std::abs(clause[i])) != indexToName.end())
            printf(" %s%s", (clause[i] < 0) ? "~" : "", indexToName.at(std::abs(clause[i])).c_str());
        else
            printf(" %s%s", (clause[i] < 0) ? "~" : "", "X");
    printf("\n");
}*/
/*
void Soft:: my_print(indexMap indexToName){   
    printf("c\t");
    for(int i = 0; i < clause.size(); i++)
//         printf(" %s%d", ((sign(clause[i])) ? "~" : ""), std::abs(clause[i]));
        
        if(indexToName.find(std::abs(clause[i])) != indexToName.end())
            printf(" %lu %s%s", weight, (clause[i] < 0) ? "~" : "", indexToName.at(std::abs(clause[i])).c_str());
        else
            printf(" %s%d", (clause[i] < 0) ? "~" : "", std::abs(clause[i]));
    printf(" [");
    for(int i = 0; i < relaxation_vars.size(); i++)
        printf(" %s%d", (relaxation_vars[i] < 0) ? "~" : "", std::abs(relaxation_vars[i]));
    printf("]");
    printf(" A(%d)\n", std::abs(assumption_var));
}
*/


void MaxSATFormula::my_print(){
    printf("c -----------------------------------\n");
    printf("c --------- index to var ------------\n");
//     int k;
//     string v;
    /*
    for (auto const& pair: _indexToName) {
        std::cout << "c\t{" << pair.first+1 << ": " << pair.second << "}\n";
    }*/
    
//     int maxsize = 20;
    int maxcs = 10; //100
    printf("c ------- Objective Func (%d) -----------------\n", n_objf);
    for(int di = 0; di < n_objf; di++)
        objective_functions[di].my_print(_indexToName);
    
    printf("c ------- PB Constraints (%d) ------------\n", pb_constraints.size());
    for(int i = 0; i < pb_constraints.size(); i++){
        pb_constraints[i].my_print(_indexToName);
    }
    
    printf("c ------- Cardinality Constr (%d) --------\n", cardinality_constraints.size());
    if(cardinality_constraints.size() < maxcs){
        for(int i = 0; i < cardinality_constraints.size(); i++){
            cardinality_constraints[i].my_print(_indexToName);
        }
    }

    printf("c ---------------------------------------\n");
    printf("c ------- Hard clauses (%d) --------------\n", hard_clauses.size());
    if(hard_clauses.size() < maxcs){
        for(int i = 0; i < hard_clauses.size(); i++){
            const Clause &clause (hard_clauses.at(i));
            printf("c\t");
            for(size_t j = 0; j < clause.size(); j++)
                if(_indexToName.find(std::abs(clause.at(j))) != _indexToName.end())
                    printf(" %s%s", (clause.at(j) < 0) ? "~" : "", _indexToName.at(std::abs(clause[i])).c_str());
                else
                    printf(" %s%s", (clause.at(j) < 0) ? "~" : "", "X");
            printf("\n");
        }
    }
    /*
    printf("c ------- Soft clauses (%d) --------------\n", soft_clauses.size());
    if(soft_clauses.size() < maxcs){
        for(int i = 0; i < soft_clauses.size(); i++){
            soft_clauses[i].my_print(_indexToName);
        }
    }*/
    
    printf("c ---------------------------------------\n");
    
}
/*
void MaxSATFormula::sync_first(Glucose::Solver *s){
  for(int i = 0; i < nInitialVars(); i++)
    if(s->value(i) != l_Undef)
      fv.insert(mkLit(i,true));
}
*/
