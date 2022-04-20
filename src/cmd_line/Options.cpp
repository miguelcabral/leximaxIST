#include <Options.h>
#include <leximaxIST_types.h>
#include <leximaxIST_printing.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <iostream>

namespace leximaxIST {

    // getters
    int Options::get_help() {return m_help.get_data();}
    int Options::get_verbosity() {return m_verbosity.get_data();}
    int Options::get_disjoint_cores() {return m_disjoint_cores.get_data();}
    std::string Options::get_optimise() {return m_optimise.get_data();}
    std::string Options::get_approx() {return m_approx.get_data();}
    std::string Options::get_input_file_name() {return m_input_file_name.get_data();}
    double Options::get_timeout() {return m_timeout.get_data();}
    int Options::get_mss_tol() {return m_mss_tol.get_data();}
    int Options::get_mss_add_cls() {return m_mss_add_cls.get_data();}
    int Options::get_mss_incr() {return m_mss_incr.get_data();}
    int Options::get_gia_pareto() {return m_gia_pareto.get_data();}
    int Options::get_gia_incr() {return m_gia_incr.get_data();}
    int Options::get_pb_enc() {return m_pb_enc.get_data();}
    int Options::get_card_enc() {return m_card_enc.get_data();}
    
    // constructor
    Options::Options()
    : m_help (0)
    , m_verbosity (0)
    , m_optimise ("")
    , m_input_file_name ("")
    , m_disjoint_cores (0)
    , m_approx ("")
    , m_timeout (86400)
    , m_mss_tol (0)
    , m_mss_add_cls (1)
    , m_mss_incr (0)
    , m_gia_pareto (0)
    , m_gia_incr (0)
    , m_pb_enc (_PB_SWC_)
    , m_card_enc (_CARD_MTOTALIZER_)
    {
        // help
        const std::string name_tab (2, ' ');
        const std::string exp_tab (4, ' ');
        const std::string values_tab (6, ' ');
        std::string description (name_tab);
        description += "-h, --help\n" + exp_tab;
        description += "print usage with a list of the options\n";
        m_help.set_description(description);
        
        // verbosity
        description = name_tab + "-v <int>\n" + exp_tab;
        description += "verbosity of output\n";
        description += values_tab + "0 (default) - quiet, only prints the solution\n";
        description += values_tab + "1 - print general info\n";
        description += values_tab + "2 - debug - print encoding\n";
        m_verbosity.set_description(description);
        
        // input file
        description = name_tab + "<input_file> is the input file and it must be in pbmo format\n";
        m_input_file_name.set_description(description);
        
        // optimise
        description = name_tab + "--optimise <string>\n";
        description += exp_tab + "find a leximax-optimal solution using the algorithm specified in <string>:\n";
        description += values_tab + "lin_su - static sorting networks with linear search sat-unsat\n";
        description += values_tab + "lin_us - static sorting networks with linear search unsat-sat\n";
        description += values_tab + "bin - static sorting networks with binary search\n";
        description += values_tab + "core_static - static sorting networks with core-guided unsat-sat search\n";
        description += values_tab + "core_merge (default) - core-guided unsat-sat search using dynamic sorting networks that grow by sort and merge\n";
        description += values_tab + "core_rebuild - core-guided unsat-sat search using dynamic sorting networks that grow by rebuild (not incremental)\n";
        description += values_tab + "core_rebuild_incr - core-guided unsat-sat search using dynamic sorting networks that grow by rebuild (incremental)\n";
        m_optimise.set_description(description);
        
        // disjoint cores strategy
        description = name_tab + "--dcs\n";
        description += exp_tab + "when optimising, use the disjoint cores strategy\n";
        m_disjoint_cores.set_description(description);
        
        // approx
        description = name_tab + "--approx <string>\n";
        description += exp_tab + "approximate the leximax-optimum using the approach specified in <string>:\n";
        description += values_tab + "mss - Maximal Satisfiable Subset enumeration (using extended linear search)\n";
        description += values_tab + "gia - Guided Improvement Algorithm adapted to leximax (conversion to CNF using sorting networks)\n";
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
        description += values_tab + "0 - add all satisfied clauses\n";
        description += values_tab + "1 (default) - add as much as possible while trying to even out the upper bound (default)\n";
        description += values_tab + "2 - add only the satisfied clause used in the SAT test\n";
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
        
        // pb-enc
        description = name_tab + "--pb-enc <int>\n";
        description += exp_tab + "Set the encoding of Pseudo-Boolean non-cardinality constraints to CNF\n";
        description += values_tab + "0 - SWC \n";
        description += values_tab + "1 (default) - GTE \n";
        description += values_tab + "2 - Adder\n";
        description += values_tab + "3 - IGTE \n";
        m_pb_enc.set_description(description);
        
        // card-enc
        description = name_tab + "--card-enc <int>\n";
        description += exp_tab + "Set the encoding of cardinality constraints to CNF\n";
        description += values_tab + "0 - cardinality networks\n";
        description += values_tab + "1 - totalizer\n";
        description += values_tab + "2 (default) - modulo totalizer\n";
        m_card_enc.set_description(description);
    }

