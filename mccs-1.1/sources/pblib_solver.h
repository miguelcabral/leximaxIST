
/*******************************************************/
/* CUDF solver: pblib_solver.h                         */
/* Concrete class for pblib format based solvers       */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// concrete class which implements an interface to a pblib compliant solver

#ifndef _PBLIB_SOLVER_H
#define _PBLIB_SOLVER_H

#include <abstract_solver.h>
#include <scoeff_solver.h>

class pblib_solver: public abstract_solver, public scoeff_solver<CUDFcoefficient, 1, 0> {
 public:
  // Solver initialization
  int init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars);
  // Write the lp on a file
  int writelp(char *filename);

  // Solve the problem
  int solve();
  // Get the objective value (final one)
  CUDFcoefficient objective_value();
  // Init solutions (required before calling get_solution)
  int init_solutions();
  // Get the solution for a package
  CUDFcoefficient get_solution(CUDFVersionedPackage *package);

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

  CUDFVersionedPackageList *all_versioned_packages;  // list of all versioned packages
  int nb_packages; // number of packages

  int nb_constraints; // number of constraints
  char mult;

  CUDFcoefficient *solution; // array of solution values
  CUDFcoefficient objval;    // objective value

  char ctpbfilename[256];
  char pbfilename[256];
  char pboutfilename[256];
  FILE *pbfile, *ctpbfile;

  char *pb_solver;  // name of the solver to call

  // solver creation
  pblib_solver(char *pb_solver) {
    this->pb_solver = pb_solver;
    nb_packages = 0;
    all_versioned_packages = (CUDFVersionedPackageList *)NULL;
    nb_constraints = 0;
    solution = (CUDFcoefficient *)NULL;
    pbfile = stdout;
    mult = ' ';
  }
};

#endif
