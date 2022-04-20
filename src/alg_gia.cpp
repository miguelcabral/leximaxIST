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

    
    // TODO: Add lower bounds from maxsat presolving
    /* Approximate the Leximax Optimisation Problem with a "greedy" technique that is:
     * 1) less "greedy" than MCS enumeration
     * 2) works towards getting intermediate Pareto-optimal solutions
     * the MCSes are not Pareto-optimal necessarily
     * Disadvantage: finding one Pareto-optimal solution is slower than finding an MCS
     * This is based on the Guided Improvement Algorithm (GIA)
     */
    void Solver::gia()
    {
        int max_index (0);
        bool skip (false);
        double initial_time (read_cpu_time());
        while (true /*stops when interrupted or last max can not be improved*/) {
            IpasirWrap new_solver;
            IpasirWrap *solver (nullptr);
            if (!m_gia_incr) {
                solver = &new_solver;
                solver->addClauses(m_input_hard);
                solver->addClauses(m_encoding);
            }
            else
                solver = m_sat_solver;
            solver->set_timeout(m_approx_tout, initial_time);
            if (!skip) {// skip if we know the solution can not be improved
                const int rv (pareto_search(max_index, solver));
                if (rv == 0)
                    break;
            }
            // unsat
            if (max_index == m_num_objectives - 1 && skip) {
                /* skip being true implies:
                 * max m_num_objectives - 2 can not improve
                 * max m_num_objectives - 1 has been minimised
                 */
                break;
            }
            if (max_index == m_num_objectives)
                break;
            // check if the ith max can be improved without restrictions on smaller objs
            const std::vector<int> &obj_vec = get_objective_vector();
            if (m_verbosity == 2) {
                std::cout << "c Current objective vector: ";
                print_obj_vector(obj_vec);
            }
            std::vector<int> s_obj_vec (obj_vec);
            std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
            if (s_obj_vec.at(max_index) == 0)
                break;
            std::vector<int> assumps;
            // fix values of previous maxima in assumps
            fix_previous_max(assumps, max_index, obj_vec);
            // bound the remaining objs with max - 1
            decrease_max(assumps, max_index, obj_vec);
            // solve with m_sat_solver because the new_solver is now forever unsat
            // set upper bound on all objectives as hard clauses
            if (max_index == 0) 
                encode_ub_sorted(s_obj_vec.at(max_index));
            double t (0.0);
            if (m_verbosity >= 1) {
                t = read_cpu_time();
                std::cout << "c Calling SAT solver...\n";
            }
            const int rv (m_sat_solver->solve(assumps));
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - t, "c SAT call CPU time: ");
            if (rv == 0)
                break;
            if (rv == 20) {// unsat -> move on to next max
                skip = m_gia_pareto;
                ++max_index;
                if (max_index == m_num_objectives)
                    break;
            }
            else { // sat -> try to decrease further the current max
                skip = false;
                set_solution(m_sat_solver->model());
            }
        }
        // unset timeout
        m_sat_solver->set_timeout(std::numeric_limits<double>::max(), 0);
    }
    
    /* adds to unit_clauses the clauses that:
     * fix some objectives whose values are equal to previous maxima
     * CONVENTION: if there are previous maxima equal to the current maximum,
     * then fix the objectives equal to the previous maxima with the smallest index
     */
    void Solver::fix_previous_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const
    {
        std::vector<int> s_obj_vec (obj_vec);
        std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
        int min_prev (0);
        if (max_index > 0) // otherwise it does not matter, no objs will be fixed
            min_prev = s_obj_vec.at(max_index - 1); // minimum of previous maxima
        // fix the first max_index objectives with values in the range [min_prev, max]
        int j (0);
        for (int i (0); i < max_index; ++j) {
            // i is the nb of fixed objs; j is the obj index
            const int obj_val (obj_vec.at(j));
            if (obj_val >= min_prev) {
                const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
                const int size (sorted_vec.size());
                const int pos (size - obj_val); // 0 <= pos <= size
                if (pos < size)
                    unit_clauses.push_back(sorted_vec.at(pos)); // lower bound
                if (pos - 1 >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << j << ": = " << obj_val << '\n';
                ++i; // one more fixed
            }
        }
    }
    
    /* adds to unit_clauses the clauses that
     * upper bound the remaining not fixed objectives; bound: <= max - 1
     */
    void Solver::decrease_max(std::vector<int> &unit_clauses, int max_index, const std::vector<int> &obj_vec) const
    {
        std::vector<int> s_obj_vec (obj_vec);
        std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
        int min_prev (0);
        if (max_index > 0) // otherwise it does not matter, no objs will be fixed
            min_prev = s_obj_vec.at(max_index - 1); // minimum of previous maxima
        // upper bound (<= max-1) the objectives not fixed to previous maxima
        int j (0);
        for (int i (0); i < max_index; ++j) {
            // i is the nb of fixed objs; j is the obj index
            const int obj_val (obj_vec.at(j));
            if (obj_val >= min_prev)
                ++i; // fixed obj
            else {
                const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
                const int size (sorted_vec.size());
                const int ub (s_obj_vec.at(max_index) - 1);
                const int pos (size - ub); // 0 <= pos <= size
                if (pos - 1 >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << j << ": <= " << ub << '\n';
            }
        }
        // bound the remaining objs
        for (; j < m_num_objectives; ++j) {
            const std::vector<int> &sorted_vec (m_sorted_vecs.at(j));
            const int size (sorted_vec.size());
            const int ub (s_obj_vec.at(max_index) - 1); // <= max-1
            const int pos (size - ub); // 0 <= pos <= size
            if (pos - 1 >= 0)
                unit_clauses.push_back(-sorted_vec.at(pos - 1)); // upper bound
            if (m_verbosity == 2)
                std::cout << "c Objective " << j << ": <= " << ub << '\n';
        }
    }
    
    /* adds to unit_clauses the clauses that bound the objectives less than or equal to max
     */
    void Solver::bound_objs(std::vector<int> &unit_clauses, int max, const std::vector<int> &obj_vec) const
    {
        for (int i (0); i < m_num_objectives; ++i) {
            const std::vector<int> &sorted_vec (m_sorted_vecs.at(i));
            const int obj_val (obj_vec.at(i));
            const int size (sorted_vec.size());
            if (obj_val < max) {
                const int pos (size - obj_val - 1); // -1 <= pos <= size - 1
                if (pos >= 0)
                    unit_clauses.push_back(-sorted_vec.at(pos)); // upper bound
                if (m_verbosity == 2)
                    std::cout << "c Objective " << i << ": <= " << obj_val << '\n';
            }
        }
    }
    
    // TODO: Add lower bounds from maxsat presolving
    /* Find one Pareto-optimal solution
     */
    // It may happen in this function that we prove that the current max can't improve
    // then we do not need to check that again in the pareto_presolve function
    // keep track of which maxs are optimal - advance max_index
    // how do we know when the current max can't improve?
    // when max_index == max_index_local, we get unsat and the smaller objs
    // differ from the current local max by at most 1
    int Solver::pareto_search(int &max_index, IpasirWrap *solver)
    {
        double initial_time (read_cpu_time());
        int nb_calls (0);
        int rv;
        int max_index_local (max_index);
        while (true/* ends when unsat or interrupted */) {
            std::vector<int> assumps;
            const std::vector<int> &obj_vec = get_objective_vector();
            if (m_verbosity == 2) {
                std::cout << "c Current objective vector: ";
                print_obj_vector(obj_vec);
            }
            std::vector<int> s_obj_vec (obj_vec);
            std::sort(s_obj_vec.begin(), s_obj_vec.end(), descending_order);
            const int max (s_obj_vec.at(max_index_local));
            if (max == 0) {
                rv = 20; // unsat - can not improve max
                break;
            }
            std::vector<int> unit_clauses;
            // decrease current max - bound all objectives not fixed and with value <= max
            decrease_max(unit_clauses, max_index_local, obj_vec);
            // add clauses to the problem
            for (int lit : unit_clauses) {
                if (!m_gia_incr && !m_gia_pareto)
                    solver->addClause(lit);
                else
                    assumps.push_back(lit);
            }
            unit_clauses.clear(); // clear clauses already added to the problem
            // fix previous maxima
            fix_previous_max(unit_clauses, max_index_local, obj_vec); // construct clauses
            // bound (permanently if not incremental) all objs with their current values
            bound_objs(unit_clauses, max, obj_vec); // construct clauses
            // add clauses to the problem
            for (int lit : unit_clauses) {
                if (m_gia_incr)
                    assumps.push_back(lit);
                else
                    solver->addClause(lit);
            }
            double initial_time;
            if (m_verbosity >= 1) {
                initial_time = read_cpu_time();
                std::cout << "c Calling SAT solver...\n";
            }
            rv = solver->solve(assumps);
            if (m_verbosity >= 1)
                print_time(read_cpu_time() - initial_time, "c SAT call CPU time: ");
            ++nb_calls;
            if (rv == 10)
                set_solution(solver->model());
            if (rv == 20) {
                /* check if we have proved that the current max can't improve
                 * removing the constraints on the smaller objectives
                 * check if the objectives <= max only differ by at most 1
                 * and if max_index == max_index_local
                 */
                const int min (s_obj_vec.at(m_num_objectives - 1));
                if (max_index == max_index_local && max - min <= 1)
                    ++max_index;
                if (m_gia_pareto) {
                    // continue until a Pareto-optimal solution is obtained
                    ++max_index_local;
                    if (max_index_local == m_num_objectives)
                        break; // Pareto-optimal solution has been found
                }
                else // stop to check if we can improve the max_index maximum
                    break;
            }
            if (rv == 0)
                break;
        }
        if (m_verbosity >= 1) {
            print_time(read_cpu_time() - initial_time, "c Single Pareto Optimal Solution CPU time: ");
            std::cout << "c Number of SAT calls: " << nb_calls << '\n';
        }
        return rv;
    }

} /* namespace leximaxIST */
