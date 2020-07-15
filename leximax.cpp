// hey hey hey!

// the sorting network is an array of stacks. Each stack contains pairs: the wire number, a variable of the output.
// sorting network: array of pointers to the class stack. Pairs are structs. The wire is an integer. The variable is a string

#include <string>
#include <iostream>
//#include <stack>

// first: read input package upgradeability formula and criteria.
// create sorting network and encoding. Then, destroy sorting network?
// 

int main()
{
    std::list<std::list<std::string>> constraints;
    
    // read input problem.
    
    unsigned long num_vars, num_clauses;
    std::cin >> num_vars >> num_clauses;
    unsigned long i = 0;
    std::list<std::string> clause;
    while(i != num_clauses){
        long literal;
        std::cin >> literal;
        std::string l ("x");
        while(literal!=0){
            if(literal < 0){
                l = "-x";
                literal = -literal;
            }
            l+=std::to_string(literal);
            clause.push_back(l);
            std::cin >> literal;
        }
        
        
        i++;
    }

    
    std::string 
    
    return 0;
}
