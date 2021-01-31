#include <leximaxIST_Encoder.h>
#include <fstream>
#include <iostream>
#include <forward_list>

namespace leximaxIST {

    void Encoder::print_error_msg(const std::string &msg) const
    {
        if (m_err_file.empty())
            std::cerr << "Error leximaxIST: " << msg << std::endl;
        else {
            std::ofstream err (m_err_file, std::ofstream::app);
            if (err.is_open())
                err << "Error leximaxIST: " << msg << std::endl;
            else {
                std::cerr << "Error leximaxIST: " << "Can't open error log file '";
                std::cerr << m_err_file << "'. Logging to Standard error instead.";
                std::cerr << std::endl;
                std::cerr << "Error leximaxIST: " << msg << std::endl;
            }
        }
    }

    void Encoder::print_waitpid_error(const std::string &errno_str) const
    {
        std::string errmsg ("When calling");
        errmsg += " waitpid() on external solver (pid " + std::to_string(m_child_pid) + "): '";
        errmsg += errno_str + "'";
        print_error_msg(errmsg);
    }

    void Encoder::print_clause(std::ostream &output, const Clause *clause) const
    {
        for (long long lit : *clause)
            output << lit << " "; 
        output << "0\n";
    }

    void Encoder::print_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight) const
    {
        for (Clause * const cl : clauses) {
            output << weight << " ";
            print_clause(output, cl);
        }
    }

    void Encoder::print_pb_constraint(const Clause *cl, std::ostream &output) const
    {
        long long num_negatives(0);
        for (long long literal : *cl) {
            bool sign = literal > 0;
            if (!sign)
                ++num_negatives;
            output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
        }
        output << " >= " << 1 - num_negatives << ";\n";
    }

    void Encoder::print_lp_constraint(const Clause *cl, std::ostream &output) const
    {
        long long num_negatives(0);
        size_t nb_vars_in_line (0);
        for (size_t j (0); j < cl->size(); ++j) {
            long long literal (cl->at(j));
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

    void Encoder::print_atmost_pb(int i, std::ostream &output) const
    {
        // i = 1 means position 0, i = 2, means position 1, etc
        const std::forward_list<long long> &relax_vars (m_all_relax_vars[i-1]);
        for (long long var : relax_vars) {
            output << "-1" << m_multiplication_string << "x" << var << " ";
        }
        output << " >= " << -i << ";\n";
    }

    void Encoder::print_atmost_lp(int i, std::ostream &output) const
    {
        // i = 1 means position 0, i = 2, means position 1, etc
        const std::forward_list<long long> &relax_vars (m_all_relax_vars[i-1]);
        bool first_iteration (true);
        for (long long var : relax_vars) {
            if (first_iteration) {
                output << 'x' << var;
                first_iteration = false;
            }
            else
                output << " + " << 'x' << var;
        }
        output << " <= " << i << '\n';
    }

    void Encoder::print_sum_equals_pb(int i, std::ostream &output) const
    {
        const std::forward_list<long long> &relax_vars (m_all_relax_vars.back());
        for (long long var : relax_vars) {
            output << "+1" << m_multiplication_string << "x" << var << " ";
        }
        output << " = " << i << ";\n";
    }

    void Encoder::print_sum_equals_lp(int i, std::ostream &output) const
    {
        const std::forward_list<long long> &relax_vars (m_all_relax_vars.back()); // last relax vars
        bool first_iteration (true);
        for (long long var : relax_vars) {
            if (first_iteration) {
                output << 'x' << var;
                first_iteration = false;
            }
            else
                output << " + " << 'x' << var;
        }
        output << " = " << i << '\n';
    }

}/* namespace leximaxIST */
