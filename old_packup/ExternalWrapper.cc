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
#include <leximaxIST_types.h>

using std::sort;

ExternalWrapper::ExternalWrapper(IDManager& id_manager)
:min_cost(LONG_MAX)
,solution_value(-1)
,_id_manager(id_manager)
,opt_solver_cmd("minisat+ -cs -ansi")
,multiplication_string("*")
,temporary_directory("/tmp")
,leave_temporary_files(false)
,m_disjoint_cores(false)
,m_leximax_solver(nullptr)
{}

ExternalWrapper::~ExternalWrapper()
{
    delete m_leximax_solver;
    m_leximax_solver = nullptr;
}

void clause_to_constraint(BasicClause& clause, vector<LINT>& constraint);

void print_pb_constraint(Literator i, Literator end, std::ostream &os)
{
    int num_negatives(0);
    while (i != end) {
        int literal (*i);
        bool sign = literal > 0;
        if (!sign)
            ++num_negatives;
        os << (sign ? "+1" : "-1") << " x" << (sign ? literal : -literal) << " ";
        ++i;
    }
    os << ">= " << 1 - num_negatives << " ;\n";
}

void ExternalWrapper::write_pbmo_file()
{
    // print header
    const std::string star_sep (79, '*');
    std::cout << star_sep << '\n';
    std::cout << "* " << clause_split.size() << " objectives\n";
    int nb_vars (_id_manager.top_id());
    for (const BasicClauseVector &soft_cls: clause_split)
        nb_vars += soft_cls.size();
    std::cout << "* " << nb_vars << " variables\n";
    int nb_constraints (hard_clauses.size());
    // add the constraints for adding a variable equivalent to each soft clause
    for (const BasicClauseVector &soft_cls: clause_split) {
        for (BasicClause *cl : soft_cls) {
            ++nb_constraints; // new var implies cl
            nb_constraints += cl->size(); // cl imples new var
        }
    }
    std::cout << "* " << nb_constraints << " constraints\n";
    std::cout << star_sep << '\n';
    // print objective functions
    int obj_var (_id_manager.top_id() + 1);
    std::vector<std::vector<LINT>> obj_clauses;
    //std::ofstream weight_os ("/home/mcabral/scripts-thesis/weights.txt", std::ofstream::app);
    for (const BasicClauseVector &soft_cls: clause_split) {
        std::cout << "min: ";
        for (BasicClause *cl : soft_cls) {
            //weight_os << cl->get_weight() << '\n';
            //neg of new var implies cl
            std::vector<LINT> lits;
            lits.push_back(obj_var);
            for (LINT l : *cl)
                lits.push_back(l);
            obj_clauses.push_back(lits);
            // cl imples neg of new var
            for (LINT l : *cl) {
                lits.clear();
                lits.push_back(-obj_var);
                lits.push_back(-l);
                obj_clauses.push_back(lits);
            }
            // print obj_var
            std::cout << "+1 x" << obj_var << ' ';
            ++obj_var;
        }
        std::cout << ";\n";
    }
    //weight_os.close();
	/*
	LINT five_long (-4294967291);
	int five_normal (-5);
	std::cout << "LINT: " << five_long << '\n';
	std::cout << "int: " << five_normal << '\n';
    for (BasicClause *cl : hard_clauses) {
		std::cout << "hard clause: ";
		for (LINT lit : *cl)
			std::cout << lit << ' ';
		std::cout << '\n';
	}
	*/
    for (BasicClause *cl : hard_clauses)
        print_pb_constraint(cl->begin(), cl->end(), std::cout);
    for (std::vector<LINT> &cl : obj_clauses)
        print_pb_constraint(cl.begin(), cl.end(), std::cout);
    exit(EXIT_SUCCESS);
}

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
    // The next line has been used to store a file with the corresponding pbmo instance
    //write_pbmo_file();
    if ((!m_leximax_approx.empty()) || (!m_leximax_opt.empty()))
        return solve_leximax();
    else {
        if (formalism == "opb")
            return solve_it();
        if (formalism == "wcnf")
            return solve_max();
        // otherwise:
        std::cerr << "Error: Can not solve lexicographic with formalism '" << formalism << "'. Try 'opb' or 'wcnf'." << std::endl;
        return false;
    }
}

