// hey hey hey!

// the sorting network is an array of stacks. Each stack contains pairs: the wire number, a variable of the output.
// sorting network: array of pointers to the class stack. Pairs are structs. The wire is an integer. The variable is a string

#include <string>

class Pair
{
public:
    unsigned long m_wire;
    std::string m_var;
    Pair * m_next;
    
    Pair(unsigned long wire, std::string var)
    {
       m_wire =  wire;
       m_var = var;
       m_next = nullptr;
    }
    
    void set_Next(Pair * next)
    {
        m_next = next;
    }
    
    
};    

class Stack
{
public:
    Pair * m_last;
    
    Stack(Pair * first)
    {
        m_last = first;
    }
    
    void push(Pair * p)
    {
        m_last.set_Next(p);
        m_last = p;
    }
    
    Pair * top()
    {
        return m_last;
    }
};

// first: read input package upgradeability formula and criteria.
// create sorting network and encoding. Then, destroy sorting network?
// 