    /* converts optarg to a double and stores it in d
    * if there is a problem with the conversion, it prints an error message and exits
    */
    void Options::read_double(const char *optarg, const std::string &optname, double &d)
    {
        std::string s (optarg);
        try {
            d = std::stod(s);
        }
        catch (const std::invalid_argument&) {
            print_error_msg("Option '" + optname + "' must be a double");
            exit(EXIT_FAILURE);
        }
        catch (const std::out_of_range&) {
            print_error_msg("Out of range double for Option '" + optname + "'");
            exit(EXIT_FAILURE);
        }
    }

    /* converts optarg to an integer and stores it in i
    * if there is a problem with the conversion, it prints an error message and exits
    */
    void Options::read_integer(const char *optarg, const std::string &optname, int &i)
    {
        std::string s (optarg);
        try {
            i = std::stoi(s);
        }
        catch (const std::invalid_argument&) {
            print_error_msg("Option '" + optname + "' must be an integer");
            exit(EXIT_FAILURE);
        }
        catch (const std::out_of_range&) {
            print_error_msg("Out of range integer for Option '" + optname + "'");
            exit(EXIT_FAILURE);
        }
    }

    /* converts optarg to an integer, checks if it is between 0 and 9, and stores it in digit
    * if there is a problem with the conversion, it prints an error message and exits
    */
    void Options::read_digit(const char *optarg, const std::string &optname, int &digit)
    {
        read_integer(optarg, optname, digit);
        if (digit > 9 || digit < 0) {
            print_error_msg("Option '" + optname + "' must be an integer between 0 and 9");
            exit(EXIT_FAILURE);
        }
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
            {"mss-add-cls",  required_argument,  0, 504},
            {"pb-enc",  required_argument,  0, 505},
            {"card-enc",  required_argument,  0, 506},
            {0, 0, 0, 0}
                };
        int c;
        while (true) {
        /* getopt_long stores the option index here. */
        int option_index = 0;
        c = getopt_long(argc, argv, "hv:", long_options, &option_index);
        opterr = 0;
        /* Detect the end of the options. */
        if (c == -1) break;
        switch (c) {
                case 0: if (long_options[option_index].flag != 0) break;
                    else return false;
                case 'h': m_help.get_data() = 1; break;
                case 'v': read_digit(optarg, "-v", m_verbosity.get_data()); break;
                case 500: m_optimise.get_data() = optarg; break;
                case 501: m_approx.get_data() = optarg; break;
                case 502: read_integer(optarg, "--mss-tol", m_mss_tol.get_data()); break;
                case 503: read_double(optarg, "--timeout", m_timeout.get_data()); break;
                case 504: read_digit(optarg, "--mss-add-cls", m_mss_add_cls.get_data()); break;
                case 505: read_digit(optarg, "--pb-enc", m_pb_enc.get_data()); break;
                case 506: read_digit(optarg, "--card-enc", m_card_enc.get_data()); break;
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
        if (m_help.get_data() == 0) {
            if (optind >= argc) {
                print_error_msg("pbmo file expected");
                exit(EXIT_FAILURE);
            } else {
                m_input_file_name.get_data() = argv[optind++];
            }
        }
        if (optind < argc) {
            print_error_msg("WARNING: Unprocessed options at the end.");
        }
        
        // check if either optimise or approximate are set
        if (get_optimise().empty() && get_approx().empty() && m_help.get_data() == 0) {
            print_error_msg("missing one of the options --optimise or --approx");
            exit(EXIT_FAILURE);
        }
        
        return return_value;
    }
    
    void Options::print_usage(std::ostream &os)
    {
        os << "Usage: ./leximaxIST [<options>] <input_file>\n";
        os << m_input_file_name.get_description();
        os << "Options:\n";
        os << m_help.get_description();
        os << m_verbosity.get_description();
        os << m_optimise.get_description();
        os << m_disjoint_cores.get_description();
        os << m_approx.get_description();
        os << m_timeout.get_description();
        os << m_mss_incr.get_description();
        os << m_mss_add_cls.get_description();
        os << m_mss_tol.get_description();
        os << m_gia_incr.get_description();
        os << m_gia_pareto.get_description();
        os << m_pb_enc.get_description();
        os << m_card_enc.get_description();
    }

}
