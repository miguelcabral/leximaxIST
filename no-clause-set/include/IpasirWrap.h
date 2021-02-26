/*
 * File:  ipasir_wrap.hh
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Modified by: Miguel Cabral
 * Copyright (C) 2019, Mikolas Janota
 */
#ifndef LEXIMAXIST_IPASIRWRAP
#define LEXIMAXIST_IPASIRWRAP
#include <leximaxIST_error.h>
#include <leximaxIST_types.h>
#include"ipasir.h"
#include<iostream>
#include<cstdlib>
#include<cmath>
#include<vector>

namespace leximaxIST {
    class IpasirWrap {
    public:
        IpasirWrap(int nvars) : _nvars(nvars) { _s = ipasir_init(); }

        virtual ~IpasirWrap() { ipasir_release(_s); }
        void addClause(const Clause *clause)  {
            if (clause == nullptr) {
                print_error_msg("In IpasirWrap, nullptr in addClause()");
                exit(EXIT_FAILURE);
            }
            for (int literal : *clause)
                add(literal);
            f();
        }
        bool addClause(int p) {  add(p); return f(); }
        bool addClause(int p, int q) {  add(p); add(q); return f(); }
        bool addClause(int p, int q, int r) {  add(p); add(q);  add(r); return f(); }
        inline int fresh() { return ++_nvars; }
        inline bool is_ok_var(int v) { return 1 <= _nvars && v <= _nvars; }
        inline bool is_ok_lit(int l) { return is_ok_var(std::abs(l)); }
        const std::vector<int>& model() const { return _model; }
        const std::vector<int>& conflict() const { return _conflict; }

        int nVars() const {return _nvars;}

        bool solve(const std::vector<int>& assumps);
        inline bool solve() {
            std::vector<int> assumps;
            return solve(assumps);
        }
    private:
        //const int           _verb = 1;
        int                _nvars;
        void*              _s;
        std::vector<int> _model;
        std::vector<int> _conflict;

        inline void add(int p) {
            is_ok_lit(p);
            ipasir_add(_s, p);
        }

        inline bool f() {
            ipasir_add(_s, 0);
            return true;
        }
};

inline bool IpasirWrap::solve(const std::vector<int>& assumps) {
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
} /* namespace leximaxIST */
#endif /* LEXIMAXIST_IPASIRWRAP */
