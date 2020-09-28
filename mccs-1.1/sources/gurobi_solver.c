
/*******************************************************/
/* CUDF solver: gurobi_solver.c                        */
/* Interface to the Gurobi solver                      */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <gurobi_solver.h>
#include <math.h>

#define OUTPUT_MODEL 1

// solver creation 
abstract_solver *new_gurobi_solver() { return new gurobi_solver(); }

// solver initialisation 
// requires the list of versioned packages and the total amount of variables (including additional ones)
int gurobi_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {
  int status;

  nb_packages = all_versioned_packages->size();
  this->all_versioned_packages = all_versioned_packages;

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  /* Initialize the gurobi environment */
  status = GRBloadenv(&env, NULL); // NULL => no file log

  if (status || (env == (GRBenv *)NULL)) {
    fprintf (stderr, "Could not create Gurobi environment.\n");
    exit(-1);
  }

  // Limit display according to verbosity
  if (verbosity < 2)  status = GRBsetintparam(env, "OutputFlag", 0);

  /* Set MIP gap to zero */
  status = GRBsetdblparam(env, "MIPGap", 0.0);

  /* Create Gurobi model */
  status = GRBnewmodel(env, &model, "mip1", 0, NULL, NULL, NULL, NULL, NULL);
  if (status || (model == (GRBmodel *)NULL)) {
    fprintf (stderr, "Could not create Gurobi model.\n");
    exit(-1);
  }

  /* Other data initialization */
  first_objective = 0;

  lb = (double *)malloc(nb_vars*sizeof(double));
  ub = (double *)malloc(nb_vars*sizeof(double));
  vartype = (char *)malloc(nb_vars*sizeof(char));
  varname = (char **)malloc(nb_vars*sizeof(char *));

  if ((lb  == (double *)NULL) ||
      (ub  == (double *)NULL) ||
      (vartype  == (char *)NULL) ||
      (varname  == (char **)NULL)) {
    fprintf(stderr, "gurobi_solver: initialization: not enough memory.\n");
    exit(-1);
  }

  // Set package variable names
  int i = 0;
  for (CUDFVersionedPackageListIterator ipkg = all_versioned_packages->begin(); ipkg != all_versioned_packages->end(); ipkg++) {
    lb[i] = 0; 
    ub[i] = 1;
    vartype[i] = GRB_BINARY;
    varname[(*ipkg)->rank] = (*ipkg)->versioned_name;
    i++;
  }

  // Set additional variable names
  for (i = nb_packages; i < nb_vars; i++) {
    char *name;
    char buffer[20];

    sprintf(buffer, "x%d", i);
    if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
      fprintf(stderr, "CUDF error: can not alloc memory for variable name in glpk_solver::end_objective.\n");
      exit(-1);
    }
    strcpy(name, buffer);

    lb[i] = 0; 
    ub[i] = 1;
    vartype[i] = GRB_BINARY;
    varname[i] = name;
  }

  return 0;
}

// cplex can handle integer variables
bool gurobi_solver::has_intvars() { return true; }

// set integer variable range (must be used before end_objective)
int gurobi_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) { 
  lb[rank] = lower; 
  ub[rank] = upper; 
  vartype[rank] = GRB_INTEGER;
  return 0; 
};

// Just write the lp problem in a file
int gurobi_solver::writelp(char *filename) { 
  return 0; 
}

// solve the current problem
int gurobi_solver::solve() {
  int nb_objectives = objectives.size();
  int mipstat, status;

  // Solve the objectives in a lexical order
  for (int i = first_objective; i < nb_objectives; i++) {
    // Solve the mip problem
    if (GRBoptimize(model)) return 0;
  
    // Get solution status
    status = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &mipstat);
    if (status) { exit(-1); }

    if (mipstat == GRB_OPTIMAL) {
      if (i < nb_objectives - 1) {
	// Get next non empty objective
	// (must be done here to avoid conflicting method calls
	int previ = i, nexti, nexti_nb_coeffs = 0;

	for (; i < nb_objectives - 1; i++) {
	  nexti = i + 1;
	  nexti_nb_coeffs = objectives[nexti]->nb_coeffs;
	  if (nexti_nb_coeffs > 0) break;
	}

	if (nexti_nb_coeffs > 0) { // there is one more objective to solve
	  // Set objective constraint value to objval
	  double objval = objective_value();
	  
	  if (verbosity > 0) printf(">>>> Objective value %d = %f\n", previ, objval);

	  if (GRBaddconstr(model, objectives[previ]->nb_coeffs, objectives[previ]->sindex, objectives[previ]->coefficients, GRB_EQUAL, objval, NULL)) {
	    fprintf(stderr, "gurobi_solver: end_objective: cannot add %d objective as constraint.\n", i); 
	    exit(-1); 
	  }

	  // Set the new objective value
	  reset_coeffs();

	  // Set previous objective coefficients to zero
	  for (int k = 0; k < objectives[previ]->nb_coeffs; k++) GRBsetdblattrelement(model, "Obj", objectives[previ]->sindex[k], 0);

	  // Set next objective coefficients to their actual values
	  for (int k = 0; k < nexti_nb_coeffs; k++)  GRBsetdblattrelement(model, "Obj", objectives[nexti]->sindex[k], objectives[nexti]->coefficients[k]);
	  
	  // Output model to file (when requested)
	  if (OUTPUT_MODEL) {
	  }
	} else
	  return 1;
      } else
	return 1;
    } else {
      fprintf(stderr, "CPLEX solution status = %d\n", mipstat);
      return 0;
    }
  }

  return 0;
}

