#include <Leximax_encoder.h>

void Leximax_encoder::set_solver_command(std::string &command)
{
    m_solver_command = command;
}

void Leximax_encoder::set_solver_format(std::string &format)
{
    m_solver_format = format;
}

void Leximax_encoder::set_lp_solver(std::string &lp_solver)
{
    m_lp_solver = lp_solver;
}

void Leximax_encoder::set_leave_temporary_files(bool val) { m_leave_temporary_files = val; }

void Leximax_encoder::set_multiplication_string(std::string &str) { m_multiplication_string = str; }
