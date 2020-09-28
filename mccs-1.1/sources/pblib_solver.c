
/*******************************************************/
/* CUDF solver: pblib_solver.c                         */
/* Interface to the pblib format solvers               */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <pblib_solver.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#define CLEAN_FILES 1
#define TMP_FILES_PATH "/tmp/"

// external function for solver creation
abstract_solver *new_pblib_solver(char *pb_solver) { return new pblib_solver(pb_solver); }

// solver initialisation
int pblib_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {

  nb_packages = all_versioned_packages->size();
  this->all_versioned_packages = all_versioned_packages;

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  nb_constraints = 0;
  mult = ' ';

  for (int i = 0; i < ((int)strlen(pb_solver) - 7); i++)
    if ((pb_solver[i] == 'm') && (strncmp(pb_solver+i, "minisat", 7) == 0)) { mult = '*'; break; }

  solution = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));

  sprintf(ctpbfilename, TMP_FILES_PATH "ctpblib_%lu_%lu.lp", (long unsigned)getuid(), (long unsigned)getpid()); 
  ctpbfile = fopen(ctpbfilename, "w");

  if (solution == (CUDFcoefficient *)NULL) {
    fprintf(stderr, "lp_solver: intialize: not enough memory to store solution.\n");
    exit(-1);
  } else if (ctpbfile == (FILE *)NULL) {
    fprintf(stderr, "lp_solver: intialize: can not open %s.\n", ctpbfilename);
    exit(-1);
  } else
    return 0;
}

// write the problem into a file
int pblib_solver::writelp(char *filename) { return 0; }

