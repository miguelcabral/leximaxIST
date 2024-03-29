
/*******************************************************/
/* CUDF solver: cud.c                                  */
/* main of the cudf solver                             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cudf.h>
#include <abstract_solver.h>
#include <constraint_generation.h>
#include <criteria.h>
#include <combiner.h>
#include <cudf_reductions.h>
#include <sys/stat.h>
#include <errno.h>
#include <rusage.h>
#include <string>
#include <string.h>
#include <cstdio>
#include <unistd.h>

// scip command
char scip_cmd[80] ("bash /home/mcabral/thesis/mccs-1.1/");
char pid_str[10];

// underlying solver declaration
// allows using solvers withour having to include the whole solver classes
extern abstract_solver *new_lp_solver(char *lpsolver);
extern abstract_solver *new_pblib_solver(char *pbsolver);
//extern abstract_solver *new_ampl_solver(char *amplsolver);
#ifdef USECPLEX 
extern abstract_solver *new_cplex_solver();
#endif
#ifdef USEGUROBI
extern abstract_solver *new_gurobi_solver();
#endif
#ifdef USELPSOLVE 
extern abstract_solver *new_lpsolve_solver();
#endif
#ifdef USEGLPK
extern abstract_solver *new_glpk_solver(bool use_exact);
#endif

bool criteria_opt_var = false;

// print cudf help
void print_help() {
  fprintf(stderr, "This software is distributed under a modified BSD licence (see LICENCE file) and was\n"
	  "partially supported by the European Community's 7th Framework Programme (FP7/2007-2013),\n" 
	  "MANCOOSI project, grant agreement n. 214898.\n");
  fprintf(stderr, "Usual call: mccs -i <input_file> -o <outputfile> <criteria combiner>[<criteria>{, <criteria>}*] <solver option> <other options>?\n");
  fprintf(stderr, "file options:\n");
  fprintf(stderr, " -i <input_file>: set the input file to <input_file> (by default attempt to read on stdin)\n");
  fprintf(stderr, " -o <output_file>: set the output file to <output_file> (by default write on stdout)\n");
  fprintf(stderr, "solver options:\n");
#ifdef USECPLEX
  fprintf(stderr, " -cplex: use IBM ILOG Cplex solver\n");
#endif
#ifdef USEGUROBI
  fprintf(stderr, " -gurobi: use Gurobi solver\n");
#endif
#ifdef USELPSOLVE
  fprintf(stderr, " -lpsolve: use lpsolve solver\n");
#endif
#ifdef USEGLPK
  fprintf(stderr, " -glpk: use glpk solver\n");
#endif
  fprintf(stderr, " -lp <lpsolver>: use lp (cplex format) solver <lpsolver> (tested with scip and cbc)\n");
  fprintf(stderr, " -pblib <pbsolver>: use pseudo boolean solver <pbsolver>\n");
  fprintf(stderr, " -nosolve: do not solve the problem (for debug purpose)\n");
  fprintf(stderr, "constraint generation options:\n");
  fprintf(stderr, " -noreduce: do not reduce the initial problem\n");
  fprintf(stderr, " -only-agregate-constraints: generate only agregate constraints\n");
  fprintf(stderr, " -only-desagregate-constraints: generate only deagregate constraints (default)\n");
  fprintf(stderr, " -all-constraints: generate all kind of constraints (insure redundancy)\n");
  fprintf(stderr, "combining criteria:\n");
  fprintf(stderr, " -lexicographic[<lccriteria>{,<lccriteria>}*]\n");
  fprintf(stderr, "   with <lccriteria> ::= {+,-}leximax[<ccriteria>{,<ccriteria>}*] |\n");
  fprintf(stderr, "                         {+,-}leximin[<ccriteria>{,<ccriteria>}*] |\n");
  fprintf(stderr, "                         <ccriteria>\n");
  fprintf(stderr, "   with  <ccriteria> ::= {+,-}agregate[<ccriteria>{,<ccriteria>}*]{[lambda]}? |\n");
  fprintf(stderr, "                         {+,-}lexagregate[<ccriteria>{,<cccriteria>}*]{[lambda]}? |\n");
  fprintf(stderr, "                         <criteria>\n");
  fprintf(stderr, "   with   <criteria> ::= {+,-}removed{[lambda]}? |                             # number of removed packages \n");
  fprintf(stderr, "                         {+,-}changed{[lambda]}? |                             # number of package with a modified version\n");
  fprintf(stderr, "                         {+,-}notuptodate{[lambda]}? |                         # number of no uptodate packages\n");
  fprintf(stderr, "                         {+,-}new{[lambda]}? |                                 # number of newly installed packages\n");
  fprintf(stderr, "                         {+,-}nunsat[<property:>,<withproviders>]{[lambda]}? | # number of unsatisfied dijunct in property (must be a vpkgformula)\n");
  fprintf(stderr, "                                where <property:> is a property name of type vpkgformula\n");
  fprintf(stderr, "                                  and <withproviders> is a boolean (true or false) telling whether providers have to be taken into account\n");
  fprintf(stderr, "                         {+,-}count[<property:>,<onlynew>]{[lambda]}?          # count the property quantity (must be an int, posint or nat) \n");
  fprintf(stderr, "                                where <property:> is a property name of type int, nat or posint\n");
  fprintf(stderr, "                                  and <onlynew> is a boolean (true or false) telling whether count applies only on newly installed packages\n");
  fprintf(stderr, "                         {+,-}unaligned[<alignment>]{[lambda]}?                # count the number of unaligned packages \n");
  fprintf(stderr, "                                where <alignment> is either packages, pairs, clusters or changes\n");
  fprintf(stderr, "  eg.: -lexicographic[-removed,-notuptodate,-nunsat[recommends:,true],-new]\n");
  fprintf(stderr, "  eg.: -lexicographic[-lexagregate[-removed,-notuptodate],-lexagregate[-nunsat[recommends:,true],-new]]\n");
  fprintf(stderr, "  WARNING: spaces can not be used within a criteria combination.\n");
  fprintf(stderr, "criteria combination shortcuts:\n");
  fprintf(stderr, " -lex[<lccriteria>{,<lccriteria>}*] equivalent to -lexicographic[<lccriteria>{,<lccriteria>}*]\n");
  fprintf(stderr, " -lexagregate[<ccriteria>{,<ccriteria>}*] equivalent to -lex[-lexagregate[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, " -lexsemiagregate[<ccriteria>{,<ccriteria>}*] equivalent to \n");
  fprintf(stderr, "                        -lex[-lexagregate[<ccriteria>,<ccriteria>],-lexagregate[<ccriteria>,<ccriteria>],...]\n");
  fprintf(stderr, " -agregate[<ccriteria>{,<ccriteria>}*] equivalent to -lex[-agregate[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, " -leximax[<ccriteria>{,<ccriteria>}*] equivalent to -lex[-leximax[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, " -leximin[<ccriteria>{,<ccriteria>}*] equivalent to -lex[-lexmin[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, " -lexleximax[<ccriteria>{,<ccriteria>}*] equivalent to -lex[<ccriteria>,-leximax[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, " -lexleximin[<ccriteria>{,<ccriteria>}*] equivalent to -lex[<ccriteria>,-leximin[<ccriteria>{,<ccriteria>}*]]\n");
  fprintf(stderr, "  eg.: -agregate[-removed[100],-notuptodate[50],-nunsat[recommends:,true][10],-new]\n");
  fprintf(stderr, "other options:\n");
  fprintf(stderr, " -fo: full solution output\n");
  fprintf(stderr, " -v<n>: set verbosity level to n\n");
  fprintf(stderr, " -h: print this help\n");

}

// Basic user defined criteria option handling
int get_criteria_options(char *crit_descr, unsigned int &pos, vector< pair<unsigned int, unsigned int> *> *opts) {

  if (crit_descr[pos] == '[') {
    int nb_read = 0;
    unsigned int start = ++pos;

    for (; pos < strlen(crit_descr); pos++)
      switch(crit_descr[pos]) {
      case '[':
	crit_descr[pos] = '\0';
	fprintf(stderr, "ERROR: criteria options: found '[' within criteria options: %s.\n", crit_descr);
	exit(-1);
	break;
      case ']': 
	{
	  unsigned int length = pos - start;
	  if (length == 0) {
	    crit_descr[pos] = '\0';
	    fprintf(stderr, "ERROR: criteria options: found empty criteria option: %s.\n", crit_descr);
	    exit(-1);
	  }
	  opts->push_back(new pair<unsigned int, unsigned int>(start, length));
	  nb_read++;
	  pos++; // ignore ending ']'
	  return nb_read;
	}
	break;
      case ',':
	{
	  unsigned int length = pos - start;
	  if (length == 0) {
	    crit_descr[pos] = '\0';
	    fprintf(stderr, "ERROR: criteria options: found empty criteria option: %s.\n", crit_descr);
	    exit(-1);
	  }
	  opts->push_back(new pair<unsigned int, unsigned int>(start, length));
	  nb_read++;
	  start = ++pos;
	}
	break;
      }

    fprintf(stderr, "ERROR: criteria options: criteria options ended without an ending ']': %s.\n", crit_descr);
    exit(-1);
  } 

  return 0;
}

// Get user defined weight for a criteria
CUDFcoefficient get_criteria_lambda(char *crit_descr, unsigned int &pos, char sign) {
  CUDFcoefficient lambda = 1;
  vector< pair<unsigned int, unsigned int> *> opts;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 1) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    for (unsigned int i = 0; i < length; i++) 
      if ((crit_descr[start+i] < '0') || (crit_descr[start+i] > '9')) {
	crit_descr[start+i+1] = '\0';
	fprintf(stderr, "ERROR: criteria options: a lambda value must be an integer int: %s\n", crit_descr);
	exit(-1);
      }

    if (sscanf(crit_descr+start, CUDFflags, &lambda) != 1) {
      crit_descr[start+length+1] = '\0';
      fprintf(stderr, "ERROR: criteria options: a lambda value is espected here: %s\n", crit_descr);
      exit(-1);
    }
  } else if (n > 1) {
    crit_descr[pos] = '\0';
    fprintf(stderr, "ERROR: criteria options: a lambda value is espected here: %s\n", crit_descr);
    exit(-1);
  }

  if (sign == '+') lambda *= -1;

  return lambda;
}

// Get property name from a user defined criteria
char *get_criteria_property_name(char *crit_descr, unsigned int &pos) {
  vector< pair<unsigned int, unsigned int> *> opts;
  char *property = (char *)NULL;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 1) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    if (crit_descr[start+length-1] != ':') {
      crit_descr[start+length] = '\0';
      fprintf(stderr, "ERROR: criteria options: a property name must end with a ':': %s\n", crit_descr);
      exit(-1);
    }


    if ((property = (char *)malloc((length+1)*sizeof(char))) == (char *)NULL) {
      fprintf(stderr, "ERROR: criteria options: not enough memory to store property name.\n");
      exit(-1);
    }
    
    strncpy(property, crit_descr+start, length);
    property[length] = '\0';
  } else {
    crit_descr[pos] = '\0';
    fprintf(stderr, "ERROR: criteria options: a property name is required here: %s\n", crit_descr);
    exit(-1);
  }

  return property;
}

// Get unaligned criteria parameters
int get_unaligned_criteria_params(char *crit_descr, unsigned int &pos) {
  vector< pair<unsigned int, unsigned int> *> opts;
  int alignment = 0;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 1) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    if (strncmp("packages", crit_descr+start, length) == 0)
      alignment = ALIGNED_PACKAGES;
    else if (strncmp("pairs", crit_descr+start, length) == 0)
      alignment = ALIGNED_PAIRS;
    else if (strncmp("clusters", crit_descr+start, length) == 0)
      alignment = ALIGNED_CLUSTERS;
    else if (strncmp("changes", crit_descr+start, length) == 0)
      alignment = ALIGNED_CHANGES;

  } else {
    crit_descr[pos] = '\0';
    fprintf(stderr, "ERROR: unaligned criteria options:  either packages, pairs, clusters or changes required here: %s\n", crit_descr);
    exit(-1);
  }

  return alignment;
}

// Get name and boolean options from user defined criteria
char *get_criteria_property_name_and_bool(char *crit_descr, unsigned int &pos, bool &value) {
  vector< pair<unsigned int, unsigned int> *> opts;
  char *property = (char *)NULL;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 2) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    if (crit_descr[start+length-1] != ':') {
      crit_descr[start+length] = '\0';
      fprintf(stderr, "ERROR: criteria options: a property name must end with a ':': %s\n", crit_descr);
      exit(-1);
    }


    if ((property = (char *)malloc((length+1)*sizeof(char))) == (char *)NULL) {
      fprintf(stderr, "ERROR: criteria options: not enough memory to store property name.\n");
      exit(-1);
    }
    
    strncpy(property, crit_descr+start, length);
    property[length] = '\0';

    start = opts[1]->first;
    length = opts[1]->second;

    if ((length == 4) && 
	(crit_descr[start+0] == 't') &&
	(crit_descr[start+1] == 'r') &&
	(crit_descr[start+2] == 'u') &&
	(crit_descr[start+3] == 'e')) 
      value = true;
    else if ((length == 5) && 
	     (crit_descr[start+0] == 'f') &&
	     (crit_descr[start+1] == 'a') &&
	     (crit_descr[start+2] == 'l') &&
	     (crit_descr[start+3] == 's') &&
	     (crit_descr[start+4] == 'e'))
      value = false;
    else {
      crit_descr[start+length] = '\0';
      fprintf(stderr, "ERROR: criteria options: a boolean is required here (either 'true' or 'false'): %s\n", crit_descr);
      exit(-1);
    }
  } else {
    crit_descr[pos] = '\0';
    fprintf(stderr, "ERROR: criteria options: a property name and a booleen are required here: %s\n", crit_descr);
    exit(-1);
  }

  return property;
}

unaligned_criteria *unalignedc = (unaligned_criteria *)NULL; // temp

// Process a user defined criteria
CriteriaList *process_criteria(char *crit_descr, unsigned int &pos, bool first_level, vector<abstract_criteria *> *criteria_with_property) {
  CriteriaList *criteria = new CriteriaList();

  if (crit_descr[pos] == '[') {
    for (pos += 1; pos < strlen(crit_descr) && crit_descr[pos] != ']';) {
      unsigned int sign, crit_name, crit_name_length;

      // check for criteria sense
      switch (crit_descr[pos]) {
      case '+':
      case '-':
	sign = pos++;
	crit_name = pos;
	break;
      default:
	fprintf(stderr, "ERROR: criteria options: a criteria description must begin with a sign which gives its sense (- = min, + = max): %s\n", 
		crit_descr+pos);
	exit(-1);
	break;
      }

      // look for end of criteria name
      for (; pos < strlen(crit_descr); pos++) {
	char c = crit_descr[pos];
	if ((c == ',') || (c == '[') || (c == ']')) break; 
      }
      crit_name_length = pos - crit_name;

      // handle criteria
      if (strncmp(crit_descr+crit_name, "removed", crit_name_length) == 0) {
	criteria->push_back(new removed_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "changed", crit_name_length) == 0) {
	criteria->push_back(new changed_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "new", crit_name_length) == 0) {
	criteria->push_back(new new_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "notuptodate", crit_name_length) == 0) {
	criteria->push_back(new notuptodate_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "nunsat", crit_name_length) == 0) {
	bool with_providers = true;
	char *property_name = get_criteria_property_name_and_bool(crit_descr, pos, with_providers);
	if (property_name != (char *)NULL) {
	  abstract_criteria *crit = new nunsat_criteria(property_name, with_providers, get_criteria_lambda(crit_descr, pos, crit_descr[sign]));
	  criteria_with_property->push_back(crit);
	  criteria->push_back(crit);
	}
      } else if (strncmp(crit_descr+crit_name, "count", crit_name_length) == 0) {
	bool onlynew = false;
	char *property_name = get_criteria_property_name_and_bool(crit_descr, pos, onlynew);
	if (property_name != (char *)NULL) {
	  abstract_criteria *crit = new count_criteria(property_name, onlynew, get_criteria_lambda(crit_descr, pos, crit_descr[sign]));
	  criteria_with_property->push_back(crit);
	  criteria->push_back(crit);
	}
      } else if (strncmp(crit_descr+crit_name, "unaligned", crit_name_length) == 0) {
	int alignment = get_unaligned_criteria_params(crit_descr, pos);
	if (alignment > 0) {
	  abstract_criteria *crit = unalignedc = new unaligned_criteria(alignment, (char *)"source:", (char *)"sourceversion:", (char *)"number:", 
									get_criteria_lambda(crit_descr, pos, crit_descr[sign]));
	  criteria->push_back(crit);
	  criteria_with_property->push_back(crit);
	}
      } else if (strncmp(crit_descr+crit_name, "agregate", crit_name_length) == 0) {
	criteria->push_back(new agregate_combiner(process_criteria(crit_descr, pos, false, criteria_with_property), 
						  get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "lexagregate", crit_name_length) == 0) {
	criteria->push_back(new lexagregate_combiner(process_criteria(crit_descr, pos, false, criteria_with_property), 
						     get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (strncmp(crit_descr+crit_name, "leximax", crit_name_length) == 0) {
	if (crit_descr[sign] == '-')
	  criteria->push_back(new leximax_combiner(process_criteria(crit_descr, pos, false, criteria_with_property)));
	else
	  criteria->push_back(new leximin_combiner(process_criteria(crit_descr, pos, false, criteria_with_property)));
      } else if (strncmp(crit_descr+crit_name, "leximin", crit_name_length) == 0) {
	if (crit_descr[sign] == '-')
	  criteria->push_back(new leximin_combiner(process_criteria(crit_descr, pos, false, criteria_with_property)));
	else
	  criteria->push_back(new leximax_combiner(process_criteria(crit_descr, pos, false, criteria_with_property)));
      } else {
	crit_descr[pos] = '\0';
	fprintf(stderr, "ERROR: criteria options: this is not a criteria: %s\n", crit_descr+crit_name);
	exit(-1);
      }

      if (crit_descr[pos] == ',') pos++; // skip comma
    }
  } else {
    fprintf(stderr, "ERROR: criteria options: a criteria list must begin with a '[': %s\n", crit_descr+pos);
    exit(-1);
  }

  pos++;
  return criteria;
}

// Handling user criteria definitions
CriteriaList *get_criteria(char *crit_descr, bool first_level, vector<abstract_criteria *> *criteria_with_property) {
  unsigned int pos = 0;
  return process_criteria(crit_descr, pos, first_level, criteria_with_property);
}

// main CUDF function
int main(int argc, char *argv[]) {    
  FILE *output_file = (FILE *)NULL;
  FILE *output_installed = (FILE *)NULL;
  FILE *output_removed = (FILE *)NULL;
  abstract_solver *solver = (abstract_solver *)NULL;
  abstract_combiner *combiner = (abstract_combiner *)NULL;
  bool nosolve = false;
  bool got_input = false;
  bool fulloutput = false;
  CUDFproblem *problem;
  vector<abstract_criteria *> criteria_with_property;

  // parameter handling
  if (argc > 1) 
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-i") == 0) {
	i++;
	if (i < argc) {
	  FILE *input_file = (FILE *)NULL;

	  if ((input_file = fopen(argv[i], "r")) == (FILE *)NULL) {
	    fprintf(stderr, "ERROR: cannot open file %s as input file.\n", argv[i]);
	    exit(-1);
	  } else {
	    got_input = true;
	    switch (parse_cudf(input_file)) {
	    case 0: break;
	    case 1: fprintf(stderr, "ERROR: invalid input in problem.\n"); exit(-1);
	    case 2: fprintf(stderr, "ERROR: parser memory issue.\n"); exit(-1);
	    }
	    fclose(input_file);
	  }
	}
      } else if (strcmp(argv[i], "-o") == 0) {
	i++;
	if (i < argc) {
	  if ((output_file = fopen(argv[i], "w")) == (FILE *)NULL) {
	    fprintf(stderr, "ERROR: cannot open file %s as ouput file.\n", argv[i]);
	    exit(-1);
	  }
	}
      } else if (strcmp(argv[i], "-fo") == 0) {
	fulloutput = true;
      } else if (strncmp(argv[i], "-v", 2) == 0) {
	sscanf(argv[i]+2, "%u", &verbosity);
      } else if (strcmp(argv[i], "-only-agregate-constraints") == 0) {
	generate_agregate_constraints = true;
	generate_desagregate_constraints = false;
      } else if (strcmp(argv[i], "-only-desagregate-constraints") == 0) {
	generate_agregate_constraints = false;
	generate_desagregate_constraints = true;
      } else if (strcmp(argv[i], "-all-constraints") == 0) {
	generate_agregate_constraints = true;
	generate_desagregate_constraints = true;
      } else if (strcmp(argv[i], "-cov") == 0) {
	criteria_opt_var = true;
      } else if (strcmp(argv[i], "-noreduce") == 0) {
	use_reduced = false;
      } else if (strncmp(argv[i], "-lex[", 5) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+4, true, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexicographic_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lex option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-lexicographic[", 15) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+14, true, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexicographic_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lexicographic option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-agregate[", 10) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+9, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new agregate_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -agregate option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-lexagregate[", 13) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+12, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexagregate_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lexagregate option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-lexsemiagregate[", 17) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+16, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexsemiagregate_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lexsemiagregate option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-leximax[", 9) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+8, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new leximax_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -leximax option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-leximin[", 9) == 0) {
        CriteriaList *criteria = get_criteria(argv[i]+8, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new leximin_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -leximin option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-lexleximax[", 12) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+11, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexleximax_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lexleximax option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strncmp(argv[i], "-lexleximin[", 12) == 0) {
	CriteriaList *criteria = get_criteria(argv[i]+11, false, &criteria_with_property);
	if (criteria->size() > 0)
	  combiner = new lexleximin_combiner(criteria);
	else { 
	  fprintf(stderr, "ERROR: -lexleximin option requires a list of criteria.\n"); 
	  exit(-1); 
	}
      } else if (strcmp(argv[i], "-h") == 0) {
	print_help();
	exit(-1);
      } else if (strcmp(argv[i], "-nosolve") == 0) {
	nosolve = true;
      } else if (strcmp(argv[i], "-lp") == 0) {
	if (++i < argc) {
	  /*struct stat sts;
	  if (stat(argv[i], &sts) == -1 && errno == ENOENT) {
	    fprintf(stderr, "ERROR: -lp option require a lp solver: -lp <lpsolver> and %s does not exist.\n", argv[i]);
	    exit(-1);
	  } else*/
        // change to the scip script identified by this pid
        const int str_size (snprintf(pid_str, 10, "%d", getpid()));
        if (str_size < 0 || str_size >= 10) {
            fprintf(stderr, "Error converting PID to c-style string - PID is possibly larger than expected\n");
        }
        strcat(scip_cmd, pid_str);
        strcat(scip_cmd, "_scip.sh");
        //solver = new_lp_solver(scip_cmd);
	solver = new_lp_solver(argv[i]);
	} else {
	  fprintf(stderr, "ERROR: -lp option require a lp solver: -lp <lpsolver>\n");
	  exit(-1);
	}
      } else if (strcmp(argv[i], "-pblib") == 0) {
	if (++i < argc) {
	  struct stat sts;
	  if (stat(argv[i], &sts) == -1 && errno == ENOENT) {
	    fprintf(stderr, "ERROR: -pblib option require a PB solver: -pblib <pbsolver> and %s does not exist.\n", argv[i]);
	    exit(-1);
	  } else
	    solver = new_pblib_solver(argv[i]);
	} else {
	  fprintf(stderr, "ERROR: -pblib option require a PB solver: -pblib <pbsolver>\n");
	  exit(-1);
	}
#ifdef USECPLEX
      } else if (strcmp(argv[i], "-cplex") == 0) {
	solver = new_cplex_solver();
#endif
#ifdef USEGUROBI
      } else if (strcmp(argv[i], "-gurobi") == 0) {
	solver = new_gurobi_solver();
#endif
#ifdef USELPSOLVE
      } else if (strcmp(argv[i], "-lpsolve") == 0) {
	solver = new_lpsolve_solver();
#endif
#ifdef USEGLPK
      } else if (strcmp(argv[i], "-glpk") == 0) {
	solver = new_glpk_solver(false);
#endif
      } else {
	fprintf(stderr, "ERROR: unrecognized option %s\n", argv[i]);
	exit(-1);
      }
    }

  //  if (input_file == (FILE *)NULL) input_file = stdin;
  if (output_file == (FILE *)NULL) output_file = stdout;

  // if no input file defined, then use stdin
  if (! got_input) {
    switch (parse_cudf(stdin)) {
    case 0: break;
    case 1: fprintf(stderr, "ERROR: invalid input in problem.\n"); exit(-1);
    case 2: fprintf(stderr, "ERROR: parser memory issue.\n"); exit(-1);
    }
  }

  // if whished, print out the read problem
  if (verbosity > 2) {
    fprintf(stdout, "================================================================\n");
    if (properties.size() > 0) {
      fprintf(stdout, "preamble:\n");
      print_properties(stdout, &properties);
      fprintf(stdout, "\n");
    }
    fprintf(stdout, "# %d versioned packages:\n\n", (int)all_packages.size());
    for (CUDFVersionedPackageListIterator ipkg = all_packages.begin(); ipkg != all_packages.end(); ipkg++)
      print_versioned_package(stdout, *ipkg, false);
    print_problem(stdout, the_problem);

    fprintf(stdout, "================================================================\n");
    fprintf(stdout, "%d virtual packages:\n\n", (int)all_virtual_packages.size());
    for (CUDFVirtualPackageListIterator vpkg = all_virtual_packages.begin(); vpkg != all_virtual_packages.end(); vpkg++)
      print_virtual_package(stdout, *vpkg);
    fprintf(stdout, "================================================================\n");
  }

  // choose the solver
  if (solver == (abstract_solver *)NULL) 
