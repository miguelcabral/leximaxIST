#ifndef LEXIMAXIST_OPTIONS
#define	LEXIMAXIST_OPTIONS
#include <string>
#include <vector>

namespace leximaxIST {

template <typename T>
class Option {
public:
    Option(const T &data);
    void set_data(const T &data);
    const T& get_data() const;
    const std::string& get_description() const;
    void set_description(const std::string &description);
private:
    std::string m_description;
    T m_data;
};

template <typename T>
Option<T>::Option(const T &data) : m_data = data {}

template <typename T>
void Option<T>::set_data(const T &data)
{
    m_data = data;
}

template <typename T>
const T& Option<T>::get_data() const
{
    return m_data;
}

template <typename T>
const std::string& Option<T>::get_description() const
{
    return m_description;
}

class Options {
private:
    Option<int> m_help;
    Option<int> m_verbosity;
    Option<int> m_simplify_last;
    Option<std::string> m_solve;
    Option<std::string> m_input_file_name;
    Option<int> m_disjoint_cores;
    Option<std::string> m_approx;
    Option<double> m_timeout;
    Option<int> m_mss_tol;
    Option<int> m_mss_add_cls;
    Option<int> m_mss_incr;
    Option<int> m_gia_pareto;
    Option<int> m_gia_incr;
    
public:
    Options(); // starts Options with default settings and descriptions
    bool parse(int count, char** arguments); // changes settings based on user input while checking if input is valid
};

}

#endif