// solve the current problem
int pblib_solver::solve() {
  int status = 0;
  int rank, iobjval;
  char command[1024];
  FILE *fsol = (FILE *)NULL;
  bool is_scip = false;
  CUDFcoefficient objvals[20];  int nb_objectives = (int)objectives.size();

  for (int i = 0; i < ((int)strlen(pb_solver) - 4); i++)
    if ((pb_solver[i] == 's') && (strncmp(pb_solver+i, "scip", 4) == 0)) { is_scip = true; break; }

  sprintf(pbfilename, TMP_FILES_PATH "pblib_%lu_%lu.opb", (long unsigned)getuid(), (long unsigned)getpid()); 
  sprintf(pboutfilename, TMP_FILES_PATH "pblib_%lu_%lu.out", (long unsigned)getuid(), (long unsigned)getpid()); 

  for (int iobj = 0; iobj < nb_objectives; iobj++) {
    if (objectives[iobj]->nb_coeffs == 0) continue;

    if ((pbfile = fopen(pbfilename, "w")) == (FILE *)NULL) {
      fprintf(stderr, "pblib_solver: cannot open %s.\n", pbfilename);
      exit(-1);
    }

    fprintf(pbfile, "* #variable= %d #constraint= %d \n* a CUDF problem\nmin:", nb_vars, nb_constraints+iobj);

    for (int i = 0; i < objectives[iobj]->nb_coeffs; i++) 
      fprintf(pbfile, " "CUDFflagsplus"%cx%d", objectives[iobj]->coefficients[i], mult, objectives[iobj]->sindex[i]);
    fprintf(pbfile, ";\n");

    for (int i = 0; i < iobj; i++) {
      if (objectives[i]->nb_coeffs > 0) {
	char *sep = (char *)"";
	for (int j = 0; j < objectives[i]->nb_coeffs; j++) {
	  fprintf(pbfile, "%s"CUDFflagsplus"%cx%d", sep, objectives[i]->coefficients[j], mult, objectives[i]->sindex[j]);
	  sep = (char *)" ";
	}
	fprintf(pbfile, " = "CUDFflags";\n", objvals[i]);
      }
    }

    fclose(pbfile);

    if (verbosity < 2) {
      if (is_scip)
	sprintf(command, "cat %s >> %s; %s -f %s > %s 2> /dev/null", 
		ctpbfilename, pbfilename, pb_solver, pbfilename, pboutfilename);
      else
	sprintf(command, "cat %s >> %s; %s %s > %s 2> /dev/null", 
		ctpbfilename, pbfilename, pb_solver, pbfilename, pboutfilename);
    } else {
      if (is_scip)
	sprintf(command, "cat %s >> %s; %s -f %s | tee %s", 
		ctpbfilename, pbfilename, pb_solver, pbfilename, pboutfilename);
      else
	sprintf(command, "cat %s >> %s; %s %s | tee %s", 
		ctpbfilename, pbfilename, pb_solver, pbfilename, pboutfilename);
    }

    if (system(command) == -1) {
      fprintf(stderr, "mccs: error while calling solver.\n");
      exit(-1);
    }

    if ((fsol = fopen(pboutfilename, "r")) == (FILE *)NULL) {
      fprintf(stderr, "Cannot open solution file \"%s\".\n", pboutfilename);
      exit(-1);
    }
      
    for (int i = 0; i < nb_packages; i++) solution[i] = 0;
      

    if (is_scip) {
      int state = 1;
      size_t size = 2048;
      char buff[2048]; char *buffer = buff;

      while (! feof(fsol)) { /* seek : primal solution: */
	int nbread = getline((char **)&buffer, &size, fsol);
	if ((state == 1) && (buffer[0] == 'p') && (nbread >= 16) && (strncmp(buffer, "primal solution:", 16) == 0)) {
	  nbread = getline((char **)&buffer, &size, fsol); /* forget next line */
	  nbread = getline((char **)&buffer, &size, fsol); /* forget next line */
	  state = 2;
	} else if (state == 2) {
	  if ((buffer[0] == 'n') && (nbread >= 21) && (strncmp(buffer, "no solution available", 21) == 0))
	    return 0;
	  else if ((buffer[0] == 'o') && (nbread >= 16) && (strncmp(buffer, "objective value:", 16) == 0)) {
	    int i = 17;
	    while ((buffer[i] != '-') && ((buffer[i] < '0') || (buffer[i] > '9'))) i++;
	    if (sscanf(buffer+i, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;
	    status = 1;
	    state = 3;
	  }
	} else if (state == 3) {
	  if (buffer[0] == ' ') 
	    break;
	  else if ((buffer[0] == 'x') && (sscanf(buffer+1, "%d", &rank) > 0)) 
	    solution[rank-1] = 1;
	}
      }

    } else {

      objval = objvals[iobj] = 0;

      while (! feof(fsol)) {
	char c = fgetc(fsol);
	switch (c) {
	case '\n':
	  break;
	case 's': 
	  fgetc(fsol);
	  if (fgets(command, 1000, fsol) != NULL) {
	    if (strncmp(command, "OPTIMUM FOUND", 12) == 0) status = 1; else return 0;
	  }
	  break;
	case 'v':
	  while (! feof(fsol)) {
	    c = fgetc(fsol);
	    if (c == '\n')
	      break;
	    else if (c == '-')
	      while (! feof(fsol)) { if (fgetc(fsol) == ' ') break;}
	    else if (c == 'x')
	      if (fscanf(fsol, "%d", &rank) > 0) solution[rank-1] = 1;
	  }
	  break;
	case 'o':
	  fgetc(fsol);
	  if (fscanf(fsol, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;
	default:
	  while ((! feof(fsol)) && (fgetc(fsol) != '\n'));
	}
      }
    }
  }

  if (CLEAN_FILES) {
    remove(ctpbfilename);
    remove(pbfilename);
    remove(pboutfilename);
  }

  return status;
}

// get objective function value
CUDFcoefficient pblib_solver::objective_value() { return objval; }

// solution initialisation
int pblib_solver::init_solutions() { return 0; }

// return the status of a package within the final configuration
CUDFcoefficient pblib_solver::get_solution(CUDFVersionedPackage *package) { return solution[package->rank]; }

// initialize objective function
int pblib_solver::begin_objectives(void) { return 0; }

// return the package coefficient of the objective function 
CUDFcoefficient pblib_solver::get_obj_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// return the package coefficient of the objective function 
CUDFcoefficient pblib_solver::get_obj_coeff(int rank) { return get_coeff(rank); }

// set the package coefficient of the objective function 
int pblib_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  set_coeff(package, value);
  return 0; 
}

// set the column coefficient of the objective function 
int pblib_solver::set_obj_coeff(int rank, CUDFcoefficient value) { 
  set_coeff(rank, value);
  return 0; 
}

int pblib_solver::new_objective(void) { reset_coeffs(); return 0; }

int pblib_solver::add_objective(void) {
  push_obj();
  return 0;
}

// finalize the objective function
int pblib_solver::end_objectives(void) { return 0; }

// initialize constraints
int pblib_solver::begin_add_constraints(void) { return 0; }

// begin a new constraint
int pblib_solver::new_constraint(void) { reset_coeffs(); return 0; }

// get the package coefficient of the current constraint
CUDFcoefficient pblib_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// get the package coefficient of the current constraint
CUDFcoefficient pblib_solver::get_constraint_coeff(int rank) { return get_coeff(rank); }

// set package coefficient of the current constraint
int pblib_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { 
  set_coeff(package, value);
  return 0;
}

// set column coefficient of the current constraint
int pblib_solver::set_constraint_coeff(int rank, CUDFcoefficient value) { 
  set_coeff(rank, value);
  return 0;
}

// add current constraint as a greater equal constraint
int pblib_solver::add_constraint_geq(CUDFcoefficient bound) {
  char *sep = (char *)"";

  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++) {
      fprintf(ctpbfile, "%s"CUDFflagsplus"%cx%d", sep, coefficients[i], mult, sindex[i]);
      sep = (char *)" ";
    }
    if (bound == 0) fprintf(ctpbfile, " >= 0;\n"); else fprintf(ctpbfile, " >= "CUDFflags";\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as a less or equal constraint
int pblib_solver::add_constraint_leq(CUDFcoefficient bound) {
  char *sep = (char *)"";

  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++) {
      fprintf(ctpbfile, "%s"CUDFflagsplus"%cx%d", sep, coefficients[i], mult, sindex[i]);
      sep = (char *)" ";
    }
    if (bound == 0) fprintf(ctpbfile, " <= 0;\n"); else fprintf(ctpbfile, " <= "CUDFflags";\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as an equality constraint
int pblib_solver::add_constraint_eq(CUDFcoefficient bound) {
  char *sep = (char *)"";

  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++) {
      fprintf(ctpbfile, "%s"CUDFflagsplus"%cx%d", sep, coefficients[i], mult, sindex[i]);
      sep = (char *)" ";
    }
    if (bound == 0) fprintf(ctpbfile, " = 0;\n"); else fprintf(ctpbfile, " = "CUDFflags";\n", bound);
    nb_constraints++;
  }
  return 0;
}


// finalize constraints
int pblib_solver::end_add_constraints(void) { 
  fclose(ctpbfile);
  return 0; 
}
