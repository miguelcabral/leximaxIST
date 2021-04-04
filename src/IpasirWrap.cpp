#include <leximaxIST_error.h>
#include <IpasirWrap.h>
#include <ipasir.h>
#include <cstdlib>
#include <cmath>

namespace leximaxIST {
    
    IpasirWrap::IpasirWrap() : _nvars(0) { _s = ipasir_init(); }
    
    IpasirWrap::~IpasirWrap() { ipasir_release(_s); }
    
    void IpasirWrap::addClause(const Clause *clause)  {
        if (clause == nullptr) {
            print_error_msg("In IpasirWrap, nullptr in addClause()");
            exit(EXIT_FAILURE);
        }
        for (int literal : *clause)
            add(literal);
        f();
    }
    
    void IpasirWrap::addClause(int p) {  add(p); f(); }
    
    void IpasirWrap::addClause(int p, int q) {  add(p); add(q); f(); }
    
    void IpasirWrap::addClause(int p, int q, int r) {  add(p); add(q);  add(r); f(); }
    
    int IpasirWrap::fresh() { return ++_nvars; }
    
    bool IpasirWrap::is_ok_var(int v) { return 1 <= _nvars && v <= _nvars; }
    
    bool IpasirWrap::is_ok_lit(int l) { return is_ok_var(std::abs(l)); }
    
    // return a non-const reference to be able to steal _model's data with move semantics
    std::vector<int>& IpasirWrap::model() { return _model; }
    
    const std::vector<int>& IpasirWrap::conflict() const { return _conflict; }

    int IpasirWrap::nVars() const {return _nvars;}
    
    bool IpasirWrap::solve() {
        std::vector<int> assumps;
        return solve(assumps);
    }

    bool IpasirWrap::solve(const std::vector<int>& assumps) {
        for (auto l : assumps)
            ipasir_assume(_s, l);

        const int r = ipasir_solve(_s);
        if (r != 10 && r != 20) {
            print_error_msg("Something went wrong with ipasir_solve call, retv: " + r);
            exit(EXIT_FAILURE);
        }
        _model.clear();
        _conflict.clear();
        if (r == 10) {
            _model.resize(_nvars + 1, 0);
            for (int v = _nvars; v; v--) {
            _model[v] = ipasir_val(_s, v);
            }
        } else {
            for (auto l : assumps) {
                if (ipasir_failed(_s, l))
                    _conflict.push_back(-l);
            }
        }
        return r == 10;
    }
    
    void IpasirWrap::add(int p) {
         if (std::abs(p) > _nvars)
             _nvars = p;
        ipasir_add(_s, p);
    }

    void IpasirWrap::f() {
        ipasir_add(_s, 0);
    }

}
