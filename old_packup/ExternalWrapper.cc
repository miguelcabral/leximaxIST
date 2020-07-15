/******************************************************************************\
 *    This file is part of packup.                                            *
 *                                                                            *
 *    packup is free software: you can redistribute it and/or modify          *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    packup is distributed in the hope that it will be useful,               *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with packup.  If not, see <http://www.gnu.org/licenses/>.         *            
\******************************************************************************/           
/* 
 * File:   ExternalWrapper.cc
 * Author: mikolas
 * 
 * Created on April 19, 2011, 8:33 AM
 * Copyright (C) 2011, Mikolas Janota
 */

#include <algorithm>
#include <zlib.h>
#include <iostream>
#include "ExternalWrapper.hh"
#include "fmtutils.hh"
using std::sort;

ExternalWrapper::ExternalWrapper(IDManager& id_manager)
:min_cost(LONG_MAX)
,solution_value (-1)
,_id_manager(id_manager)
,solver_command("minisat+ -cs -ansi")
,multiplication_string("*")
,temporary_directory("/tmp")
,leave_temporary_files(false)
,iterative(true)
{}

void clause_to_constraint(BasicClause& clause, vector<LINT>& constraint);

bool ExternalWrapper::solve() {
    stamp = time(NULL);
    // set up datastructures
    const size_t function_count = weights.size();
    sorted_weights.insert(sorted_weights.end (),weights.begin(), weights.end ());
    sort(sorted_weights.begin(), sorted_weights.end(),greater<XLINT>());

    solution_weights.resize(function_count,-1);
    functions.resize(function_count);
    clause_split.resize(function_count);
    split(); // split clauses into the classes according to weight
    min_cost=get_top();    
    return iterative ? solve_it() : solve_max();
}

bool ExternalWrapper::solve_it() {
    const size_t function_count = weights.size();
    // generate hard clauses
    size_t i=constraints.size();
    constraints.resize(i+hard_clauses.size());
    FOR_EACH(cset_iterator, clause_index, hard_clauses) {
        vector<LINT>& constraint = constraints[i++];
        clause_to_constraint(**clause_index, constraint);
    }
    bool solution_found = false;
    if (function_count==0) {
        vector<LINT> dummy;
        external_solve(dummy, constraints, model);
        solution_found = !model.empty();
    }
    //solve one by one
    for  (size_t function_index = 0;function_index < function_count ;++function_index) {
        solution_found = solve(function_index);
        if (!solution_found) break;
    }
    return solution_found;
}

bool ExternalWrapper::has_solution() {return !model.empty();}

bool ExternalWrapper::solve(const size_t function_index) {
    cerr << "# starting function level " << function_index << endl;
    // relaxation of current function
    vector<LINT>& relaxation_literals = functions[function_index];
    const BasicClauseVector& clauses=clause_split[function_index];
    FOR_EACH(BasicClauseVector::const_iterator,clause_index,clauses) {
       BasicClause& clause = **clause_index;
       if (clause.size()==1) {
         relaxation_literals.push_back(-(*(clause.begin())));
       } else {
           vector<LINT> constraint;
           const LINT relaxation_variable = _id_manager.new_id();
           relaxation_literals.push_back(relaxation_variable);
           constraint.push_back(relaxation_variable);
           clause_to_constraint(clause, constraint);
           constraints.push_back(constraint);
       }
    }        
    // call external solver
    vector<LINT> temporary_model;
    external_solve(relaxation_literals, constraints, temporary_model);
    if (temporary_model.empty())  return false; // stop here
    // copy solution to the output model
    model.clear();
    model.insert (model.end(), temporary_model.begin(), temporary_model.end());
    //constrain the following computation based on the score
    LINT score = 0;
    FOR_EACH(vector<LINT>::const_iterator,literal_index,relaxation_literals) {
        const LINT literal = *literal_index;
        const bool fsign = literal>0;
        const size_t var = fsign ? (size_t)literal : (size_t)(-literal);
        const bool msign = model[var] > 0;
        if (!msign) continue;
        if (fsign) ++score;
        else --score;
    }
    solution_weights[function_index] = score;
    const vector<LINT>& cs = functions[function_index];
    if (cs.size() != 0) {
        vector<LINT> c;
        FOR_EACH(vector<LINT>::const_iterator, literal_index, cs) c.push_back(-(*literal_index));
        c.push_back(-score);
        constraints.push_back(c);
    }

    //min_cost = abs(score); //todo
    return model.size() != 0; // ? -1 : score;
}


