#include <leximaxIST_Solver.h>
#include <leximaxIST_rusage.h>
#include <leximaxIST_printing.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <list>
#include <cmath>

namespace leximaxIST {
    
    int mss_choose_obj_seq (const std::vector<std::vector<int>> &todo_vec)
    {
        int obj_index;
        // obj_index points to the first objective whose todo is not empty
        for (obj_index = 0; obj_index < todo_vec.size(); ++obj_index) {
            if (!todo_vec.at(obj_index).empty())
                break;
        }
        if (obj_index == todo_vec.size()) // all empty
            obj_index = -1;
        return obj_index;
    }
    
    void erase_from_todo (std::vector<std::vector<int>> &todo_vec, const int obj_index, const int var_index)
    {
        // erase next_var from todo by moving last element to its position
        todo_vec.at(obj_index).at(var_index) = todo_vec.at(obj_index).back();
        todo_vec.at(obj_index).pop_back();
    }
    
    int mss_choose_obj_max (const std::vector<std::vector<int>> &todo_vec, const std::vector<int> &upper_bounds)
    {
        int max (*std::max_element(upper_bounds.begin(), upper_bounds.end()));
        for (int i (0); i < todo_vec.size(); ++i) {
            if (upper_bounds.at(i) == max)
                return i;
        }
        return -1; // can not reach this line
    }
    
    /* choose next obj in todo using an heuristic - choose the maximum if
     * the ratio between the min and the max of upper_bounds is greater than m_mss_tolerance
     * returns the index of the objective that we will try to minimise next
     * if the maximum can not be improved, then stop, that is, return -1
     * check how much each objective can decrease: check todo and upper_bounds
     * if there is an objective that can not, in the best case, be less than best_max
     * then stop - return -1
     */
    int Solver::mss_choose_obj (const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss, const int best_max) const
    {
        // compute the upper bounds
        std::vector<int> upper_bounds (m_num_objectives);
        for (int j (0); j < m_num_objectives; ++j)
            upper_bounds.at(j) = m_objectives.at(j).size() - mss.at(j).size();
        // first check if the maximum can not be improved
        for (int j (0); j < m_num_objectives; ++j) {
            const int todo_size (todo_vec.at(j).size());
            if (upper_bounds.at(j) - todo_size >= best_max)
                return -1;
        }
        const int max (*std::max_element(upper_bounds.begin(), upper_bounds.end()));
        const int min (*std::min_element(upper_bounds.begin(), upper_bounds.end()));
        const int ratio ((max - min) * 100 / max);
        if (ratio >= m_mss_tolerance)
            return mss_choose_obj_max (todo_vec, upper_bounds);
        else
            return mss_choose_obj_seq (todo_vec);
    }
    
    /* Add satisfied soft clauses (or falsified variables) to the mss
     * Which clauses are added to the mss is defined by parameter m_mss_add_cls:
     * 0 - add all clauses
     * 1 - add as much as possible while simultaneously trying to even out the upper bounds
     * 2 - add only the satisfied clause used in the SAT test (already added)
     * remove those variables from todo_vec
     * update assumps
     * update upper_bounds
     */
    void Solver::mss_add_falsified (IpasirWrap *solver, const std::vector<int> &model, std::vector<std::vector<int>> &mss, std::vector<std::vector<int>> &todo_vec, std::vector<int> &assumps)
    {
        std::vector<std::vector<int>> vars_to_add (m_num_objectives); // by objective
        // find all satisfied soft clauses that haven't been added to the mss
        for (int j (0); j < m_num_objectives; ++j) {
            std::vector<int> &todo (todo_vec[j]);
            for (size_t i (0); i < todo.size(); ++i) {
                int var (todo.at(i));
                if (model[var] < 0)
                    vars_to_add.at(j).push_back(i);
            }
        }
        // add the clauses to the mss according to m_mss_add_cls
        if (m_mss_add_cls == 0 || m_mss_add_cls == 1) {
            // compute the maximum of the best case decreased upper bounds
            // add clauses for each objective until the upper bound is decreased to max, if possible
            std::vector<int> upper_bounds (m_num_objectives);
            for (int j (0); j < m_num_objectives; ++j)
                upper_bounds.at(j) = m_objectives.at(j).size() - mss.at(j).size();
            int max (upper_bounds.at(0) - vars_to_add.at(0).size());
            for (int j (1); j < m_num_objectives; ++j) {
                const int best_case_ub (upper_bounds.at(j) - vars_to_add.at(j).size());
                if (max < best_case_ub)
                    max = best_case_ub;
            }
            for (int j (0); j < m_num_objectives; ++j) {
                const std::vector<int> add (vars_to_add.at(j));
                std::vector<int> &todo (todo_vec[j]);
                int limit_to_add; // number of variables to add to objective j
                if (m_mss_add_cls == 0)
                    limit_to_add = add.size(); // all
                else
                    limit_to_add = upper_bounds.at(j) - max; // even out upper bounds
                // add clauses to mss and remove them from todo
                // must do so from end to begining, because of how we erase from todo
                for (int k (limit_to_add - 1); k >= 0; --k) { // Do not change the order!
                    const int i (add.at(k));
                    int var (todo.at(i));
                    // add clause to the mss
                    if (m_mss_incr)
                        assumps.push_back(-var);
                    else
                        solver->addClause(-var);
                    mss.at(j).push_back(-var);
                    // erase element in todo by moving last element to current position
                    erase_from_todo(todo_vec, j, i);
                }
            }
        }
        else if (m_mss_add_cls == 2) { // do not add
            // nothing to do here
        }
    }
    
