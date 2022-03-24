#ifndef LEXIMAXIST_OPTIONS
#define	LEXIMAXIST_OPTIONS
#include <string>
#include <vector>

namespace leximaxIST {

// TODO: class Option; add all options of library
template <typename T>
class Option {
public:
    Option(std::string &name, std::string &description);
    void set_data(const T &data);
    const T& get_data() const;
    const std::string& get_description() const;
private:
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
private:
    Option<int> m_help;
    Option<int> m_verbosity;
    Option<int> m_simplify_last;
    
public:
    Options();
    bool parse(int count,char** arguments);
    void add_option(std::string &name, );
};

}

#endif