int ExternalWrapper::external_solve(const vector<LINT>& function, vector< vector<LINT> >& constraints
                       ,IntVector&  tmp_model) {
    stringstream strstr;
    strstr<<temporary_directory << "/" << "o" << stamp << "_" << (call_counter++) << ".opb";
    const string input_file_name = strstr.str();
    ofstream output(input_file_name.c_str());
    // prepare input for the solver
    output << "* #variable= " << _id_manager.top_id() << " #constraint= " << constraints.size() << endl;
    if (function.size() > 0) {// print minimization function
        output << "min:";
        FOR_EACH(vector<LINT>::const_iterator, literal_index, function) {
            const LINT literal = *literal_index;
            const bool sign = literal > 0;
            output << " " << (sign ? "+1" : "-1") << multiplication_string << "x" << (sign ? literal : -literal);
        }
        output << ";" << endl;
    }
    FOR_EACH(vector< vector<LINT> >::const_iterator,constraint_index,constraints) {
        print_constraint(*constraint_index, output) << endl;// print constraints
    }
    output.close();
    // call the solver
    stringstream scommand;
    const string output_filename = input_file_name + ".out";
    scommand << solver_command << " " << input_file_name << " >" << output_filename;
    const string command = scommand.str();
    const int retv = system (command.c_str());
    cerr << "# " <<  "external command finished with exit value " << retv << endl;
    gzFile of=gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);//TODO
    StreamBuffer r(of);
    bool sat=false;
    tmp_model.resize((size_t)(_id_manager.top_id()+1),0);
    while (*r != EOF) {
        if (*r != 'v') {// ignore all the other lines
            skipLine(r);
        } else {
            sat=true;
            ++r; // skip 'v'
            while ( (*r != '\n')  && (*r != EOF)  && (*r != '\r') ) {
                skipTrueWhitespace(r);
                const bool sign = (*r) != '-';
                if ((*r == '+') || (*r == '-')) ++r;
                if ((*r == 'x')) ++r;
                if (*r < '0' || *r > '9') break;
                const LINT l = parseInt(r);
                // if ( model.size()<=(size_t)l )
                //cerr << "# " << l << " " << (sign ? l : -l) << endl;
                assert(tmp_model.size()>(size_t)l);
                tmp_model[l] = (sign ? l : -l);
            }
            assert (*r=='\n');
            ++r; // skip '\n'
        }
    }
    if (!sat) tmp_model.clear();
    if (!leave_temporary_files) {
        remove(input_file_name.c_str());
        remove(output_filename.c_str());
    }

    return retv;
}

bool ExternalWrapper::solve_max() {
    // call external solver
    vector<LINT> temporary_model;
    external_solve_max(temporary_model);
    if (temporary_model.empty())  return false; // stop here
    // copy solution to the output model
    model.clear();
    model.insert (model.end(), temporary_model.begin(), temporary_model.end());
    //min_cost = abs(score); //todo
    return model.size() != 0; // ? -1 : score;
}

int ExternalWrapper::external_solve_max(IntVector&  tmp_model) {
    stringstream strstr;
    strstr<<temporary_directory << "/" << "o" << stamp << "_" << (call_counter++) << ".wcnf";
    const string input_file_name = strstr.str();
    ofstream output(input_file_name.c_str());
    // prepare input for the solver
    dump(output);
    output.close();
    // call the solver
    stringstream scommand;
    const string output_filename = input_file_name + ".out";
    scommand << solver_command << " " << input_file_name << " >" << output_filename;
    const string command = scommand.str();
    const int retv = system (command.c_str());
    cerr << "# " <<  "external command finished with exit value " << retv << endl;
    gzFile of=gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);//TODO
    StreamBuffer r(of);
    bool sat=false;
    tmp_model.resize((size_t)(_id_manager.top_id()+1),0);
    while (*r != EOF) {
        if (*r != 'v') {// ignore all the other lines
            skipLine(r);
        } else {
            sat=true;
            ++r; // skip 'v'
            while ( (*r != '\n')  && (*r != EOF)  && (*r != '\r') ) {
                skipTrueWhitespace(r);
                const bool sign = (*r) != '-';
                if ((*r == '+') || (*r == '-')) ++r;
                if (*r < '0' || *r > '9') break;
                const LINT l = parseInt(r);
                assert(tmp_model.size()>(size_t)l);
                tmp_model[l] = (sign ? l : -l);
            }
            assert (*r=='\n');
            ++r; // skip '\n'
        }
    }
    if (!sat) tmp_model.clear();
    if (!leave_temporary_files) {
        remove(input_file_name.c_str());
        remove(output_filename.c_str());
    }

    return retv;
}


