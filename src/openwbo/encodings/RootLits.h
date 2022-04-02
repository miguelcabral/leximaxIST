#ifndef ROOTLITS_h
#define ROOTLITS_h

#include "Enc_KPA.h"
#include <iterator>
#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#include <memory>
#endif

#include <vector>

using NSPACE::Lit;
using namespace std;
using namespace openwbo;

namespace rootLits{

  class RootLitsGeq;
  class RootLits: public vector<pair<uint64_t, Lit>>{
  public: 
    using myType = vector<pair<uint64_t, Lit>>;
    using value_t =pair<uint64_t, Lit>; 
    //My encoding. Iterates through the variables as if they encoded
    //my encoding. That is, the pair returned corresponds to f \geq u
    RootLitsGeq as_geq();
  };

  class RootLitsGeq{
    struct GeqIterator;
  public:
    using myType = RootLits::myType;
    using value_t =RootLits::value_t;
    RootLitsGeq(RootLits& rl):base{make_shared<RootLits>(rl)}{}
    GeqIterator begin(){return GeqIterator{base->begin()};}
    GeqIterator end(){return GeqIterator{base->end()};}

  private:
    struct GeqIterator {
    public:
      using iterator_category = std::input_iterator_tag;
      using difference_type   = myType::const_iterator::difference_type;
      using value_type        = myType::const_iterator::value_type;
      using pointer           = myType::const_iterator;

      GeqIterator(myType::iterator it) : m_ptr{it} {}
      value_type operator*() const { return {(*m_ptr).first, ~((*m_ptr).second)}; }
      pointer operator->() { return m_ptr; }
      GeqIterator& operator++() { m_ptr++; return *this; }  
      GeqIterator operator++(int) { GeqIterator tmp = *this; ++(*this); return tmp; }
      friend bool operator== (const GeqIterator& a, const GeqIterator& b) { return a.m_ptr == b.m_ptr; };
      friend bool operator!= (const GeqIterator& a, const GeqIterator& b) { return a.m_ptr != b.m_ptr; };  
    private:
      pointer m_ptr;
    };

  private:
    shared_ptr<RootLits> base;

  };
}


#endif
