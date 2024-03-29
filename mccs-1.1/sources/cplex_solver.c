
/*******************************************************/
/* CUDF solver: cplex_solver.c                         */
/* Interface to the CPLEX solver                       */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cplex_solver.h>
#include <math.h>
#include <rusage.h>
#include <iostream>

#define OUTPUT_MODEL 1
#define USEXNAME 0

// solver creation 
abstract_solver *new_cplex_solver() { return new cplex_solver(); }

// solver initialisation 
// requires the list of versioned packages and the total amount of variables (including additional ones)
int cplex_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {
  int status;

  nb_packages = all_versioned_packages->size();
  this->all_versioned_packages = all_versioned_packages;

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  /* Initialize the CPLEX environment */
  env = CPXopenCPLEX (&status);
  if ( env == NULL ) {
    char  errmsg[1024];
    fprintf (stderr, "Could not open CPLEX environment.\n");
    CPXgeterrorstring (env, status, errmsg);
    fprintf (stderr, "%s", errmsg);
    exit(-1);
  }

  
   /* Enhance EPGAP to handle big values correctly */
  status = CPXsetdblparam (env, CPX_PARAM_EPGAP, 0.0);
  if ( status ) {
    fprintf (stderr, "Failure to set EPGAP, error %d.\n", status);
    exit(-1);
  }
  /* Edit: Miguel - use EPAGAP */
  /*status = CPXsetdblparam (env, CPXPARAM_MIP_Tolerances_AbsMIPGap, 0.0);
  if ( status ) {
    fprintf (stderr, "Failure to set EPAGAP, error %d.\n", status);
    exit(-1);
  }
  status = CPXsetdblparam (env, CPXPARAM_MIP_Tolerances_Integrality, 0.0);
  if ( status ) {
    fprintf (stderr, "Failure to set Integrality tolerance, error %d.\n", status);
    exit(-1);
  }*/
  // End of edit: Miguel
  /* Limit the number of threads to 1 */  
  status = CPXsetintparam (env, CPXPARAM_Threads, 1);
  if ( status ) {
    fprintf (stderr, "Failure to set thread limit to 1, error %d.\n", status);
    exit(-1);
  }
  // Miguel edit:
  /*status = CPXsetintparam (env, CPXPARAM_Emphasis_Numerical, 1);
  if ( status ) {
    fprintf (stderr, "Failure to set numerical emphasis, error %d.\n", status);
    exit(-1);
  }*/
  // end of Miguel edit
  // set time limit as CPU time (not as Wall clock time - default)
  status = CPXsetintparam (env, CPX_PARAM_CLOCKTYPE, 1);
  if ( status ) {
    fprintf (stderr, "Failure to set CPX_PARAM_CLOCKTYPE to 1, error %d.\n", status);
    exit(-1);
  }
  

  if (verbosity > 1) {
    /* Turn on output to the screen */
    status = CPXsetintparam (env, CPX_PARAM_SCRIND, CPX_ON);
    if ( status ) {
      fprintf (stderr, "Failure to turn on screen indicator, error %d.\n", status);
      exit(-1);
    }

    if (verbosity > 2) {
      /* MIP node log display information */
      status = CPXsetintparam (env, CPX_PARAM_MIPDISPLAY, 5);
      if ( status ) {
	fprintf (stderr, "Failure to turn off presolve, error %d.\n", status);
	exit(-1);
      }
    }
  }

  /* Create the problem. */
  lp = CPXcreateprob (env, &status, "lpex1");
  
  /* A returned pointer of NULL may mean that not enough memory
     was available or there was some other problem.  In the case of
     failure, an error message will have been written to the error
     channel from inside CPLEX.  In this example, the setting of
     the parameter CPX_PARAM_SCRIND causes the error message to
     appear on stdout.  */
  
  if ( lp == NULL ) {
    fprintf (stderr, "Failed to create LP.\n");
    exit(-1);
  }

  first_objective = 0;

  lb = (double *)malloc(nb_vars*sizeof(double));
  ub = (double *)malloc(nb_vars*sizeof(double));
  vartype = (char *)malloc(nb_vars*sizeof(char));
  varname = (char **)malloc(nb_vars*sizeof(char *));

  if ((lb  == (double *)NULL) ||
      (ub  == (double *)NULL) ||
      (vartype  == (char *)NULL) ||
      (varname  == (char **)NULL)) {
    fprintf(stderr, "cplex_solver: initialization: not enough memory.\n");
    exit(-1);
  }

  // Set package variable names
  int i = 0;
  for (CUDFVersionedPackageListIterator ipkg = all_versioned_packages->begin(); ipkg != all_versioned_packages->end(); ipkg++) {
    lb[i] = 0; 
    ub[i] = 1;
    vartype[i] = 'B';
    if (USEXNAME) {
      char *name;
      char buffer[20];

      sprintf(buffer, "x%d", i);
      if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
	fprintf(stderr, "CUDF error: can not alloc memory for variable name in cplex_solver::end_objective.\n");
	exit(-1);
      }
      strcpy(name, buffer);
      varname[(*ipkg)->rank] = name;
    } else
      varname[(*ipkg)->rank] = (*ipkg)->versioned_name;
    i++;
  }

  // Set additional variable names
  for (i = nb_packages; i < nb_vars; i++) {
    char *name;
    char buffer[20];

    sprintf(buffer, "x%d", i);
    if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
      fprintf(stderr, "CUDF error: can not alloc memory for variable name in cplex_solver::end_objective.\n");
      exit(-1);
    }
    strcpy(name, buffer);

    lb[i] = 0; 
    ub[i] = 1;
    vartype[i] = 'B';
    varname[i] = name;
  }

  return 0;
}

