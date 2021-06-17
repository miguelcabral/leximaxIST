
/*******************************************************/
/* CUDF solver: lp_solver.c                            */
/* Interface to the lp format solvers                  */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <lp_solver.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <rusage.h>
#include <string>

#define CLEAN_FILES 1
#define TMP_FILES_PATH "/tmp/"

// external function for solver creation
abstract_solver *new_lp_solver(char *lpsolver) { return new lp_solver(lpsolver); }

// solver initialisation
int lp_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {

  nb_packages = all_versioned_packages->size();
  this->all_versioned_packages = all_versioned_packages;

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  nb_constraints = 0;
  mult = ' ';

  solution = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));
  lb = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));
  ub = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));

  for (int i = 0; i < nb_vars; i++) { lb[i] = 0; ub[i] = 1; }

  sprintf(ctlpfilename, TMP_FILES_PATH "ctlp_%lu_%lu.lp", (long unsigned)getuid(), (long unsigned)getpid()); 
  ctlpfile = fopen(ctlpfilename, "w");

  if ((solution == (CUDFcoefficient *)NULL) ||
      (lb == (CUDFcoefficient *)NULL) ||
      (ub == (CUDFcoefficient *)NULL)) {
    fprintf(stderr, "lp_solver: intialize: not enough memory.\n");
    exit(-1);
  } else if (ctlpfile == (FILE *)NULL) {
    fprintf(stderr, "lp_solver: intialize: can not open %s.\n", ctlpfilename);
    exit(-1);
  } else
    return 0;
}

// write the problem into a file
int lp_solver::writelp(char *filename) { return 0; }

// change settings file of scip to set a time limit
void set_scip_timeout(double tout)
{
    double cur_time (read_cpu_time());
    double time_left (tout - cur_time);
    if (time_left < 0.001)
        time_left = 0.001;
    // copy the settings file to the mccs directory and identify it with this pid
    std::string command ("cp /home/mcabral/solvers/lp/scipoptsuite-7.0.1/scip/scip.set ");
    const std::string filename ("/home/mcabral/thesis/mccs-1.1/" + std::to_string(getpid()) + "_scip.txt");
    command += filename;
    if (system(command.c_str()) == -1) { // execute cp command
      fprintf(stderr, "mccs: error while copying scip settings.\n");
      exit(-1);
    }
    command = "sed -i \"s/limits\\/time.*/limits\\/time = ";
    command += std::to_string(time_left);
    command += "/g\" /home/mcabral/solvers/lp/scipoptsuite-7.0.1/scip/scip.set";
    if (system(command.c_str()) == -1) { // execute sed command
      fprintf(stderr, "mccs: error while changing scip settings.\n");
      exit(-1);
    }
}

