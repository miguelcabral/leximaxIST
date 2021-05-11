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
        IpasirWrap();
        virtual ~IpasirWrap();
        void addClause(const Clause *clause);
        void addClause(int p);
        void addClause(int p, int q);
        void addClause(int p, int q, int r);
        int fresh();
        bool is_ok_var(int v);
        bool is_ok_lit(int l);
        // return a non-const reference to be able to steal _model's data with move semantics
        std::vector<int>& model();
        const std::vector<int>& conflict() const;

        /* set a timeout (in seconds) counting from when this function is called
         * set also the time at which this function is called
         */
        void set_timeout(double t);
        
        int nVars() const;

        /* Returns the return value of the ipasir function:
         * 10 if SAT
         * 20 if UNSAT
         * 0 if interrupted
         */
        int solve(const std::vector<int>& assumps);
        int solve();
        
    private:
        //const int           _verb = 1;
        int                _nvars;
        void*              _s;
        std::vector<int> _model;
        std::vector<int> _conflict;
        struct TimeParams _time_params;

        void add(int p);

        void f();
    };

} /* namespace leximaxIST */
#endif /* LEXIMAXIST_IPASIRWRAP */
