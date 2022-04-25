#ifndef LEXIMAXIST_ILPCONSTRAINT
#define LEXIMAXIST_ILPCONSTRAINT
#include <cassert>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

namespace leximaxIST {

    class ILPConstraint {
    public:
        ILPConstraint(const std::vector<int> &vars,
            const std::vector<int> &coeffs,
            const std::string &sign,
            int rhs
        ) :
        m_vars(vars),
        m_coeffs(coeffs),
        m_sign(sign),
        m_rhs(rhs)
        {
            assert(m_sign == ">=" || m_sign == "<=" || m_sign == "=");
            assert(m_vars.size() == m_coeffs.size());
        }
        
        std::vector<int> m_vars;
        std::vector<int> m_coeffs;
        std::string m_sign; // can be either '<=', '>=' or '='
        int m_rhs;
        
        void print(std::ostream &os) const
        {
            std::string line ("");
            for (size_t i (0); i < m_vars.size(); ++i) {
                line += (m_coeffs.at(i) > 0) ? " + " : " - ";
                line += std::to_string(std::abs(m_coeffs.at(i)));
                line += " x" + std::to_string(m_vars.at(i)) + " ";
                if (line.size() >= 80) {
                    os << line << '\n';
                    line = "";
                }
            }
            line += m_sign + " " + std::to_string(m_rhs);
        }
    }; // ILPConstraint definition

} // namespace leximaxIST

#endif
