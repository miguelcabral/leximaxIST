#ifndef LEXIMAXIST_PRINTING
#define LEXIMAXIST_PRINTING
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace leximaxIST {
    
    void print_error_msg(const std::string &msg);
    
    void print_time(double t, const std::string &s);
    
    std::string ordinal(int i);
    
    void print_lb_map(const std::unordered_map<int, int> &lb_map);
    
    void print_lower_bounds(const std::vector<int> &lower_bounds);
    
    struct stream_config {
        std::streamsize prec;
        std::ios_base::fmtflags flags;
    };
    
    stream_config set_cout();
    
    void set_cout(const stream_config &config);

}

#endif