bool ExternalWrapper::solve_it() {
    const size_t function_count = weights.size();
    // generate hard clauses
    size_t i=constraints.size();
    constraints.resize(i+hard_clauses.size());
    /* FOR_EACH(cset_iterator, clause_index, hard_clauses) { */
    for(BasicClause* pclause : hard_clauses) {
        vector<LINT>& constraint = constraints[i++];
        /* clause_to_constraint(**clause_index, constraint); */
        clause_to_constraint(*pclause, constraint);
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
    model.insert(model.end(), temporary_model.begin(), temporary_model.end());
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
    scommand << opt_solver_cmd << " " << input_file_name << " >" << output_filename;
    const string command = scommand.str();
    const int retv = system (command.c_str());
    cerr << "# " <<  "external command finished with exit value " << retv << endl;
    gzFile of=gzopen(output_filename.c_str(), "rb");
    assert(of!=NULL);//TODO
    StreamBuffer r(of);
    bool sat=false;
    tmp_model.resize(static_cast<size_t>(_id_manager.top_id()+1),0);
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
    scommand << opt_solver_cmd << " " << input_file_name << " >" << output_filename;
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

ostream& ExternalWrapper::print_constraint(const vector<LINT>& constraint,ostream& output) {
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

void ExternalWrapper::print_clause(XLINT weight, ostream& out, BasicClause& clause) {
     out << weight;
     for (auto l : clause) out << " " << l;
     out << " 0" << endl;
}

 void ExternalWrapper::dump(ostream& out) {
     size_t c = hard_clauses.size();
     FOR_EACH(vector< BasicClauseVector >::const_iterator, i, clause_split)
         c+=i->size();

     const auto top = get_top();
     out << "p wcnf"
             << " " << _id_manager.top_id()
             << " " << c
             << " " << top
             << endl;

     FOR_EACH(cset_iterator, clause_index, hard_clauses) {
         BasicClause& clause = **clause_index;
         out << get_top() << " " << clause << endl;
         print_clause(top, out, clause);
     }

     for (size_t i=0;i<clause_split.size(); ++i) {
         const BasicClauseVector& clauses = clause_split[i];
         FOR_EACH(BasicClauseVector::const_iterator,clause_index,clauses) {
             BasicClause& clause = **clause_index;
             print_clause(clause.get_weight(), out, clause);
         }
     }
 }

 void ExternalWrapper::print_leximax_info() {
     assert(m_leximax_solver != nullptr);
     /*size_t sorting_net_size (m_leximax_solver->get_sorting_net_size());
     std::cerr << "# Number of comparators of largest sorting network: " << sorting_net_size << '\n';*/
     if (m_leximax_solver->get_status() == 's' || m_leximax_solver->get_status() == 'o') {
     	const std::vector<int> obj_vector (m_leximax_solver->get_objective_vector());
     	std::cerr << "# Objective vector: ";
     	for (int o : obj_vector)
            std::cerr << o << ' ';
     	std::cerr << '\n';
     }
     std::vector<int> obj_vec_dbg;
     // count number of falsified clauses and print them
     for (BasicClauseVector &soft_cls: clause_split) {
         //std::cerr << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
         int obj_val (0);
         for (BasicClause *clause : soft_cls) {
            bool falsified (true);
            // assume all are falsified
            // If one literal is satisfied set falsified to false, otherwise the assumption holds
            for (LINT lit : *clause) {
                bool is_var (lit > 0);
                int v (model.at(std::abs(lit)));
                if ((is_var && v > 0) || (!is_var && v < 0))
                    falsified = false;
            }
            if (falsified) {
                ++obj_val;
                // print clause
                /*std::cerr << "FALSIFIED: ";
                for (LINT lit : *clause)
                    std::cerr << lit << ' ';
                std::cerr << '\n';*/
            }
         }
         obj_vec_dbg.push_back(obj_val);
     }
     std::cerr << "# Number of falsified soft clauses (by objective): ";
     for (int o : obj_vec_dbg)
         std::cerr << o << ' ';
     std::cerr << '\n';
 }
 
 /* approximate and/or optimise using leximax criterion
  * returns true if and only if the instance is satisfiable
  */
 bool ExternalWrapper::solve_leximax() {
     // create leximax solver object
     m_leximax_solver = new leximaxIST::Solver();
     // add hard clauses to m_leximax_solver
     for (BasicClause *hc : hard_clauses) {
         leximaxIST::Clause c;
         for (LINT lit : *hc)
             c.push_back(lit);
         m_leximax_solver->add_hard_clause(c);
     }
     // add each objective function
     for (BasicClauseVector &soft_packup: clause_split) {
         std::vector<leximaxIST::Clause> soft_leximax;
         for (BasicClause *sc : soft_packup) {
            leximaxIST::Clause c;
            for (LINT lit : *sc)
                c.push_back(lit);
            soft_leximax.push_back(c);
         }
         m_leximax_solver->add_soft_clauses(soft_leximax);
     }
     // set external solvers and parameters of m_leximax_solver
     m_leximax_solver->set_simplify_last(m_simplify_last);
     // disjoint cores presolving for the core-guided algorithm
     m_leximax_solver->set_disjoint_cores(m_disjoint_cores);
     // approximation
     m_leximax_solver->set_approx_tout(m_approx_tout);
     // approximation with gia
     m_leximax_solver->set_gia_incr(m_gia_incr);
     m_leximax_solver->set_gia_pareto(m_gia_pareto);
     // approximation using msses
     m_leximax_solver->set_mss_add_cls(m_mss_add_cls);
     m_leximax_solver->set_mss_incr(m_mss_incr);
     m_leximax_solver->set_mss_nb_limit(m_mss_nb_limit);
     m_leximax_solver->set_mss_tol(m_mss_tolerance);
     // other
     m_leximax_solver->set_verbosity(m_verbosity);
     m_leximax_solver->set_ext_solver_cmd(opt_solver_cmd);
     m_leximax_solver->set_multiplication_string(multiplication_string);
     m_leximax_solver->set_leave_tmp_files(leave_temporary_files);
     m_leximax_solver->set_formalism(formalism);
     m_leximax_solver->set_ilp_solver(m_lp_solver);
     m_leximax_solver->set_maxsat_presolve(m_maxsat_presolve);
     if (!m_maxsat_psol_cmd.empty())
        m_leximax_solver->set_maxsat_psol_cmd(m_maxsat_psol_cmd);
     // solve
     char status = '?';
     if (!m_leximax_approx.empty()) { // approximation
         m_leximax_solver->set_approx(m_leximax_approx);
         m_leximax_solver->approximate();
         status = m_leximax_solver->get_status();
     }
     if (!m_leximax_opt.empty() && status != 'u') { // optimisation
         m_leximax_solver->set_opt_mode(m_leximax_opt);
         m_leximax_solver->optimise();
         status = m_leximax_solver->get_status();
     }
     if (status == 's' || status == 'o') {
        std::vector<int> sol (m_leximax_solver->get_solution());
        model.assign(sol.begin(), sol.end());
        print_leximax_info();
     }
     else
         model.clear();
     // return did I find a solution? 
     return status == 's' || status == 'o';
 }
 
