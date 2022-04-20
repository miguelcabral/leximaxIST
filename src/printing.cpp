#include <leximaxIST_Solver.h>
#include <leximaxIST_printing.h>
#include <leximaxIST_rusage.h>
#include <fstream>
#include <iostream>
#include <list>

namespace leximaxIST {

    void print_error_msg(const std::string &msg)
    {
        std::cerr << "Error leximaxIST: " << msg << '\n';
    }
    
    std::string ordinal (int i)
    {
        std::string i_str (std::to_string(i));
        if (i_str.back() == '1' && i != 11)
            i_str += "st";
        else if (i_str.back() == '2' && i != 12)
            i_str += "nd";
        else if (i_str.back() == '3' && i != 13)
            i_str += "rd";
        else
            i_str += "th";
        return i_str;
    }
    
    void print_lb_map(const std::unordered_map<int, int> &lb_map)
    {
        std::cout << "c -------------- lb_map --------------\n";
        for (const std::pair<int, int> &p : lb_map)
            std::cout << "c " << p.first << " : " << p.second << '\n';
        std::cout << "c ------------------------------------\n";
    }
    
    void print_lower_bounds(const std::vector<int> &lower_bounds)
    {
        std::cout << "c Lower bounds of the maxima: ";
        std::cout << lower_bounds.at(0);
        for (int j (1); j < lower_bounds.size(); ++j)
            std::cout << ", " << lower_bounds.at(j);
        std::cout << '\n';
    }
    
    stream_config set_cout()
    {
        stream_config old_config;
        old_config.flags = std::cout.flags();
        old_config.prec = std::cout.precision();
        std::cout << std::fixed;
        std::cout.precision(5);
        return old_config;
    }
    
    void set_cout(const stream_config &config)
    {
        std::cout.flags(config.flags);
        std::cout.precision(config.prec);
    }
    
    void print_time(double t, const std::string &s)
    {
        const stream_config &old_config (set_cout());
        std::cout << s << t << "s\n";
        set_cout(old_config);
    }
    
    /* print percentage of falsified objective variables by chance
     * print number of calls to SAT solver
     */
    void Solver::print_mss_info(int nb_calls, const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss) const
    {
        int total_nb_vars (0);
        for (const std::vector<int> &obj : m_objectives)
            total_nb_vars += obj.size();
        int todo_size (0);
        for (const std::vector<int> &v : todo_vec)
            todo_size += v.size();
        int mss_size (0);
        for (const std::vector<int> &v : mss)
            mss_size += v.size();
        if (mss_size == 0)
            return;
        const int nb_fixed_vars (total_nb_vars - todo_size);
        const int nb_tested_vars (nb_calls - 1);
        const int nb_lucky_vars (nb_fixed_vars - nb_tested_vars);
        // denominator can not be zero, that has been tested
        double percentage (static_cast<double>(nb_lucky_vars) / mss_size);
        percentage *= 100;
        print_time(read_cpu_time(), "c MSS found: ");
        std::cout << "c Number of SAT calls: " << nb_calls << '\n';
        std::cout << "c Automatically satisfied soft clauses: ";
        const stream_config &old_config (set_cout());
        std::cout << percentage << "% \n";
        set_cout(old_config);
    }
    
    void Solver::print_mss_enum_info() const
    {
        std::cout << "c MSS enumeration...\n";
        std::cout << "c Parameters: \n";
        std::cout << "c \tAddition of satisfied clauses to the MSS in construction: ";
        if (m_mss_add_cls == 0)
            std::cout << "Add all\n";
        else if (m_mss_add_cls == 1)
            std::cout << "Add some, trying to even out the upper bounds\n";
        else if (m_mss_add_cls == 2)
            std::cout << "Add only one\n";
        std::cout << "c \tIncremental enumeration: ";
        std::cout << (m_mss_incr ? "Yes\n" : "No\n");
        std::cout << "c \tTimeout: ";
        const stream_config &old_config (set_cout());
        std::cout << m_approx_tout << '\n';
        set_cout(old_config);
        std::cout << "c \tLimit number of MSSes: ";
        if (m_mss_nb_limit <= 0)
            std::cout << "No limit\n";
        else
            std::cout << m_mss_nb_limit << '\n';
        std::cout << "c \tNext clause choice policy: " << m_mss_tolerance << "% \n";
    }
    
    // in this case we already have computed obj_vec
    void Solver::print_obj_vector(const std::vector<int> &obj_vec) const
    {
        if (!obj_vec.empty()) {
            std::cout << "o ";
            for (int v : obj_vec)
                std::cout << v << ' ';
            std::cout << '\n';
        }
    }
    
