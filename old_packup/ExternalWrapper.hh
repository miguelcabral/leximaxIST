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
 * File:   ExternalWrapper.hh
 * Author: mikolas
 *
 * Created on April 19, 2011, 8:33 AM
 * Copyright (C) 2011, Mikolas Janota
 */

#ifndef EXTERNALWRAPPER_HH
#define	EXTERNALWRAPPER_HH
#include <time.h>
#include "common_types.hh"
#include "id_manager.hh"
#include "basic_clset.hh"
#include "SolverWrapperBase.hh"
#include <leximaxIST_Solver.h>

class ExternalWrapper : public SolverWrapperBase<BasicClause*> {
public:
    explicit ExternalWrapper(IDManager& id_manager);
    ~ExternalWrapper();

    virtual void  init();
    virtual XLINT get_top();
    virtual void  set_top(XLINT top);
    virtual bool  solve();  
    bool solve_leximax();

    virtual IntVector& get_model()          { return model; }
    virtual void set_leximax_model(const std::vector<int> &lib_model) {
        model.assign(lib_model.begin(), lib_model.end()); // this allows type conversion (int to LINT)
    }
    
    virtual XLINT      get_min_unsat_cost() {return min_cost;}

    virtual bool register_weight(XLINT weight) {
        std::pair<WeightSet::const_iterator, bool> r = weights.insert(weight);
        return r.second;
    }

    inline void set_opt_solver_cmd(const string& opt_solver_cmd);
    inline void set_multiplication_string(const string& _multiplication_string);
    inline void set_temporary_directory(const string& value);
    inline void set_leave_temporary_files(bool value=true);
    inline void set_leximax(bool value=true);
    inline void set_simplify_last(bool value=true);
    inline void set_maxsat_presolve(bool value = true);
    inline void set_disjoint_cores(bool value = true);
    inline void set_lp_solver (const string &solver_name);
    inline void set_formalism (const string &format);
    inline void set_opt_mode (const string &mode);
    inline void set_maxsat_psol_cmd (const string &cmd);
    inline void set_mss_presolve(int v);
    inline void set_pareto_presolve(int v);
    inline void set_mss_incremental(int v);
    inline void set_pareto_incremental(int v);
    inline void set_mss_add_cls(int v);
    inline void set_mss_tolerance(int v);
    inline void set_mss_nb_limit(int v);
    inline void set_mss_timeout(double t);
    inline void set_pareto_timeout(double t);
    inline void set_truly_pareto(int v);
    inline void set_verbosity (int val);
    void print_leximax_info();
    leximaxIST::Solver* get_leximax_solver() {return m_leximax_solver;}

    void _output_clause (/*const*/ LiteralVector& literals);
    void _output_unary_clause(LINT l);
    void _output_binary_clause(LINT l1, LINT l2);
    void _output_weighted_clause(/*const*/ LiteralVector& literals,XLINT weight);
    void _output_unary_weighted_clause(LINT l, XLINT weight);
    void _output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight);
    BasicClause* _record_clause(LiteralVector& literals);
    virtual void _increase_weight(BasicClause* clause, XLINT weight);
    virtual void dump(ostream& out);

    virtual bool has_solution();
    
private:
    XLINT                      min_cost;
    XLINT                      solution_value;
    IDManager&                 _id_manager;
    BasicClauseSet             clause_set;
    BasicClauseSet             hard_clauses;
    vector<BasicClauseVector>  clause_split;
    vector<LINT>               solution_weights;
    vector< vector <LINT>  >   functions;
    leximaxIST::Solver       *m_leximax_solver;
    IntVector      model;
    WeightSet      weights;
    vector<XLINT>  sorted_weights;
    int    call_counter;
    time_t stamp;
    string opt_solver_cmd;
    bool   m_mss_incr;
    bool   m_gia_incr;
    bool   m_gia_pareto;
    int    m_mss_tolerance;
    int    m_mss_add_cls;
    int    m_mss_nb_limit;
    double m_approx_tout;
    string multiplication_string;
    string temporary_directory;
    bool   leave_temporary_files;
    string m_leximax_approx;
    string m_leximax_opt;
    bool   m_simplify_last;
    bool   m_maxsat_presolve;
    bool   m_disjoint_cores;
    int m_verbosity;
    string m_formalism;
    string m_lp_solver;
    string m_maxsat_psol_cmd;

    vector< vector<LINT> > constraints;
    void   split();
    size_t get_weight_index(XLINT weight) const;
    bool   solve(size_t function_index);
    bool   solve_max();
    bool   solve_it();

    int external_solve(const vector<LINT>& function
                       ,vector< vector <LINT> >& constraints
                       ,IntVector&  tmodel);
    int external_solve_max(IntVector&  tmodel);

    bool has_weight(XLINT weight) const {
        FOR_EACH(WeightSet::const_iterator,weight_index, weights)
            if (*weight_index==weight) return true;
        return false;
    }
    ostream& print_constraint(const vector<LINT>& constraint,ostream& output);
    void print_clause(XLINT weight, ostream& out, BasicClause& clause);
    void write_pbmo_file();
};

inline void ExternalWrapper::set_opt_solver_cmd(const string& cmd) {
  opt_solver_cmd = cmd; }
inline void ExternalWrapper::set_multiplication_string(const string& _multiplication_string) {
  multiplication_string =_multiplication_string; }
inline void ExternalWrapper::set_temporary_directory(const string& value) {
    temporary_directory = value; }
inline void ExternalWrapper::set_leave_temporary_files(bool value/*=true*/) {
    leave_temporary_files = value; }
inline void ExternalWrapper::set_leximax_opt(string value) {
    m_leximax_opt = value; }
inline void ExternalWrapper::set_simplify_last(bool value) {
    m_simplify_last = value; }
inline void ExternalWrapper::set_maxsat_presolve(bool value) {
    m_maxsat_presolve = value; }
inline void ExternalWrapper::set_disjoint_cores(bool value) {
    m_disjoint_cores = value; }
inline void ExternalWrapper::set_lp_solver (const string &solver_name) {
    m_lp_solver = solver_name; }
inline void ExternalWrapper::set_formalism (const string &format) {
    formalism = format; }
inline void ExternalWrapper::set_leximax_approx (const string &mode) {
    m_leximax_approx = mode; }
inline void ExternalWrapper::set_maxsat_psol_cmd (const string &cmd) {
    m_maxsat_psol_cmd = cmd; }
inline void ExternalWrapper::set_verbosity (int val) {
    m_verbosity = val; }
inline void ExternalWrapper::set_mss_presolve(int v) {m_mss_presolve = v; /*conversion from int to bool*/ }
inline void ExternalWrapper::set_mss_incr(int v) { m_mss_incr = v; }
inline void ExternalWrapper::set_gia_incr(int v) { m_gia_incr = v; }
inline void ExternalWrapper::set_mss_add_cls(int v) { m_mss_add_cls = v; }
inline void ExternalWrapper::set_mss_tolerance(int v) { m_mss_tolerance = v; }
inline void ExternalWrapper::set_mss_nb_limit(int v) { m_mss_nb_limit = v; }
inline void ExternalWrapper::set_approx_tout(double t) { m_approx_tout = t; }
inline void ExternalWrapper::set_gia_pareto(int v) { m_gia_pareto = v; }
#endif	/* EXTERNALWRAPPER_HH */

