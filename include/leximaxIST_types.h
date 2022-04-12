#ifndef LEXIMAXIST_TYPES
#define LEXIMAXIST_TYPES
#include <vector>
#include <utility>

namespace leximaxIST {

    typedef int Lit; // definition of Lit for leximaxIST (encodings from PB to CNF of openwbo)
    constexpr int lit_Undef (0);
    constexpr int var_Undef (0);
    constexpr int lit_Error (0); // all these constants have the same value, I hope this is not an issue
    constexpr int max_clauses (3000000);    
    enum { _FORMAT_MAXSAT_ = 0, _FORMAT_PB_ };
    enum { _UNWEIGHTED_ = 0, _WEIGHTED_ };
    enum StatusCode {
    _SATISFIABLE_ = 10,
    _UNSATISFIABLE_ = 20,
    _OPTIMUM_ = 30,
    _UNKNOWN_ = 40,
    _ERROR_ = 50,
    _MEMOUT_ = 60
    };
    enum {
    _INCREMENTAL_NONE_ = 0,
    _INCREMENTAL_BLOCKING_,
    _INCREMENTAL_WEAKENING_,
    _INCREMENTAL_ITERATIVE_
    };
    enum { _CARD_CNETWORKS_ = 0, _CARD_TOTALIZER_, _CARD_MTOTALIZER_ };
    enum { _AMO_LADDER_ = 0 };
    enum { _PB_SWC_ = 0, _PB_GTE_, _PB_ADDER_, _PB_IGTE_, _PB_KP_, _PB_KP_MINISATP_};
    enum { _PART_SEQUENTIAL_ = 0, _PART_SEQUENTIAL_SORTED_, _PART_BINARY_ };
    
    // sorting network and clauses:
    typedef std::vector<std::pair<int, int>> SNET;
    typedef std::vector<int> Clause;
}
#endif /* LEXIMAXIST_TYPES */
