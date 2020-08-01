#include <vector>
#include <string>
#include <iostream> // std::cout, std::cin

int main(int argc, char *argv[])
{
    // this code checks if the output of the SAT solver gives a sorted vector
    int num_vars = std::stoi(argv[1]);
    // store variables of objective function
    std::vector<int> obj(num_vars, 0);
    std::string in("hello");
    while(in.compare("v") != 0){
        std::cin >> in;
    }
    // ignore first variables
    for(int i = 0; i < num_vars; ++i){
        std::cin >> in;
    }
    for(int i = 0; i < num_vars; ++i){
        std::cin >> in;
        obj[i] = std::stoi(in);
    }
    // store sorted vector
    std::vector<int> sorted_vec(num_vars, 0);
    for(int i = 0; i < num_vars; ++i){
        std::cin >> in;
        sorted_vec[i] = std::stoi(in);
    }
    
    /*
    // check obj and vec
    for(int i = 0; i < num_vars; ++i){
        std::cout << obj[i] << std::endl;
    }
    for(int i = 0; i < num_vars; ++i){
        std::cout << sorted_vec[i] << std::endl;
    }
    */
    
    // check if the vector is in increasing order: if I see a 1 then I can not see a 0 again.
    bool saw_one = false;
    for(int i = 0; i < num_vars; ++i){
        if(saw_one && sorted_vec[i] < 0){
            std::cout << "Problem: vector is not sorted!\n";
            return 1;
        }
        else if(sorted_vec[i] > 0)
            saw_one = true;
    }
    // check if number of 0s is the same
    int num_zeros_obj = 0;
    int num_zeros_sorted = 0;
    for(int i = 0; i < num_vars; ++i){
        if(obj[i] < 0)
            num_zeros_obj++;
        if(sorted_vec[i] < 0)
            num_zeros_sorted++;
    }
    if(num_zeros_obj != num_zeros_sorted){
        std::cout << "Problem: vector is not sorted!\n";
        return 1;
    }
    
    std::cout << "OK\n";
    return 0;
}