// solve the current problem
int lp_solver::solve() {
  int status = 0;
  int rank, iobjval;
  char command[1024];
  FILE *fsol = (FILE *)NULL;
  CUDFcoefficient objvals[20];
  unsigned int nb_objectives = objectives.size();

  sprintf(lpfilename, TMP_FILES_PATH "lppbs_%lu_%lu.lp", (long unsigned)getuid(), (long unsigned)getpid()); 
  sprintf(lpoutfilename, TMP_FILES_PATH "lppbs_%lu_%lu.out", (long unsigned)getuid(), (long unsigned)getpid()); 

  for (unsigned int iobj = 0; iobj < nb_objectives; iobj++) {
    if (objectives[iobj]->nb_coeffs == 0) continue;
 
    if ((lpfile = fopen(lpfilename, "w")) == (FILE *)NULL) {
      fprintf(stderr, "lp_solver: cannot open %s.\n", lpfilename);
      exit(-1);
    }
    
    fprintf(lpfile, "Minimize\n obj:");
    for (int i = 0, nbc = 0; i < objectives[iobj]->nb_coeffs; i++, nbc++) {
      if (nbc == 20) { // scip does not like too long lines
	nbc = 0;
	fprintf(lpfile, "\n  ");
      }
      fprintf(lpfile, " "CUDFflagsplus"%cx%d", objectives[iobj]->coefficients[i], mult, objectives[iobj]->sindex[i]);
    }
    fprintf(lpfile, "\n");

    fprintf(lpfile, "Subject To\n");
    for (unsigned int i = 0; i < iobj; i++) {
      if (objectives[i]->nb_coeffs > 0) {
	for (int j = 0, nbc = 0; j < objectives[i]->nb_coeffs; j++, nbc++) {
	  if (nbc == 20) { // scip does not like too long lines
	    nbc = 0;
	    fprintf(lpfile, "\n  ");
	  }
	  fprintf(lpfile, " "CUDFflagsplus"%cx%d", objectives[i]->coefficients[j], mult, objectives[i]->sindex[j]);
	}
	fprintf(lpfile, " = "CUDFflags"\n", objvals[i]);
      }
    }

    fclose(lpfile);

    if (verbosity < 2)
      sprintf(command, "cat %s >> %s; %s %s > %s 2> /dev/null", 
    	      ctlpfilename, lpfilename, lpsolver, lpfilename, lpoutfilename);
    else
      sprintf(command, "cat %s >> %s; %s %s | tee %s", 
	      ctlpfilename, lpfilename, lpsolver, lpfilename, lpoutfilename);

    // Set timeout for SCIP - change settings file
    set_scip_timeout(10);
      
    if (system(command) == -1) { // call solver
      fprintf(stderr, "mccs: error while calling solver.\n");
      exit(-1);
    }

    if ((fsol = fopen(lpoutfilename, "r")) == (FILE *)NULL) {
      fprintf(stderr, "Cannot open solution file \"%s\".\n", lpoutfilename);
      exit(-1);
    }
    
    bool found (false);
    bool sat_call (false);
    fgets(command, 1000, fsol);
    if (command != NULL) {
      switch (command[0]) {
        case 'S': // scip ?
            while (!found && (! feof(fsol)) && (fgets(command, 1000, fsol) != NULL)) {
                if (strncmp(command, "primal solution", 15) == 0) {
                if (fgets(command, 1000, fsol) != NULL) { // read ===========
                    if (fgets(command, 1000, fsol) != NULL) {  // read empty line
                        if (fgets(command, 1000, fsol) != NULL) { // read objective value or no solution
                            if (strncmp(command, "objective value:", 16) == 0) {
                                status = 1;
                                found = true;
                                sat_call = true; // sat
                                if (sscanf(command+16, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;
                                // Reading scip solution
                                for (int i = 0; i < nb_packages; i++) solution[i] = 0; // Set solution values to 0
                                while ((! feof(fsol)) && (fgets(command, 1000, fsol) != NULL)){
                                    if (command[0] == 'x') {
                                    if (sscanf(command+1, "%d", &rank) > 0) 
                                    solution[rank] = 1;
                                    } else // end of solution reached
                                    break;
                                }
                            } else if (strncmp(command, "no solution available", 21) == 0) {
                                found = true;
                                sat_call = false; // unsat
                                if (status == -1) // if a solution hasn't been found in a previous iteration
                                    status = 0;
                            }
                        } else {
                            fprintf(stderr, "mccs: error while reading solution file.\n");
                            exit(-1);
                        }
                    }
                    else {
                        fprintf(stderr, "mccs: error while reading solution file.\n");
                        exit(-1);
                    }
                }
                else {
                    fprintf(stderr, "mccs: error while reading solution file.\n");
                    exit(-1);
                }
                }
            }
            break;
        case 'W':  // COIN or CPLEX ?
            while (!found && (! feof(fsol)) && (fgets(command, 1000, fsol) != NULL)) {
                if ((strncmp(command, "Coin:Infeasible - objective value", 33) == 0) ||
                    (strncmp(command, "CPLEX> MIP - Integer infeasible.", 32) == 0)){
                    found = true;
                    sat_call = false;
                    if (status == -1) // if a solution hasn't been found in a previous iteration
                        status = 0;
                }
                else if (strncmp(command, "Coin:Optimal - objective value", 30) == 0) {
                    found = true;
                    sat_call = true;
                    status = 1;
                    if (sscanf(command+30, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval; // obj vector
                    // Reading COIN solution
                    if (iobj + 1 == nb_objectives) { // read solution
                        for (int i = 0; i < nb_packages; i++) solution[i] = 0; // Set solution values to 0
                        while ((! feof(fsol)) && (fgets(command, 1000, fsol) != NULL))
                        if ((command[0] == ' ') && (command[8] == 'x')) {
                        if (sscanf(command+9, "%d", &rank) > 0) 
                        if (((command[30] == ' ') && (command[31] == '1')) || 
                            ((command[31] == ' ') && (command[32] == '1')) ||
                            ((command[32] == ' ') && (command[33] == '1')) ||
                            ((command[33] == ' ') && (command[34] == '1'))) solution[rank] = 1;
                        } else // end of solution reached
                        break;
                    }
                }/* else if (strncmp(command, "CPLEX> MIP - Integer optimal solution:  Objective = ", 52) == 0) {*/
            }
            break;
        }
    }
    
    if (!sat_call) { // if unsat return right away
        break;
    }
    // If we are here with a status = -1, then we were unable to read the solution (or the infeasability)
    if (status == -1) {
      fprintf(stderr, "ERROR: Cannot read solution from lp solver.\n");
      exit(-1);
    }
    double obj_val = objective_value();
	printf("# Objective value %d = %f\n", iobj, obj_val);

  } // end for objectives

  if (CLEAN_FILES) {
    remove(ctlpfilename);
    remove(lpfilename);
    remove(lpoutfilename);
    const std::string settings_file ("/home/mcabral/thesis/mccs-1.1/" + std::to_string(getpid()) + "_scip.txt");
    remove(settings_file.c_str());
  }

  return status;
}

// get objective function value
CUDFcoefficient lp_solver::objective_value() { return objval; }

// solution initialisation
int lp_solver::init_solutions() { return 0; }

// lp solvers have integer variables
bool lp_solver::has_intvars() { return true; }

// set integer variable range (must be used before end_objective)
int lp_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) { 
  lb[rank] = lower;
  ub[rank] = upper;
  return 0; 
}

// return the status of a package within the final configuration
CUDFcoefficient lp_solver::get_solution(CUDFVersionedPackage *package) { return solution[package->rank]; }

// initialize objective function
int lp_solver::begin_objectives(void) { return 0; }

// return the package coefficient of the objective function 
CUDFcoefficient lp_solver::get_obj_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// return the package coefficient of the objective function 
CUDFcoefficient lp_solver::get_obj_coeff(int rank) { return get_coeff(rank); }

// set the package coefficient of the objective function 
int lp_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  set_coeff(package, value);
  return 0; 
}

// set the column coefficient of the objective function 
int lp_solver::set_obj_coeff(int rank, CUDFcoefficient value) { 
  set_coeff(rank, value);
  return 0; 
}

int lp_solver::new_objective(void) {
  reset_coeffs();
  return 0; 
}

// add current objective to the set of objectives
int lp_solver::add_objective(void) {
  push_obj();
  return 0;
}

// finalize the objective function
int lp_solver::end_objectives(void) { return 0; }

// initialize constraints
int lp_solver::begin_add_constraints(void) { return 0; }

// begin a new constraint
int lp_solver::new_constraint(void) { 
  reset_coeffs(); 
  return 0; 
}

// get the package coefficient of the current constraint
CUDFcoefficient lp_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// get the package coefficient of the current constraint
CUDFcoefficient lp_solver::get_constraint_coeff(int rank) { return get_coeff(rank); }

// set package coefficient of the current constraint
int lp_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { 
  set_coeff(package, value);
  return 0;
}

// set column coefficient of the current constraint
int lp_solver::set_constraint_coeff(int rank, CUDFcoefficient value) { 
  set_coeff(rank, value);
  return 0;
}

// add current constraint as a greater equal constraint
int lp_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " "CUDFflagsplus"%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " >= 0\n"); else fprintf(ctlpfile, " >= "CUDFflags"\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as a less or equal constraint
int lp_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " "CUDFflagsplus"%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " <= 0\n"); else fprintf(ctlpfile, " <= "CUDFflags"\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as an equality constraint
int lp_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " "CUDFflagsplus"%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " = 0\n"); else fprintf(ctlpfile, " = "CUDFflags"\n", bound);
    nb_constraints++;
  }
  return 0;
}

// finalize constraints
int lp_solver::end_add_constraints(void) { 
  int nbcols = 0, nbvars = 0;

  fprintf(ctlpfile, "Bounds\n");
  for (int i = 0; i < nb_vars; i++) {
    fprintf(ctlpfile, " "CUDFflags" <= x%d <= "CUDFflags"\n", lb[i], i, ub[i]);
  }

  fprintf(ctlpfile, "Binaries\n");
  for (int i = 0; i < nb_vars; i++) {
    if ((lb[i] == 0) && (ub[i] == 1)) {
      nbcols++;
      if (nbcols == 10) { fprintf(ctlpfile, "\n"); nbcols = 0; }
      fprintf(ctlpfile, " x%d", i);
    }
  }
  for (int i = 0; i < nb_vars; i++) {
    if ((lb[i] != 0) || (ub[i] != 1)) {
      if (nbvars == 0) fprintf(ctlpfile, "\nGenerals\n");
      nbcols++;
      nbvars++;
      if (nbcols == 10) { fprintf(ctlpfile, "\n"); nbcols = 0; }
      fprintf(ctlpfile, " x%d", i);
    }
  }
  fprintf(ctlpfile, "\nEnd\n");
  fclose(ctlpfile);
  return 0; 
}
