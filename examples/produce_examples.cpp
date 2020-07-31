#include <string> // stoi, std::string, append...
#include <iostream> // std::cout
#include <fstream> // ofstream
#include <math.h> // pow

int main(int argc, char *argv[])
{
    int example_top_size = std::stoi(argv[1]);
    for(int i = 2; i <= example_top_size; ++i){
        // produce file with objective function x_1 + x_2 + ... + x_i
        std::string file_name ("f_");
        file_name += std::to_string(i);
        file_name += ".cnf";
        std::ofstream obj_file (file_name, std::ofstream::out);
        if(!obj_file){
            std::cerr << "Could not open " << file_name << " for writing" << std::endl;
            return 1;
        }
        obj_file << "p cnf " << std::to_string(i) << " " <<  std::to_string(i) << '\n';
        for(int j = 1; j <= i; ++j){
            obj_file << std::to_string(-j) << " 0\n";
        }
        obj_file.close();
        // create many files with each possible assignment to x_1,...,x_i. There are pow(2,i) assignments.
        for(int j = 0; j < pow(2,i); ++j){
            file_name = "hard_";
            file_name += std::to_string(i) + "_" + std::to_string(j);
            file_name += ".cnf";
            std::ofstream constraints_file (file_name, std::ofstream::out);
            if(!constraints_file){
                std::cerr << "Could not open " << file_name << " for writing" << std::endl;
                return 1;
            }
            constraints_file << "p cnf " << std::to_string(i) << " " <<  std::to_string(i) << '\n';
            int k = j;
            for(int x = 1; x <= i; ++x){
                if(k != 0){
                    if(k % 2 == 0)
                        constraints_file << std::to_string(-x) << " 0\n";
                    else
                        constraints_file << std::to_string(x) << " 0\n";
                    k = k/2;
                }
                else
                    constraints_file << std::to_string(-x) << " 0\n";
            }
            constraints_file.close();
        }        
    }
    return 0;
}
