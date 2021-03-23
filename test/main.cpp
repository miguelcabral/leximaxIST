


int main()
{
    
    // TODO: test all formalisms; test lp solvers
    // test signal handling
    // test simplify_last
    // test upper bound encoding
    // test different number of objective functions
    // test empty constraints and empty obj functions
    // test empty clauses
    if (basic_test() != 0) {
        std::cerr << "Basic test"
    }
    return 0;
}