#ifdef USECPLEX
    solver = new_cplex_solver();
#else
  #ifdef USEGLPK
    solver = new_glpk_solver(false);
  #else
    #ifdef USELPSOLVE
    solver = new_lpsolve_solver();
    #else
    { fprintf(stderr, "ERROR: no solver defined\n"); exit(-1); }
    #endif
  #endif
#endif

  // check criteria properties
  for (vector<abstract_criteria *>::iterator icrit = criteria_with_property.begin(); icrit != criteria_with_property.end(); icrit++)
    (*icrit)->check_property(the_problem);

  // default combiner
  if (combiner == (abstract_combiner *)NULL) {
    CriteriaList *criteria = new CriteriaList();
    criteria->push_back(new removed_criteria());
    combiner = new lexicographic_combiner(criteria);
  }

  // reduce the problem (if use_reduced is true)
  if (combiner->can_reduce()) {
    if (verbosity > 0) fprintf(stdout, "Can reduce graph.\n");
  } else {
    use_reduced = false;
    if (verbosity > 0) fprintf(stdout, "Can NOT reduce graph.\n");
  }
  problem = compute_reduced_CUDF(the_problem);

  // combiner initialization
  combiner->initialize(problem, solver);
  
  // temp
  if ((verbosity > 2) && (unalignedc == (unaligned_criteria *)NULL)) {
    unalignedc = new unaligned_criteria(ALIGNED_CHANGES, (char *)"source:", (char *)"sourceversion:", (char *)"number:", 1);
    unalignedc->check_property(the_problem);
    unalignedc->initialize(problem, solver);
  }
    printf("# solving \n");
  // generate the constraints, solve the problem and print out the solutions
  if ((problem->all_packages->size() > 0) && (generate_constraints(problem, *solver, *combiner) == 0) && (! nosolve) && (solver->solve())) {
    double t = read_cpu_time();
    printf("# solving time: %fs\n", t);
#ifndef CPLEX
    solver->init_solutions(); // in cplex we store the solution at the end of each iteration
#endif

    double obj = solver->objective_value();
    if (verbosity > 2) {
      fprintf(stdout, "================================================================\n");
      printf("Objective value: %f\n", obj);

      for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++)
	printf("%s = "CUDFflags"\n", (*ipkg)->versioned_name, solver->get_solution(*ipkg));
      
      fprintf(stdout, "================================================================\n");
    
      fprintf(output_file, "\n");
    }

    if (verbosity > 2) {
      output_installed = fopen("installed.txt", "w");
      output_removed = fopen("removed.txt", "w");
    }

    if (properties.size() > 0) {
      fprintf(output_file, "\npreamble: \n");
      print_properties(output_file, problem->properties);
      fprintf(output_file, "\n\n");
    }

    // printing out CUDF solution
    int nb_installed = 0;
    int nb_removed = 0;
    int nb_newinstalled = 0;
    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      if (solver->get_solution(*ipkg)) { 
	nb_installed++; 
	if (! (*ipkg)->installed) { 
	  nb_newinstalled++; 
	  if (output_installed != (FILE *)NULL) print_versioned_package_as_installed(output_installed, (*ipkg), true); 
	}
	print_versioned_package_as_installed(output_file, (*ipkg), true); 
      } else {
	if (fulloutput) print_versioned_package_with_install(output_file, (*ipkg), 0, true);
	if ((*ipkg)->installed) { 
	  nb_removed++; 
	  if (output_removed != (FILE *)NULL) print_versioned_package(output_removed, (*ipkg), true); 
	}
      }
    }

    if ((verbosity > 2) && (unalignedc != (unaligned_criteria *)NULL)) unalignedc->display_struct(); // temp
    
    if (verbosity > 2) {
      fclose(output_installed);
      fclose(output_removed);
    }

    // print out additional informations 
    fprintf(output_file, "# problem = %zu packages (%zu virtual packages), %zu installed packages.\n", 
	    all_packages.size(), all_virtual_packages.size(), installed_packages.size());
    fprintf(output_file, "# request = %zu install, %zu remove, %zu upgrade.\n", 
	    (problem->install == (CUDFVpkgList *)NULL)?0:problem->install->size(), 
	    (problem->remove  == (CUDFVpkgList *)NULL)?0:problem->remove->size(), 
	    (problem->upgrade == (CUDFVpkgList *)NULL)?0:problem->upgrade->size());
    fprintf(output_file, "# solution = %d installed packages, %d new installed, %d has been removed.\n", nb_installed, nb_newinstalled, nb_removed);
    //fprintf(output_file, "# objective value = %f.\n", obj);
    //if (verbosity > 0) printf(">>>> Objective value = %f.\n", obj);
  } else {
    if (verbosity > 0) fprintf(stdout, "================================================================\n");
    double t = read_cpu_time();
    printf("# solving time: %fs\n", t);
    fprintf(stdout, "No solution found.\n");
    fprintf(output_file, "FAIL\n");
    fprintf(output_file, "No solution found.\n");
  }
  exit(0);
}