    // in this case we compute obj_vec
    void Solver::print_obj_vector() const
    {
        const std::vector<int> &obj_vec (get_objective_vector());
        if (!obj_vec.empty()) {
            std::cout << "o ";
            for (int v : obj_vec)
                std::cout << v << ' ';
            std::cout << '\n';
        }
    }
    
    void Solver::print_soft_clauses() const
    {
        std::cout << "c -------- Soft Clauses of current iteration --------\n";
        for (int lit : m_soft_clauses)
            std::cout << "c " << lit << " 0\n";
    }
    
    void Solver::print_sorted_vec (int i) const
    {
        std::cout << "c ---------------- m_sorted_vecs[" << i << "] -----------------\n";
        for(size_t j{0}; j < m_sorted_vecs.at(i).size(); j++)
            std::cout << "c sorted_vec[" << j << "]: " << m_sorted_vecs.at(i).at(j) << '\n';
    }
    
    // print separately the variables of the jth objective that are in the sorting network and those that are not
    void Solver::print_objs_sorted(const std::vector<std::vector<int>> &inputs_not_sorted) const
    {
        for (int j (0); j < m_num_objectives; ++j) {
            std::cout << "c Variables of the " << ordinal(j+1) << " objective not in the sorting network: ";
            for (int v : inputs_not_sorted.at(j))
                std::cout << v << ' ';
            std::cout << '\n';
            std::cout << "c Variables of the " << ordinal(j+1) << " objective in the sorting network: ";
            // find which variables are in the jth sorting network
            for (int v : m_objectives.at(j)) {
                bool found (false);
                // find v in inputs_not_sorted. If it's there do not print, otherwise print
                for (int v2 : inputs_not_sorted.at(j)) {
                    if (v == v2) {
                        found = true;
                        break;
                    } 
                }
                if (!found)
                    std::cout << v << ' ';
            }
            std::cout << '\n';
        }
    }
    
    void Solver::print_obj_func(int i) const
    {
        const std::vector<int> &objective (m_objectives.at(i));
        size_t num_terms (objective.size());
        std::cout << "c --------------- Objective Function " << i << " (size = ";
        std::cout << num_terms << ") --------------\n";
        for (size_t j = 0; j < num_terms; ++j)
            std::cout << "c " << objective.at(j) << '\n';
    }
    
    void Solver::print_snet_info() const
    {
        for (int i (0); i < m_num_objectives; ++i) {
            std::cout << "c " << ordinal(i + 1) << " Sorting Network: ";
            std::cout << m_snet_info.at(i).first << " wires and " << m_snet_info.at(i).second << " comparators\n";
        }
    }
    
    void Solver::print_mss_debug(const std::vector<std::vector<int>> &todo_vec, const std::vector<std::vector<int>> &mss) const
    {
        std::cout << "c Todo Sizes: ";
        for (int i(0); i < m_num_objectives; ++i)
            std::cout << todo_vec.at(i).size() << ' ';
        std::cout << '\n';
        std::cout << "c MSS Sizes: ";
        for (int i(0); i < m_num_objectives; ++i)
            std::cout << mss.at(i).size() << ' ';
        std::cout << '\n';
        std::cout << "c Upper Bounds: ";
        for (int i(0); i < m_num_objectives; ++i) {
            const int obj_size (m_objectives.at(i).size());
            std::cout << obj_size - mss.at(i).size() << ' ';
        }
        std::cout << '\n';
    }
    
    void Solver::print_waitpid_error(const std::string &errno_str) const
    {
        std::string errmsg ("When calling");
        errmsg += " waitpid() on external solver (pid " + std::to_string(m_child_pid) + "): '";
        errmsg += errno_str + "'";
        print_error_msg(errmsg);
    }

    // leadingStr can be "c ", to print comments, or e.g. "100 " to print weights
    void Solver::print_clause(std::ostream &output, const Clause &clause, const std::string &leadingStr) const
    {
        output << leadingStr;
        for (int lit : clause)
            output << lit << " "; 
        output << "0\n";
    }

    void Solver::print_soft_clauses(std::ostream &output) const
    {
        for (int lit : m_soft_clauses)
            output << "1 " << lit << " 0\n";
    }
    
    void Solver::print_hard_clauses(std::ostream &output) const
    {
        size_t weight (m_soft_clauses.size() + 1);
        for (const Clause &cl : m_input_hard) {
            output << weight << " ";
            print_clause(output, cl);
        }
        for (const Clause &cl : m_encoding) {
            output << weight << " ";
            print_clause(output, cl);
        }
    }