    // TODO: what to do if the MSS subset returned is empty? 
    // No blocking clause! Use the subset of MCS?
    /* MSS enumeration, using basic/extended linear search
     * MSS search: if the maximum can not be improved, stop
     * Thus, we will actually get subsets of the MSSes
     * Enumeration stops if a limit of time or number of MSSes is reached
     */
    void Solver::mss_enumerate()
    {
        if (m_verbosity >= 1)
            print_mss_enum_info();
        int nb_msses (0);
        std::vector<int> obj_vec (get_objective_vector());
        int best_max (*std::max_element(obj_vec.begin(), obj_vec.end()));
        std::vector<Clause> blocking_cls;
        IpasirWrap *solver (nullptr);
        double initial_time (read_cpu_time());
        while (true) {
            if (m_mss_nb_limit > 0 && nb_msses >= m_mss_nb_limit)
                break;
            IpasirWrap new_solver;
            if (m_mss_incr)
                solver = m_sat_solver;
            else {
                solver = &new_solver;
                solver->addClauses(m_input_hard);
                solver->addClauses(blocking_cls);
            }
            std::vector<std::vector<int>> mss (m_num_objectives);
            solver->set_timeout(m_approx_tout, initial_time);
            const int rv (mss_linear_search(mss, solver, best_max));
            // remove timeout
            solver->set_timeout(std::numeric_limits<double>::max(), 0);
            if (rv != 10)
                break; // SAT call was interrupted or UNSAT (all msses were found)
            // blocking clause - at least one clause of the satisfiable subset is false
            int mss_size (0);
            for (const std::vector<int> &s : mss)
                mss_size += s.size();
            // if mss is empty then all msses were found
            if (mss_size == 0)
                break;
            Clause block_mss (mss_size);
            { // construct blocking clause
                int i (0);
                for (const std::vector<int> &s : mss) {
                    for (int lit : s) {
                        block_mss.at(i) = -lit;
                        ++i;
                    }
                }
            }
            if (m_mss_incr)
                add_hard_clause(block_mss);
            else
                blocking_cls.push_back(block_mss);
            ++nb_msses;
        }
        if (m_verbosity == 2) {
            std::cout << "c Blocking clauses:\n";
            for (const Clause &c : blocking_cls) {
                std::cout << "c ";
                for (const int lit : c)
                    std::cout << lit << ' ';
                std::cout << '\n';
            }
        }
        if (m_verbosity >= 1)
            std::cout << "c Number of MSS subsets found: " << nb_msses << '\n';
    }
    
    /* Compute an MSS of the problem with sum of obj funcs 
     * Stop if the upper bound can't be improved
     * returns rv, the return value of the first call to the sat solver:
     * 10 - sat means there is another mss
     * 20 - unsat means all msses were found
     * 0 - interrupted means timeout reached
     * in the end, variable mss is the sets of chosen satisfied soft clauses, by objective
     */
    int Solver::mss_linear_search(std::vector<std::vector<int>> &mss, IpasirWrap *solver, int &best_max)
    {
        // is there another MSS?
        std::vector<int> assumps;
        int rv (solver->solve());
        if (rv != 10)
            return rv; // UNSAT or interrupted
        // SAT, but the MSS may be empty. If so, all MSSes have been found
        std::vector<int> model (solver->model()); // copy
        const std::vector<int> &obj_vec (set_solution(solver->model())); // move
        best_max = *std::max_element(obj_vec.begin(), obj_vec.end()); 
        std::vector<std::vector<int>> todo_vec (m_num_objectives);
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> &objective (m_objectives.at(i));
            todo_vec[i] = objective; // copy assignment
        }
        mss_add_falsified (solver, model, mss, todo_vec, assumps);
        int nb_calls (1);
        while (true /*stops when obj_index == -1 or if SAT call is interrupted*/) {
            if (m_verbosity == 2)
                print_mss_debug(todo_vec, mss);
            int obj_index (mss_choose_obj (todo_vec, mss, best_max));
            if (obj_index == -1)
                break;
            const int next_var (todo_vec.at(obj_index).at(0));
            assumps.push_back(-next_var);
            const int rv_local = solver->solve(assumps);
            if (rv_local == 0) {
                rv = 0; // interrupted
                break;
            }
            if (rv_local == 10) { // SAT
                model = solver->model(); // copy
                const std::vector<int> &obj_vec (set_solution(solver->model())); // move
                best_max = *std::max_element(obj_vec.begin(), obj_vec.end());
                // add clause to the mss
                if (!m_mss_incr) {
                    assumps.pop_back();
                    solver->addClause(-next_var);
                }
                // update mss
                mss.at(obj_index).push_back(-next_var);
                // remove next_var from todo (BEFORE mss_add_falsified)
                erase_from_todo(todo_vec, obj_index, 0);
                mss_add_falsified (solver, model, mss, todo_vec, assumps);
            }
            else { // UNSAT
                // add clause to the mcs (backbone literals)
                assumps.pop_back();
                if (!m_mss_incr)
                    solver->addClause(next_var);
                // remove next_var from todo
                erase_from_todo(todo_vec, obj_index, 0);
            }
            ++nb_calls;
        }
        if (m_verbosity >= 1)
            print_mss_info(nb_calls, todo_vec, mss);
        return rv;
    }

} /* namespace leximaxIST */
