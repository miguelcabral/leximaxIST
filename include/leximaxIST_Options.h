#ifndef LEXIMAXIST_OPTIONS
#define	LEXIMAXIST_OPTIONS
#include <string>
#include <vector>

// TODO: class Option; add all options of library
template <typename T>
class Option {
public:
    Option(std::string &name, std::string &description);
    void set_data(const T &data);
    const T& get_data() const;
    const std::string& get_name() const;
    const std::string& get_description() const;
private:
    std::string m_name;
    std::string m_description;
    T m_data;
};

template <typename T>
Option<T>::Option(std::string &name, std::string &description)
{
    m_name = name;
    m_description = description;
}

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
const std::string& Option<T>::get_name() const
{
    return m_name;
}

template <typename T>
const std::string& Option<T>::get_description() const
{
    return m_description;
}

class Options {
public:
    Options();
    bool   parse(int count,char** arguments);
    
    int    m_help;
    string  m_solver;
    std::vector<string>  m_input_files;
    string  m_multiplication_string;
    int    m_leave_temporary_files;
    int    m_pbo;
    int m_num_objectives;
};

#endif

