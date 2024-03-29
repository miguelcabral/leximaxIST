
/*******************************************************/
/* CUDF solver: cplex_solver.h                         */
/* Concrete class for the cplex solver                 */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


// concrete class which implements an interface to CPLEX solver

#ifndef _CPLEX_SOLVER_H
#define _CPLEX_SOLVER_H

#include <abstract_solver.h>
#include <scoeff_solver.h>
#include <ilcplex/cplex.h>

class cplex_solver: public abstract_solver, public scoeff_solver<double, 0, 0> {
 public:
  // Solver initialization
  int init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars);

  // Does the solver use integer variables
  bool has_intvars();
  // Allocate some columns for integer variables
  int set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper);

  // Init the objective function definitions
  int begin_objectives(void);
  // Get current objective coefficient of package 
  CUDFcoefficient get_obj_coeff(CUDFVersionedPackage *package);
  // Get current objective coefficient of a column
  CUDFcoefficient get_obj_coeff(int rank);
  // Set current objective coefficient of package 
  int set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value);
  // Set current objective coefficient of column
  int set_obj_coeff(int rank, CUDFcoefficient value);
  // Begin the definition of a new objective
  int new_objective(void);
  // Add current objective to the set of objectives
  int add_objective(void);
  // End objective definitions
  int end_objectives(void);

  // Init constraint definitions
  int begin_add_constraints(void);
  // Begin the definition of a new constraint
  int new_constraint(void);
  // Get current constraint coefficient of a package
  CUDFcoefficient get_constraint_coeff(CUDFVersionedPackage *package);
  // Get current constraint coefficient of a column
  CUDFcoefficient get_constraint_coeff(int rank);
  // Set current constraint coefficient of a package
  int set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value);
  // Set current constraint coefficient of a column
  int set_constraint_coeff(int rank, CUDFcoefficient value);
  // Add current constraint as a more or equal constraint
  int add_constraint_geq(CUDFcoefficient bound);
  // Add current constraint as a less or equal constraint
  int add_constraint_leq(CUDFcoefficient bound);
  // Add current constraint as a equality constraint
  int add_constraint_eq(CUDFcoefficient bound);
  // End constraint definitions
  int end_add_constraints(void);

  // Write the lp on a file
  int writelp(char *filename);

  void set_cplex_timeout(double tout);
  
  // Solve the problem
  int solve();
  // Get the objective value (final one)
  CUDFcoefficient objective_value();
  // Init solutions (required before calling get_solution)
  int init_solutions();
  // Get the solution for a package
  CUDFcoefficient get_solution(CUDFVersionedPackage *package);
  // Get the solution for a column
  CUDFcoefficient get_solution(int k);

  // variables only for internal use (should be private)
  CPXENVptr env;   // cplex environment
  CPXLPptr lp;     // cplex linear program
  CUDFVersionedPackageList *all_versioned_packages; // a pointer to the list of versioned packages

  int nb_packages;     // number of packages

  int first_objective;

  double *cplex_coeff; // cplex coefficients are doubles ...
  double *lb;          // array of lower bounds
  double *ub;          // array of upper bounds
  char *vartype;       // array of variable types
  char **varname;      // array of variable names

  // Store the solutions
  double *solution;

  // solver creation
  cplex_solver(void) {
    all_versioned_packages = (CUDFVersionedPackageList *)NULL;
    solution = (double *)NULL;
  }
};

#endif