void ExternalWrapper::split() {
    FOR_EACH(cset_iterator, clause_index, clause_set) {
        BasicClause* clause = *clause_index;
        if (clause_set.is_cl_hard(clause)) {
            hard_clauses.attach_clause(clause);
        } else {
            XLINT total_weight = clause_set.get_cl_weight(clause);
            FOR_EACH(WeightSet::const_iterator,weight_index, weights) {
                // split into duplicate clauses
                const XLINT weight = *weight_index;
                const XLINT      k = total_weight/weight;
                total_weight %= weight;
                for (XLINT i = 0; i<k; ++i) {
                    const size_t wi = get_weight_index(weight);
                    clause_split[wi].push_back(clause);
                }
            }
        }
    }
}

ostream& ExternalWrapper::print_constraint (const vector<LINT>& constraint,ostream& output) {
    const size_t sz = constraint.size();
    assert(sz>=2);
    for (size_t index=0; index<sz-1; ++index) {
        const LINT literal = constraint[index];
        const bool sign    = literal>0;
        output << (sign ? "+1" : "-1") << multiplication_string << "x" << (sign ? literal : -literal) << " ";
    }
    output << " >= " << constraint[sz-1] << ";";
    return output;
}


void clause_to_constraint(BasicClause& clause, vector<LINT>& constraint) {
    LINT rh=1;
    FOR_EACH(Literator, literal_index, clause) {
       const LINT literal = *literal_index;
       constraint.push_back(literal);
       if (literal<0) --rh;
    }
    constraint.push_back(rh);
}

size_t ExternalWrapper::get_weight_index(XLINT weight) const {
    for (size_t i=0; i<sorted_weights.size (); ++i)
        if (sorted_weights[i]==weight) return i;
    assert(false);
    return -1;
}

void ExternalWrapper::init() {
    clause_set.set_def_cl_weight(0);
    call_counter = 0;
}

 XLINT ExternalWrapper::get_top() {return clause_set.get_top();}
 void ExternalWrapper::set_top(XLINT top) {clause_set.set_top(top);}

void ExternalWrapper::_output_clause(/*const*/ LiteralVector& literals) {
    BasicClause* clause = clause_set.create_clause(literals);
    clause_set.set_cl_hard(clause);
}

void ExternalWrapper::_output_unary_clause(LINT l) {
    BasicClause* clause = clause_set.create_unit_clause(l);
    clause_set.set_cl_hard(clause);
}

void ExternalWrapper::_output_binary_clause(LINT l1, LINT l2) {
    BasicClause* clause = clause_set.create_binary_clause(l1, l2);
    clause_set.set_cl_hard(clause);
}

void ExternalWrapper::_output_weighted_clause(/*const*/ LiteralVector& literals, XLINT weight) {
    BasicClause* clause = clause_set.create_clause(literals);
    clause_set.incr_cl_weight(clause, weight);
}

void ExternalWrapper::_output_unary_weighted_clause(LINT l, XLINT weight) {
    BasicClause* clause = clause_set.create_unit_clause(l);
    clause_set.incr_cl_weight(clause, weight);
}

void ExternalWrapper::_output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight) {
    BasicClause* clause = clause_set.create_binary_clause(l1, l2);
    clause_set.incr_cl_weight(clause, weight);
}

BasicClause* ExternalWrapper::_record_clause(LiteralVector& literals) {
    BasicClause* clause = clause_set.create_clause(literals);
    return clause;
}

void ExternalWrapper::_increase_weight(BasicClause* clause, XLINT weight) {
    assert(has_weight(weight));
    clause_set.incr_cl_weight(clause, weight);
}


 void ExternalWrapper::dump(ostream& out) {
     size_t c = hard_clauses.size();
     FOR_EACH(vector< BasicClauseVector >::const_iterator, i, clause_split) {
         c+=i->size();
     }

     out << "p wcnf"
             << " " << _id_manager.top_id()
             << " " << c
             << " " << clause_set.get_top()
             << endl;

     FOR_EACH(cset_iterator, clause_index, hard_clauses) {
            BasicClause& clause = **clause_index;
            out << get_top() << " " << clause << endl;
     }

     for (size_t i=0;i<clause_split.size(); ++i) {
         const BasicClauseVector& clauses = clause_split[i];
         FOR_EACH(BasicClauseVector::const_iterator,clause_index,clauses) {
             BasicClause& clause = **clause_index;
             out << clause.get_weight()
                     << " " << clause
                     << endl;
         }
     }
 }