// return the objective value
CUDFcoefficient gurobi_solver::objective_value() { 
  double objval;
  int  status = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
  if (status) {
    fprintf (stderr,"No MIP objective value available.  Exiting...\n");
    exit(-1);
  }
  //  printf("Objective value = % 24.24e\n", objval);
  return (CUDFcoefficient)nearbyint(objval); 
}

// solution initialisation
int gurobi_solver::init_solutions() {
  int status;

  if (solution != (double *)NULL) free(solution);

  if ((solution = (double *)malloc(nb_vars*sizeof(double))) == (double *)NULL) {
    fprintf (stderr, "gurobi_solver: init_solutions: cannot get enough memory to store solutions.\n");
    exit(-1);
  }

   status = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, nb_vars, solution);
   if ( status ) {
      fprintf (stderr, "gurobi_solver: init_solutions: failed to get solutions.\n");
      exit(-1);
   }
  return 0; 
}

// get the computed status of a package (0 = uninstalled, 1 = installed)
CUDFcoefficient gurobi_solver::get_solution(CUDFVersionedPackage *package) {  return (CUDFcoefficient)nearbyint(solution[package->rank]); }
CUDFcoefficient gurobi_solver::get_solution(int k) {  return (CUDFcoefficient)nearbyint(solution[k]); }

// initialize the objective function
int gurobi_solver::begin_objectives(void) { 
  return 0; 
}

// return the objective function coefficient of a package
CUDFcoefficient gurobi_solver::get_obj_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// return the objective function coefficient of a rank
CUDFcoefficient gurobi_solver::get_obj_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set the objective function coefficient of a package
int gurobi_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { set_coeff(package, value); return 0; }

// set the objective function coefficient of a ranked variable 
int gurobi_solver::set_obj_coeff(int rank, CUDFcoefficient value) { set_coeff(rank, value); return 0; };

// initialize an additional objective function 
int gurobi_solver::new_objective(void) {
  reset_coeffs();
  return 0;
}

// add an additional objective function
int gurobi_solver::add_objective(void) { 
  push_obj(); 
  return 0;
}

// ends up objective function construction
int gurobi_solver::end_objectives(void) {
  if (objectives.size() > 0) {
    int status = 0, nb_coeffs = 0; 

    // Set the first objective as the actual objective
    for (int k = 0; k < nb_vars; k++) coefficients[k] = 0;
    for (; first_objective < (int)objectives.size(); first_objective++) 
      if ((nb_coeffs = objectives[first_objective]->nb_coeffs) > 0) break;
    if (nb_coeffs > 0)
      for (int k = 0; k < nb_coeffs; k++) 
	coefficients[objectives[first_objective]->sindex[k]] = objectives[first_objective]->coefficients[k];
    else
      if (first_objective == (int)objectives.size()) first_objective--; // So that we solve at least one pbs
    status =  GRBaddvars(model, nb_vars, 0, NULL, NULL, NULL, coefficients, lb, ub, vartype, varname);
    if (status) {
      fprintf(stderr, "gurobi_solver: end_objective: cannot create objective function.\n");
      exit(-1);
    }
    status = GRBupdatemodel(model);
    if (status) {
      fprintf(stderr, "gurobi_solver: end_objective: cannot update model with variables.\n");
      exit(-1);
    }
  }

  return 0;
}

// initialize constraint declaration
int gurobi_solver::begin_add_constraints(void) { return 0; }

// begin the declaration of a new constraint
int gurobi_solver::new_constraint(void) { reset_coeffs(); return 0; }

// return the coefficient value of a package
CUDFcoefficient gurobi_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// return the coefficient value of a package
CUDFcoefficient gurobi_solver::get_constraint_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set the coeffcient value of a package
int gurobi_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { set_coeff(package, value); return 0; }

// set the coefficient value of a ranked variable
int gurobi_solver::set_constraint_coeff(int rank, CUDFcoefficient value) { set_coeff(rank, value); return 0; }

// add constraint under construction as a greater or equal constraint
int gurobi_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    if (GRBaddconstr(model, nb_coeffs, sindex, coefficients, GRB_GREATER_EQUAL, bound, NULL)) {
      fprintf(stderr, "gurobi_solver: add_constraint_geq: cannot create geq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// add constraint under construction as a less or equal constraint
int gurobi_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    if (GRBaddconstr(model, nb_coeffs, sindex, coefficients, GRB_LESS_EQUAL, bound, NULL)) {
      fprintf(stderr, "gurobi_solver: add_constraint_geq: cannot create geq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// add constraint under construction as an equal constraint
int gurobi_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    if (GRBaddconstr(model, nb_coeffs, sindex, coefficients, GRB_EQUAL, bound, NULL)) {
      fprintf(stderr, "gurobi_solver: add_constraint_geq: cannot create geq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// ends up constraint declaration
int gurobi_solver::end_add_constraints(void) { 
  return 0; 
}
