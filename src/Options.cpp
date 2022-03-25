#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "Options.hh"
using std::cerr;
using std::endl;

Options::Options()
: m_help (0)
, m_verbosity (0)
, m_optimise ("core-merge")
, m_input_file_name ("")
, m_disjoint_cores (0)
, m_approx ("none")
, m_timeout (86400)
, m_mss_tol (0)
, m_mss_add_cls (1)
, m_mss_incr (0)
, m_gia_pareto (0)
, m_gia_incr (0)
{
    // help
    const std::string name_tab (2, ' ');
    const std::string exp_tab (4, ' ');
    std::string description (name_tab);
    description += "-h, --help\n" + exp_tab;
    description += "print usage with a list of the options\n";
    m_help.set_description(description);
    
    // verbosity
    description = name_tab + "-v <int>\n" + exp_tab;
    description += "verbosity of output\n";
    description += exp_tab + "0 (default) - quiet, only prints the solution\n";
    description += exp_tab + "1 - print general info\n";
    description += exp_tab + "2 - debug - print encoding\n";
    m_verbosity.set_description(description);
    
    // input file
    description = name_tab + "<input_file> is the input file and it must be in pbmo format\n";
    m_input_file_name.set_description(description);
    
    // optimise
    description = name_tab + "--optimise <string>\n";
    description += exp_tab + "find a leximax-optimal solution using the algorithm specified in <string>:\n";
    description += exp_tab + "lin_su - static sorting networks with linear search sat-unsat\n";
    description += exp_tab + "lin_us - static sorting networks with linear search unsat-sat\n";
    description += exp_tab + "bin - static sorting networks with binary search\n";
    description += exp_tab + "core_static - static sorting networks with core-guided unsat-sat search\n";
    description += exp_tab + "core_merge (default) - core-guided unsat-sat search using dynamic sorting networks that grow by sort and merge\n";
    description += exp_tab + "core_rebuild - core-guided unsat-sat search using dynamic sorting networks that grow by rebuild (not incremental)\n";
    description += exp_tab + "core_rebuild_incr - core-guided unsat-sat search using dynamic sorting networks that grow by rebuild (incremental)\n";
    m_optimise.set_description(description);
    
    // disjoint cores strategy
    description = name_tab + "--dcs\n";
    description += exp_tab + "use the disjoint cores strategy\n";
    m_disjoint_cores.set_description(description);
    
    // approx
    description = name_tab + "--approx <string>\n";
    description += exp_tab + "approximate the leximax-optimum using the approach specified in <string>:\n";
    description += exp_tab + "mss - Maximal Satisfiable Subset enumeration (using extended linear search)\n";
    description += exp_tab + "gia - Guided Improvement Algorithm adapted to leximax (conversion to CNF using sorting networks)\n";
    m_approx.set_description(description);
    
    // timeout
    description = name_tab + "--timeout <double>\n";
    description += exp_tab + "specify a timeout (in seconds) for the approximation (default: 86400 (one day))\n";
    m_timeout.set_description(description);
    
    // mss-tol
    description = name_tab + "--mss-tol <int>\n";
    description += exp_tab + "tolerance for the choice of the next clause tested for satisfiability, in the MSS linear search\n";
    description += exp_tab + "the value must be a percentage, i.e. an integer between 0 and 100\n";
    description += exp_tab + "default: 50\n";
    m_mss_tol.set_description(description);
    
    // mss-add-cls
    description = name_tab + "--mss-add-cls <int>\n";
    description += exp_tab + "specify how to add the clauses to the MSS in construction during MSS extended linear search:\n";
    description += exp_tab + "0 - add all satisfied clauses\n";
    description += exp_tab + "1 (default) - add as much as possible while trying to even out the upper bound (default)\n";
    description += exp_tab + "2 - add only the satisfied clause used in the SAT test\n";
    m_mss_add_cls.set_description(description);
    
    // mss-incr
    description = name_tab + "--mss-incr\n";
    description += exp_tab + "use incremental SAT solving during the entire MSS enumeration\n";
    m_mss_incr.set_description(description);
    
    // gia-pareto
    description = name_tab + "--gia-pareto\n";
    description += exp_tab + "when approximating with GIA, this guarantees that the solver will seek Pareto-optimal solutions\n";
    m_gia_pareto.set_description(description);
    
    // gia-incr
    description = name_tab + "--gia-incr\n";
    description += exp_tab + "Use the same incremental SAT solver during the entire execution of the GIA algorithm\n";
    m_gia_incr.set_description(description);
}

/* This function is based on packup's parsing function
 * Paramaters: the same of the main function
 * Parses the command line arguments
 * Returns false if there is an error with the input and true otherwise
 */
bool Options::parse(int argc, char **argv)
{
    bool return_value = true;

    static struct option long_options[] = {
        {"help", no_argument,    &(m_help.get_data()), 1},
        {"dcs",  no_argument,  &(m_disjoint_cores.get_data()), 1},
        {"mss-incr",  no_argument,  &(m_mss_incr.get_data()), 1},
        {"gia-incr",  no_argument,  &(m_gia_incr.get_data()), 1},
        {"gia-pareto",  no_argument,  &(m_gia_pareto.get_data()), 1},
        {"optimise",  required_argument,  0, 500},
        {"approx",  required_argument,  0, 501},
        {"mss-tol",  required_argument,  0, 502},
        {"timeout",  required_argument,  0, 503},
        {0, 0, 0, 0}
             };

    int c;
    while (1) {
       /* getopt_long stores the option index here. */
       int option_index = 0;
       c = getopt_long(argc, argv, "h", long_options, &option_index);
       opterr = 0;
       /* Detect the end of the options. */
       if (c == -1) break;
       switch (c) {
            case 0: if (long_options[option_index].flag != 0) break;
                    else return false;
            case 'h': m_help     = 1; break;
            case 500: m_solver = optarg; break;
            case 501: m_multiplication_string = optarg; break;
            case '?':
             if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
             return_value = false;
             break;
           default:
               return false;
        }
    }

    if (!m_help) {
        if (optind >= argc) {
            cerr << "cnf files expected" << endl;
            return_value = false;
        } else {
            while (optind < argc) {
                // store name of input files
                m_input_files.push_back(argv[optind++]);
                m_num_objectives++;
            }
        }
    }

    if (optind < argc) {
        cerr << "WARNING: Unprocessed options at the end." << endl;
    }
    
    if (m_num_objectives == 0) {
        cerr << "objective function(s) expected" << endl;
        return_value = false;
    }
    
    return return_value;
}

