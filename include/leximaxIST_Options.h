#ifndef LEXIMAXIST_OPTIONS
#define LEXIMAXIST_OPTIONS
#include <string>

namespace leximaxIST {

    template <typename T>
    class Option {
    public:
        Option(const T &data);
        T& get_data();
        const std::string& get_description() const;
        void set_description(const std::string &d);
    private:
        std::string m_description;
        T m_data;
    };

    template <typename T>
    Option<T>::Option(const T &data) : m_data(data) {}

    template <typename T>
    T& Option<T>::get_data()
    {
        return m_data;
    }

    template <typename T>
    void Option<T>::set_description(const std::string &d)
    {
        m_description = d;
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
        Option<std::string> m_optimise;
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
        void print_usage(std::ostream &os);
        int get_help() const;
        int get_verbosity() const;
        int get_disjoint_cores() const;
        std::string get_optimise() const;
        std::string get_approx() const;
        std::string get_input_file_name() const;
        double get_timeout() const;
        int get_mss_tol() const;
        int get_mss_add_cls() const;
        int get_mss_incr() const;
        int get_gia_pareto() const;
        int get_gia_incr() const;
        
    private:
        void read_integer(const char *optarg, const std::string &optname, int &i);
        void read_digit(const char *optarg, const std::string &optname, int &digit);
        void read_double(const char *optarg, const std::string &optname, double &d);
    };

}

#endif