// cplex can handle integer variables
bool cplex_solver::has_intvars() { return true; }

// set integer variable range (must be used before end_objective)
int cplex_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) { 
  lb[rank] = lower; 
  ub[rank] = upper; 
  vartype[rank] = 'I';
  return 0; 
};

// Just write the lp problem in a file
int cplex_solver::writelp(char *filename) { 
  return CPXwriteprob (env, lp, filename, NULL); 
}

void cplex_solver::set_cplex_timeout(double tout)
{
    // set time limit
    double cur_time (read_cpu_time());
    double time_left (tout - cur_time);
    if (time_left < 0.001)
        time_left = 0.001;
    const int status = CPXsetdblparam (env, CPX_PARAM_TILIM, time_left);
    if ( status ) {
        fprintf (stderr, "Failure to set CPX_PARAM_TILIM, error %d.\n", status);
        exit(-1);
    }
}

// solve the current problem
int cplex_solver::solve() {
  int nb_objectives = objectives.size();
  int mipstat, status;

  set_cplex_timeout(10.0);
  
  // Presolving the problem
  if (CPXpresolve(env, lp, CPX_ALG_NONE)) return 0;

  bool is_sat (false);
  // Solve the objectives in a lexical order
  for (int i = first_objective; i < nb_objectives; i++) {
      
    set_cplex_timeout(10.0);
      
    // Solve the mip problem
    if (CPXmipopt (env, lp)) return 0;
  
    // Get solution status
    if ((mipstat = CPXgetstat(env, lp)) == CPXMIP_OPTIMAL) {
    if (verbosity > 0) {
        double obj_val = objective_value();
	printf("# Objective value %d = %f\n", i, obj_val);
    }
    // get solution
    init_solutions();
    is_sat = true;
	  
	// Output model to file (when requested)
	  if (OUTPUT_MODEL) {
	    char buffer[1024];
	    sprintf(buffer, "cplexpbs%d.lp", i);
	    CPXwriteprob (env, lp, buffer, NULL);
	  }
    
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
	  int index[1];
	  double values[1];
	  
	  index[0] = previ;
	  values[0] = objective_value(); 

	  {
	    int status, begin[2];
	    double rhs[1];
	    
	    begin[0] = 0;
	    rhs[0] = values[0]; //ub;
	    int n = objectives[previ]->nb_coeffs;
	    begin[1] = n - 1;
	    status = CPXaddrows(env, lp, 0, 1, n, rhs, "E", begin, objectives[previ]->sindex, objectives[previ]->coefficients, NULL, NULL);
	    if (status) { 
	      fprintf(stderr, "cplex_solver: end_objective: cannot add %d objective as constraint.\n", i); 
	      exit(-1); 
	    }
	  }

	  // Set the new objective value
	  reset_coeffs();

	  // Set previous objective coefficients to zero
	  for (int k = 0; k < objectives[previ]->nb_coeffs; k++) set_coeff(objectives[previ]->sindex[k], 0);

	  // Set next objective coefficients to their actual values
	  for (int k = 0; k < nexti_nb_coeffs; k++) set_coeff(objectives[nexti]->sindex[k], objectives[nexti]->coefficients[k]);
	  // Do set the next objective 
	  status = CPXchgobj(env, lp, nb_coeffs, sindex, coefficients);
	  if ( status ) {
	    fprintf (stderr,"Cannot change objective value.  Exiting...\n");
	    exit(-1);
	  }
	  
	} else
	  return 1;
      } else
		return 1;
    } 
    else if (mipstat == CPXMIP_TIME_LIM_FEAS) {
        fprintf(stdout, "# CPLEX reached the time limit with a solution\n");
        init_solutions();
        return 1;
    }
    else if (mipstat == CPXMIP_TIME_LIM_INFEAS) {
        fprintf(stdout, "# CPLEX reached the time limit without a solution\n");
        if (is_sat)
            return 1;
        else
            return 0;
    }
    else {
      if (verbosity > 2)
	fprintf(stderr, "CPLEX solution status = %d\n", mipstat);
      return 0;
    }
  }

  return 0;
}

