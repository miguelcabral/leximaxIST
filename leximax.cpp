#include <string>
#include <iostream> // std::cout, std::cin
#include <forward_list>
#include <vector>
#include <utility> // std::pair

// first: read input package upgradeability formula and criteria.
// create sorting network and encoding. Then, destroy sorting network?
// 

typedef std::vector<std::forward_list<std::pair<unsigned long, unsigned long>>> SNET;

void encode_network(std::forward_list<std::forward_list<long>*> &hard_clauses, std::vector<unsigned long> &elems_to_sort, unsigned long *id_count, std::forward_list<unsigned long> *objective, SNET &sorting_network)
{    
    if(elems_to_sort.size() == 2){
        // single comparator.
        
    }
}

void print_list(std::forward_list<std::forward_list<long>*> &hard_clauses)
{
    while(!hard_clauses.empty()){
        std::forward_list<long> *clause = hard_clauses.front();
        while(!clause->empty()){
            std::cout << clause->front() << ' ';
            clause->pop_front();
        };
        std::cout << 0 << std::endl;
        hard_clauses.pop_front();
    };
}

int main()
{
    std::forward_list<std::forward_list<long>*> hard_clauses;
    std::forward_list<std::forward_list<long>*> soft_clauses;
    std::forward_list<std::forward_list<unsigned long>*> objectives;
    std::forward_list<unsigned long> objective_sizes;
    std::forward_list<std::pair<unsigned long, unsigned long>> sorted_vecs;
    
    unsigned long id_count;
    
    // read input problem.
    
    unsigned long num_vars, num_clauses;
    short num_objectives;
    std::cin >> num_vars >> num_clauses >> num_objectives;
    id_count = num_vars;
    unsigned long i = 0;
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
    
    short j = 0;
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
    
   
    //print_list(hard_clauses);

    
    // encode with odd even merge sorting network
    std::forward_list<std::forward_list<unsigned long>*>::iterator it1;
    std::forward_list<unsigned long>::iterator it2;
    it2 = objective_sizes.begin();
    it1 = objectives.begin();
    while(it1 != objectives.end()){
        unsigned long num_terms = *it2;
        std::forward_list<unsigned long> *objective = *it1;
        std::pair<unsigned long, unsigned long> sorted_vec (id_count + 1, id_count + num_terms);
        sorted_vecs.push_front(sorted_vec);
        id_count += num_terms;
        SNET sorting_network(num_terms);
        std::vector<unsigned long> elems_to_sort(objective->begin(),objective->end());
        encode_network(hard_clauses, elems_to_sort, &id_count, objective, sorting_network);
        ++it1;
        ++it2;
    };
    
    return 0;
}
