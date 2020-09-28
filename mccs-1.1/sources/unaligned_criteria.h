
/*******************************************************/
/* CUDF solver: unaligned_criteria.h                   */
/* Concrete class for the unaligned criteria           */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef _UNALIGNED_CRITERIA_H_
#define _UNALIGNED_CRITERIA_H_

#include <abstract_criteria.h>
#include <tr1/unordered_map>

// allowed type of alignments
#define ALIGNED_PACKAGES 1
#define ALIGNED_PAIRS    2
#define ALIGNED_CLUSTERS 3
#define ALIGNED_CHANGES  4

typedef map<int, CUDFVersionedPackage *> a_pkg_compil;
typedef a_pkg_compil::iterator a_pkg_compil_iterator;
typedef map<string, a_pkg_compil *> a_pkg_compil_set;
typedef a_pkg_compil_set::iterator a_pkg_compil_set_iterator;
typedef map<string, a_pkg_compil_set *> a_sourceversion_set;
typedef a_sourceversion_set::iterator a_sourceversion_set_iterator;
typedef tr1::unordered_map<string, a_sourceversion_set *> a_source_set;
typedef a_source_set::iterator a_source_set_iterator;

// A concrete class for the unaligned criteria
// i.e. number of unaligned packages, pairs of packages,
// clusters or changes with respect to a given source
class unaligned_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

  char *version_name;          // property that holds the full version as a string
  char *source_name;           // property that holds source package name
  char *sourceversion_name;    // property that holds source version as a string
  bool has_properties;         // are the properties available

  // set of sources
  a_source_set *source_set;
  int nb_sources;   // number of sources with more than one versionned package
  int nb_versions;  // total amount of versions among sources with more than one versionned package
  int nb_packages;  // total amount of versionned packages among sources with more than one versionned package
  int nb_pairs;

  int range;      // criteria range

  // column of the first variable used by the criteria
  int first_free_var;

  // Allocate some columns for the criteria
  int set_variable_range(int first_free_var);

  // Initialize integer variable ranges
  void initialize_intvars();

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

  // type of alignment
  int alignment;

  // temp
  void display_struct();

  // Criteria creation
  unaligned_criteria(int alignment, char *source_name, char *sourceversion_name, char *version_name) { 
    this->alignment = alignment;
    this->source_name = source_name;
    this->sourceversion_name = sourceversion_name;
    this->version_name = version_name;
    this->lambda_crit = +1; 
    nb_sources = nb_versions = nb_packages = nb_pairs = 0;
  };
  unaligned_criteria(int alignment, char *source_name, char *sourceversion_name, char *version_name, CUDFcoefficient lambda_crit) { 
    this->alignment = alignment;
    this->source_name = source_name;
    this->sourceversion_name = sourceversion_name;
    this->version_name = version_name;
    this->lambda_crit = lambda_crit; 
    nb_sources = nb_versions = nb_packages = nb_pairs = 0;
  };
};

#endif


