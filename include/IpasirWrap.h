/*
 * File:  ipasir_wrap.hh
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Modified by: Miguel Cabral
 * Copyright (C) 2019, Mikolas Janota
 */
#ifndef LEXIMAXIST_IPASIRWRAP
#define LEXIMAXIST_IPASIRWRAP

#include <leximaxIST_types.h>
#include <vector>

namespace leximaxIST {
    class IpasirWrap {
    public:
        IpasirWrap(int nvars);
        virtual ~IpasirWrap();
        void addClause(const Clause *clause);
        bool addClause(int p);
        bool addClause(int p, int q);
        bool addClause(int p, int q, int r);
        int fresh();
        bool is_ok_var(int v);
        bool is_ok_lit(int l);
        // return a non-const reference to be able to steal _model's data with move semantics
        std::vector<int>& model();
        const std::vector<int>& conflict() const;

        int nVars() const;

        bool solve(const std::vector<int>& assumps);
        bool solve();
        
    private:
        //const int           _verb = 1;
        int                _nvars;
        void*              _s;
        std::vector<int> _model;
        std::vector<int> _conflict;

        void add(int p);

        bool f();
    };

} /* namespace leximaxIST */
#endif /* LEXIMAXIST_IPASIRWRAP */
