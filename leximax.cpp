// hey hey hey!

// the sorting network is an array of stacks. Each stack contains pairs: the wire number, a variable of the output.
// sorting network: array of pointers to the class stack. Pairs are structs. The wire is an integer. The variable is a string

#include <string>
#include <iostream>
#include <forward_list>

// first: read input package upgradeability formula and criteria.
// create sorting network and encoding. Then, destroy sorting network?
// 

void print_list(std::forward_list<std::forward_list<long>*> &hard_clauses)
{
    while(!hard_clauses.empty()){
        std::forward_list<long> *clause = hard_clauses.front();
    };
}

int main()
{
    std::forward_list<std::forward_list<long>*> hard_clauses;
    std::forward_list<std::forward_list<long>*> soft_clauses;
    std::forward_list<std::forward_list<unsigned long>*> objectives;
    std::forward_list<unsigned long> objective_sizes;
    unsigned long id_count;
    
    // read input problem.
    
    unsigned long num_vars, num_clauses;
    short num_objectives;
    std::cin >> num_vars >> num_clauses >> num_objectives;
    id_count = num_vars;
    unsigned long i = 1;
    while(i != num_clauses){
        std::forward_list<long> *clause = new std::forward_list<long>;
        long literal;
        std::cin >> literal;
        while(literal!=0){
            clause->push_front(literal);
            std::cin >> literal;
        };
        hard_clauses.push_front(clause);        
        i++;
    };
    
    short j = 1;
    while(j != num_objectives){
        unsigned long num_terms = 0;
        std::forward_list<unsigned long> *objective = new std::forward_list<unsigned long>;
        unsigned long var;
        std::cin >> var;
        while(var!=0){
            num_terms++;
            objective->push_front(var);
            std::cin >> var;
        };
        objectives.push_front(objective);
        objective_sizes.push_front(num_terms);
        num_terms = 0;
        j++;
    };
    
    std::cout << objectives.front()->front() << std::endl;
    std::cout << objective_sizes.front() << std::endl;
    objective_sizes.pop_front();
    //std::cout << objective_sizes.front() << std::endl;
    
    // encode with odd even merge sorting network
    
    
    
    
    return 0;
}
