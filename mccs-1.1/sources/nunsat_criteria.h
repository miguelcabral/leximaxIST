
/*******************************************************/
/* CUDF solver: nunsat_criteria.h                      */
/* Concrete class for the nunsat criteria              */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef _NUNSAT_CRITERIA_H_
#define _NUNSAT_CRITERIA_H_

#include <abstract_criteria.h>

// A concrete class for the nunsat criteria
// i.e. number of disjuncts from a given property
// of installed virtual packages which are unsat
// in the final configuration
class nunsat_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

  char *property_name;       // property name
  bool has_property;         // is the property available
  // list of versionned virtual packages
  CUDFVersionedPackageList *versioned_pkg_with_property;

  int disjuncts;  // number of disjunct
  int range;      // criteria range

  // column of the first variable used by the criteria
  int first_free_var;

  // Allocate some columns for the criteria
  int set_variable_range(int first_free_var);

  // Add the criteria to the objective
  int add_criteria_to_objective(CUDFcoefficient lambda);
  // Add the criteria to the constraint set
  int add_criteria_to_constraint(CUDFcoefficient lambda);
  // Add constraints required by the criteria
  int add_constraints();

  // Compute the criteria range, upper and lower bounds
  CUDFcoefficient bound_range();
  CUDFcoefficient upper_bound();
  CUDFcoefficient lower_bound();

  // Does the criteria allows problem reductions
  bool can_reduce(CUDFcoefficient lambda) { return ((lambda >= 0) && (lambda_crit >= 0)); }

  // Criteria initialization
  void initialize(CUDFproblem *problem, abstract_solver *solver); 

  void check_property(CUDFproblem *problem);

  // lambda multiplier for the criteria
  CUDFcoefficient lambda_crit ;

  // shall providers be taken into account
  bool with_providers;

  // Criteria initialization
  nunsat_criteria(char *property_name, bool with_providers) { 
    this->property_name = property_name;
    this->lambda_crit = +1; 
    this->with_providers = with_providers;
  };
  nunsat_criteria(char *property_name, bool with_providers, CUDFcoefficient lambda_crit) { 
    this->property_name = property_name;
    this->lambda_crit = lambda_crit; 
    this->with_providers = with_providers;
  };
};

#endif


