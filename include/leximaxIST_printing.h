#ifndef LEXIMAXIST_PRINTING
#define LEXIMAXIST_PRINTING
#include <string>
#include <iostream>

namespace leximaxIST {
    
    void print_error_msg(const std::string &msg);
    
    void print_time(double t, const std::string &s);
    
    struct stream_config {
        std::streamsize prec;
        std::ios_base::fmtflags flags;
    };
    
    stream_config set_cout();
    
    void set_cout(const stream_config &config);

}

#endif
