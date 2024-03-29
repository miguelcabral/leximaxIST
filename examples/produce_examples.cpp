#include <string> // stoi, std::string, append...
#include <iostream> // std::cout
#include <fstream> // ofstream
#include <math.h> // pow
#include <time.h> // time
#include <stdlib.h> // rand

void write_random_clauses(std::ostream &out, int num_clauses, int num_vars)
{
    out << "p cnf " << num_vars << ' ' <<  num_clauses << '\n';
    for (int i(1); i <= num_clauses; ++i) {
        int rand_num_of_lits ((rand() % 4) + 1); // random number of literals
        for (int j(1); j <= rand_num_of_lits; ++j) {
            // generate random literal
            int rand_sign (rand() % 2);
            int rand_lit (rand() % num_vars + 1);
            if (rand_sign == 0)
                rand_lit = -rand_lit;
            out << rand_lit << ' ';
        }
        out << "0\n";        
    }
}

int main()
{
    for (int i(1); i <= 30; ++i) {
        std::string file_name ("hard_");
        file_name += std::to_string(i);
        file_name += ".cnf";
        std::ofstream of (file_name, std::ofstream::out);
        if(!of){
            std::cerr << "Could not open " << file_name << " for writing" << std::endl;
            return 1;
        }
        // create 3 random clauses from a set of 3 variables
        write_random_clauses(of, 4, 8);
        of.close();
    }
    for (int i(1); i < 4; ++i) {
        std::string base_name ("f_");
        base_name += std::to_string(i);
        for (int j(1); j <= 30; ++j) {
            std::string file_name (base_name);
            file_name += '_' + std::to_string(j);
            file_name += ".cnf";
            std::ofstream of (file_name, std::ofstream::out);
            if(!of){
                std::cerr << "Could not open " << file_name << " for writing" << std::endl;
                return 1;
            }
            // create 3 random clauses from a set of 3 variables
            write_random_clauses(of, 3, 10);
            of.close();
        }
    }
    return 0;
}
