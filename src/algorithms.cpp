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

    
    void Solver::optimise()
    {
        if (m_verbosity >= 2)
            std::cout << std::unitbuf; // debug - flushes the output stream after any output operation       
        m_input_nb_vars = m_id_count;
        if (m_verbosity > 0 && m_verbosity <= 2) {
            std::cout << "c Optimising using algorithm " << m_opt_mode << "...\n";
            std::cout << "c Number of input variables: " << m_input_nb_vars << '\n';
            std::cout << "c Number of input hard clauses: " << m_input_hard.size() << '\n';
            std::cout << "c Number of objective functions: " << m_num_objectives << '\n';
        }
        if (m_verbosity == 2) { // print obj functions
            for (int k (0); k < m_num_objectives ; ++k)
                print_obj_func(k);
        }
        double initial_time (read_cpu_time());
        if (m_status != '?') {
            print_error_msg("Called optimise() twice without changing the formula.");
            exit(EXIT_FAILURE);
        }
        // check if solve() is called without a problem
        if (m_num_objectives == 0) {
            print_error_msg("The problem does not have an objective function");
            exit(EXIT_FAILURE);
        }
        if (m_input_hard.empty()) {
            print_error_msg("The problem does not have constraints");
            exit(EXIT_FAILURE);
        }
        if (m_num_objectives == 1) {
            print_error_msg("The problem is single-objective");
            exit(EXIT_FAILURE);
        }
        // check if problem is satisfiable
        if (!call_sat_solver(m_sat_solver, {})) {
            m_status = 'u';
            return;
        }
        m_status = 's'; // update status to SATISFIABLE
        if (m_opt_mode.substr(0, 4) == "core")
            optimise_core_guided();
        else
            optimise_non_core(0);            
        if (m_verbosity >= 1) // print total solving time
            print_time(read_cpu_time() - initial_time, "c Optimisation CPU time: ");
        m_status = 'o'; // update status to OPTIMUM FOUND
    }

    void Solver::approximate()
    {
        double initial_time (read_cpu_time());
        // check if problem is satisfiable
        if (!call_sat_solver(m_sat_solver, {})) {
            m_status = 'u';
            return;
        }
        m_status = 's'; // update status to SATISFIABLE
        if (m_approx == "gia") {
            if (m_verbosity >= 1)
                std::cout << "c Approximating using Guided Improvement Algorithm (GIA)...\n";
            // encode sorted vectors with sorting network
            for (int j (0); j < m_num_objectives; ++j)
                encode_sorted(m_objectives.at(j), j);
            encode_bounds(0, 0); // upper bound all objs based on the solution given by the SAT solver
            gia();
        }
        else if (m_approx == "mss") {
            if (m_verbosity >= 1)
                std::cout << "c Approximating using Maximal Satisfiable Subsets...\n";
            mss_enumerate();
        }
        else {
            print_error_msg("Invalid approximation algorithm");
            exit(EXIT_FAILURE);
        }
        if (m_verbosity >= 1)
            print_time(read_cpu_time() - initial_time, "c Approximation CPU time: ");
    }
    
} /* namespace leximaxIST */