// return the objective value
CUDFcoefficient cplex_solver::objective_value() { 
  double objval;
  int  status = CPXgetobjval (env, lp, &objval);
  if (status) {
    fprintf (stderr,"No MIP objective value available.  Exiting...\n");
    exit(-1);
  }
  //  printf("Objective value = % 24.24e\n", objval);
  return (CUDFcoefficient)nearbyint(objval); 
}

// solution initialisation
int cplex_solver::init_solutions() {
  int status;
  int cur_numcols = CPXgetnumcols (env, lp);

  if (solution != (double *)NULL) free(solution);

  if ((solution = (double *)malloc(nb_vars*sizeof(double))) == (double *)NULL) {
    fprintf (stderr, "cplex_solver: init_solutions: cannot get enough memory to store solutions.\n");
    exit(-1);
  }

   status = CPXgetx (env, lp, solution, 0, cur_numcols-1);
   if ( status ) {
      fprintf (stderr, "cplex_solver: init_solutions: failed to get solutions.\n");
      exit(-1);
   }
  return 0; 
}

// get the computed status of a package (0 = uninstalled, 1 = installed)
CUDFcoefficient cplex_solver::get_solution(CUDFVersionedPackage *package) {  return (CUDFcoefficient)nearbyint(solution[package->rank]); }
CUDFcoefficient cplex_solver::get_solution(int k) {  return (CUDFcoefficient)nearbyint(solution[k]); }

// initialize the objective function
int cplex_solver::begin_objectives(void) { 
  // Set Problem as a minimization problem
  CPXchgobjsen (env, lp, CPX_MIN);

  return 0; 
}

// return the objective function coefficient of a package
CUDFcoefficient cplex_solver::get_obj_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// return the objective function coefficient of a rank
CUDFcoefficient cplex_solver::get_obj_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set the objective function coefficient of a package
int cplex_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { set_coeff(package, value); return 0; }

// set the objective function coefficient of a ranked variable 
int cplex_solver::set_obj_coeff(int rank, CUDFcoefficient value) { set_coeff(rank, value); return 0; };

// initialize an additional objective function 
int cplex_solver::new_objective(void) {
  reset_coeffs();
  return 0;
}

// add an additional objective function
int cplex_solver::add_objective(void) { 
  push_obj(); 
  return 0;
}

// ends up objective function construction
int cplex_solver::end_objectives(void) {
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
    status = CPXnewcols (env, lp, nb_vars, coefficients, lb, ub, vartype, varname);
    if (status) {
      fprintf(stderr, "cplex_solver: end_objective: cannot create objective function.\n");
      exit(-1);
    }
  }

  return 0;
}

// initialize constraint declaration
int cplex_solver::begin_add_constraints(void) { return 0; }

// begin the declaration of a new constraint
int cplex_solver::new_constraint(void) { reset_coeffs(); return 0; }

// return the coefficient value of a package
CUDFcoefficient cplex_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// return the coefficient value of a package
CUDFcoefficient cplex_solver::get_constraint_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set the coeffcient value of a package
int cplex_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { set_coeff(package, value); return 0; }

// set the coefficient value of a ranked variable
int cplex_solver::set_constraint_coeff(int rank, CUDFcoefficient value) { set_coeff(rank, value); return 0; }

// add constraint under construction as a greater or equal constraint
int cplex_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    int status, begin[2];
    double rhs[1];

    begin[0] = 0;
    begin[1] = nb_coeffs;
    rhs[0] = bound;

    status = CPXaddrows(env, lp, 0, 1, nb_coeffs, rhs, "G", begin, sindex, coefficients, NULL, NULL);
    if (status) { 
      fprintf(stderr, "cplex_solver: add_constraint_geq: cannot create geq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// add constraint under construction as a less or equal constraint
int cplex_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    int status, begin[2];
    double rhs[1];

    begin[0] = 0;
    begin[1] = nb_coeffs;
    rhs[0] = bound;
    status = CPXaddrows(env, lp, 0, 1, nb_coeffs, rhs, "L", begin, sindex, coefficients, NULL, NULL);
    if (status) { 
      fprintf(stderr, "cplex_solver: add_constraint_leq: cannot create leq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// add constraint under construction as an equal constraint
int cplex_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    int status, begin[2];
    double rhs[1];

    begin[0] = 0;
    begin[1] = nb_coeffs;
    rhs[0] = bound;
    status = CPXaddrows(env, lp, 0, 1, nb_coeffs, rhs, "E", begin, sindex, coefficients, NULL, NULL);
    if (status) { 
      fprintf(stderr, "cplex_solver: add_constraint_eq: cannot create eq constraint.\n"); 
      exit(-1); 
    }
  }
  return 0;
}

// ends up constraint declaration
int cplex_solver::end_add_constraints(void) { 
  //if (OUTPUT_MODEL) CPXwriteprob (env, lp, "cplexpbs.lp", NULL); 
  return 0; 
}