    void Solver::print_pb_constraint(const Clause &cl, std::ostream &output) const
    {
        int num_negatives(0);
        for (int literal : cl) {
            bool sign = literal > 0;
            if (!sign)
                ++num_negatives;
            output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
        }
        output << " >= " << 1 - num_negatives << ";\n";
    }

    void Solver::print_lp_constraint(const Clause &cl, std::ostream &output) const
    {
        int num_negatives(0);
        size_t nb_vars_in_line (0);
        for (size_t j (0); j < cl.size(); ++j) {
            int literal (cl.at(j));
            bool sign = literal > 0;
            if (!sign)
                ++num_negatives;
            if (j == 0)
                output << 'x' << (sign ? literal : -literal);
            else
                output << (sign ? " + " : " - ") << 'x' << (sign ? literal : -literal);
            nb_vars_in_line++;
            if (nb_vars_in_line == 5) {
                output << '\n';
                nb_vars_in_line = 0;
            }
        }
        output << " >= " << 1 - num_negatives << '\n';
    }
    /*
    void Solver::print_atmost_pb(int i, std::ostream &output) const
    {
        // i = 1 means position 0, i = 2, means position 1, etc
        const std::list<int> &relax_vars (m_all_relax_vars[i-1]);
        for (int var : relax_vars) {
            output << "-1" << m_multiplication_string << "x" << var << " ";
        }
        output << " >= " << -i << ";\n";
    }

    void Solver::print_atmost_lp(int i, std::ostream &output) const
    {
        // i = 1 means position 0, i = 2, means position 1, etc
        const std::list<int> &relax_vars (m_all_relax_vars[i-1]);
        bool first_iteration (true);
        for (int var : relax_vars) {
            if (first_iteration) {
                output << 'x' << var;
                first_iteration = false;
            }
            else
                output << " + " << 'x' << var;
        }
        output << " <= " << i << '\n';
    }

    void Solver::print_sum_equals_pb(int i, std::ostream &output) const
    {
        const std::list<int> &relax_vars (m_all_relax_vars.back());
        for (int var : relax_vars) {
            output << "+1" << m_multiplication_string << "x" << var << " ";
        }
        output << " = " << i << ";\n";
    }

    void Solver::print_sum_equals_lp(int i, std::ostream &output) const
    {
        const std::list<int> &relax_vars (m_all_relax_vars.back()); // last relax vars
        bool first_iteration (true);
        for (int var : relax_vars) {
            if (first_iteration) {
                output << 'x' << var;
                first_iteration = false;
            }
            else
                output << " + " << 'x' << var;
        }
        output << " = " << i << '\n';
    }*/
    
    void Solver::print_sorted_true() const
    {
        if (!m_solution.empty()){
            std::cout << "c Sorted vecs true variables:" << '\n';
            int j = 0;
            for (const std::vector<int> &sorted_vec : m_sorted_vecs) {
                std::cout << "c Sorted vec " << j << ": ";
                for (int var : sorted_vec) {
                    if (m_solution.at(var) > 0)
                        std::cout << var << ' ';
                }
                std::cout << '\n';
                ++j;
            }
            j = 1;
            for (const std::vector<std::vector<int>> &sorted_relax_vecs : m_sorted_relax_collection) {
                std::cout << "c Sorted Relax Vecs true variables of iteration " << j << ":" << '\n';
                int k (0);
                for (const std::vector<int> &sorted_relax : sorted_relax_vecs) {
                    std::cout << "c Sorted Relax vec " << k << ": ";
                    for (int var : sorted_relax) {
                        if (m_solution[var] > 0)
                            std::cout << var << ' ';
                    }
                    std::cout << '\n';
                    ++k;
                }
                ++j;
            }
        }
    }
    
    // prints solution to std output in a similar format to the MaxSAT output format
    void Solver::print_solution() const 
    {
        // print solution status
        std::cout << "s ";
        if (m_status == 's')
            std::cout << "SATISFIABLE\n";
        if (m_status == 'u') {
            std::cout << "UNSATISFIABLE\n";
            return;
        }
        if (m_status == 'o')
            std::cout << "OPTIMUM FOUND\n";
        if (m_status == '?') {
            std::cout << "UNKNOWN\n";
            return;
        }
        size_t i (1);
        std::vector<int> solution (get_solution());
        while (i < solution.size()) {
            std::string line ("v ");
            while (line.size() < 80 && i < solution.size()) {
                line += std::to_string(solution.at(i)) + ' '; // to string ???
                ++i;
            }
            std::cout << line << '\n';
        }
    }

}/* namespace leximaxIST */
