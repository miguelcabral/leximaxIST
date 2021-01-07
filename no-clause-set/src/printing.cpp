#include <Leximax_encoder.h>

void Leximax_encoder::print_error_msg(const std::string &msg)
{
    std::cerr << "Error leximaxIST: " << msg << std::endl;
}

void Leximax_encoder::print_clause(std::ostream &output, std::vector<LINT> * const clause)
{
    for (LINT lit : *clause)
        output << lit << " "; 
    output << "0\n";
}

void Leximax_encoder::write_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight)
{
    for (Clause * const cl : clauses) {
        output << weight << " ";
        print_clause(output, cl);
    }
}

void Leximax_encoder::write_pbconstraint(Clause * const cl, std::ostream &output) {
    LINT num_negatives(0);
    for (LINT literal : *cl) {
        bool sign = literal > 0;
        if (!sign)
            ++num_negatives;
        output << (sign ? "+1" : "-1") << m_multiplication_string << "x" << (sign ? literal : -literal) << " ";
    }
    output << " >= " << 1 - num_negatives << ";\n";
}

void Leximax_encoder::write_lpconstraint(Clause * const cl, std::ostream &output) {
    LINT num_negatives(0);
    size_t nb_vars_in_line (0);
    for (size_t j (0); j < cl->size(); ++j) {
        LINT literal (cl->at(j));
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

void Leximax_encoder::write_atmost_pb(int i, std::ostream &output)
{
    // i = 1 means position 0, i = 2, means position 1, etc
    const std::forward_list<LINT> &relax_vars (m_all_relax_vars[i-1]);
    for (LINT var : relax_vars) {
        output << "-1" << m_multiplication_string << "x" << var << " ";
    }
    output << " >= " << -i << ";\n";
}

void Leximax_encoder::write_atmost_lp(int i, std::ostream &output)
{
    bool first_iteration (true);
    /*for (LINT var : m_relax_vars) {//TODO: if bug is due to relax_vars then change this to m_all_relax_vars!!!!!!
        if (first_iteration) {
            output << 'x' << var;
            first_iteration = false;
        }
        else
            output << " + " << 'x' << var;
    }*/
    output << " <= " << i << '\n';
}

void Leximax_encoder::write_sum_equals_pb(int i, std::ostream &output)
{
    const std::forward_list<LINT> &relax_vars (m_all_relax_vars.back());
    for (LINT var : relax_vars) {
        output << "+1" << m_multiplication_string << "x" << var << " ";
    }
    output << " = " << i << ";\n";
}

void Leximax_encoder::write_sum_equals_lp(int i, std::ostream &output)
{
    bool first_iteration (true);
    /*for (LINT var : m_relax_vars) {//TODO: if bug is due to relax_vars then change this to m_all_relax_vars!!!!!!
        if (first_iteration) {
            output << 'x' << var;
            first_iteration = false;
        }
        else
            output << " + " << 'x' << var;
    }*/
    output << " = " << i << '\n';
}
