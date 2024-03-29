# leximaxIST - Guide
## C++ Library
The leximax solver is essentially the class leximaxIST::Solver, defined in the header file leximaxIST_Solver.h.

In order to obtain a leximax-optimal solution of a certain multi-objective instance, one must start by creating a Solver object:
```cpp
#include <leximaxIST_Solver.h>
...
leximaxIST::Solver solver;
```
The hard clauses of the instance can be added one at a time:
```cpp
leximaxIST::Solver solver;
leximaxIST::Clause c; // create empty clause c
c.push_back(1); // add literal 1 to c
c.push_back(-2); // add literal -2 (negation of variable 2) to c
solver.add_hard_clause(c);
```
The variables and literals are represented as in the SAT solver [DIMACS format](https://jix.github.io/varisat/manual/0.2.0/formats/dimacs.html). So, a variable is a positive integer and literals are integers. The negation of a variable *k* is *-k*.

An objective function is added in the form of soft clauses, by using the member function:
```
void add_soft_clauses(const std::vector<Clause> &soft_clauses);
```
The objective function to be minimised corresponds to the sum of falsified soft clauses.

Weights can be added by repetition of soft clauses, but the encoding currently used in the library does not handle well large weights.

Here is an example:
```cpp
leximaxIST::Solver solver;
leximaxIST::Clause c; // create empty clause c
c.push_back(1); // add literal 1 to c
c.push_back(-2); // add literal -2 (negation of variable 2) to c
std::vector<Clause> soft_clauses;
soft_clauses.push_back(c); // copy c to the set of soft clauses
c.clear(); // c is now empty
c.push_back(3); // add literal 3 to c
soft_clauses.push_back(c); // add unit clause to the set of soft clauses
solver.add_soft_clauses(soft_clauses); // add objective function
```
Finally, the following line runs the leximax optimisation algorithm:
```cpp
solver.optimise(); // run optimisation algorithm
```
To retrieve the leximax-optimal solution found, one can write:
```cpp
std::vector<int> solution (solver.get_solution());
```
The vector `solution` corresponds to an assignment.
Entry i of `solution` is either i, meaning variable i is true, or -i, meaning variable i is false.

To obtain the corresponding objective vector, one can write:
```cpp
std::vector<int> obj_vec (solver.get_objective_vector());
```
Entry i of `obj_vec` is the value of the i-th objective function under the assignment found.

Besides optimising, the solver also allows to approximate the leximax-optimum, if one is interested in finding a feasible solution quickly and leximax optimisation is taking too long. For that, one can run the following:
```cpp
solver.approximate();
```

### Parameters and Solver Configuration

#### General Options

| Member function | Description |
| ------ | ------ |
| `void set_opt_mode(const std::string &mode);` | Set the optimisation algorithm |
| `void set_approx(const std::string &algorithm);` | Set the approximation algorithm |
| `void set_verbosity(int v);` | Set verbosity - what information gets printed to stdout |

| Verbosity Values | Description |
| ------ | ------ |
| 0 | prints nothing |
| 1 | prints some information about the algorithm - CPU times, encoding, ... |
| 2 | prints detailed information for debugging |

#### Optimisation Algorithms

| Optimisation Algorithms | Description |
| ------ | ------ |
| 'lin_su' | Linear search SAT-UNSAT with static sorting networks |
| 'lin_us' | Linear search UNSAT-SAT with static sorting networks |
| 'bin' | Binary search with static sorting networks |
| 'core_static' | Core-guided UNSAT-SAT search with static sorting networks |
| 'core_merge' | Core-guided UNSAT-SAT search with dynamic sorting networks that grow incrementally with sort-and-merge |
| 'core_rebuild' | Core-guided UNSAT-SAT search with dynamic sorting networks that are rebuilt non-incrementally |
| 'core_rebuild_incr' | Core-guided UNSAT-SAT search with dynamic sorting networks that are rebuilt incrementally |
| 'ilp' | ILP-based Algorithm (solvers available: Gurobi and CPLEX) |

| Member function | Description |
| ------ | ------ |
| `void set_ilp_solver(const std::string &ilp_solver);` | Select the ILP solver for the ILP-based algorithm |
| `void set_leave_tmp_files(bool val);` | Whether to leave temporary input and output files of the ILP solver |
| `void set_disjoint_cores(bool v);` | Switches the disjoint cores strategy on/off (SAT-based algorithm) |

#### Approximation Algorithms

| Approximation Algorithms | Description |
| ------ | ------ |
| 'mss' | Compute Maximal Satisfiable Subsets (MSSes) using extended linear search |
| 'gia' | Guided Improvement Algorithm (GIA) adapted to leximax |

| Member function | Description |
| ------ | ------ |
| `void set_approx_tout(double t);` | Set a timeout in seconds for the approximation algorithm |
| `void set_gia_incr(bool v);` | Switch on/off the use of fully incremental SAT solving during the GIA |
| `void set_gia_pareto(bool v);` | Switch on/off the search for guaranteed Pareto-optimal solutions during the GIA |
| `void set_mss_add_cls(int v);` | specify how to add the clauses to the MSS in construction during MSS search |
| `void set_mss_incr(bool v);` | Switch on/off the use of fully incremental SAT solving during the MSS search |
| `void set_mss_tol(int t);` | Control the choice of which clause is to be tested next to be added to the MSS |

`void set_mss_tol(int t);` receives an integer t between 0 and 100 (percentage). When finding an MSS using extended linear search, in each iteration, a clause is selected from the set of soft clauses to be tested if it can be added to the MSS in construction. In the multi-objective case, we can select a clause from multiple objective functions. Also, in each iteration, the MSS in construction fixes upper bounds on the objective functions. The choice of the next clause is done via this parameter t. If t <= (max-min)\*100/max, then the next clause is taken from an objective function with the largest upper bound, otherwise, the next clause is chosen sequentially. Here, max and min are the maximum and minimum upper bounds, respectively. Essentially, for small values of t the algorithm tends to select clauses from an objective function with the largest upper bound; and for large values of t the algorithm tends to select clauses from an objective function sequentially.

`void set_mss_add_cls(int v);` accepts the following values: 0, 1, 2. During MSS extended linear search, after each satisfiable SAT solver call, we can add to the MSS in construction any of the satisfied soft clauses. This function sets the parameter which controls how and which of those clauses are actually added to the MSS.
- 0 : add all the satisfied soft clauses to the MSS;
- 1 : add as many satisfied soft clauses as possible while trying to even out the upper bounds of the objective functions;
- 2 : add only the soft clause tested in the SAT call.

Based on experimental data on the Multi-Objective Package Upgradeability Optimisation problem, the best performing approximation algorithm is 'mss' with the following configuration:
```cpp
solver.set_approx('mss');
solver.set_mss_add_cls(1);
solver.set_mss_incr(false);
solver.set_mss_tol(0);
```

## Command-line Tool
Usage: `./leximaxIST [<options>] -h|--help|((--approx <string>)|(--optimise <string>) <input_file>)`.

Example: `./leximaxIST --optimise ilp formula.pbmo` (Find a leximax-optimal solution of the instance described in file `formula.pbmo`, using the ILP-based algorithm.)

Example: `./leximaxIST --timeout 10 --approx mss formula.pbmo` (Find a feasible solution as close to leximax-optimal as possible, using the approximation algorithm based on Maximal Satisfiable Subsets (MSSes), with a time limit of 10 seconds.)

To see all the available options run `./leximaxIST -h`.

### Input Format
The command-line tool reads an input file with the Multi-Objective Boolean Optimisation instance written in PBMO format, which is the same as the Pseudo-Boolean solver input [OPB format](https://www.cril.univ-artois.fr/PB12/format.pdf), but with multiple objective functions. leximaxIST converts the Pseudo-Boolean constraints to CNF (using encodings taken from [Open-WBO](https://github.com/sat-group/open-wbo)) before running the algorithms.

In folder `examples` there is an example PBMO instance `bp-100-20-3-10-10192-SC.pbmo`, from the set covering problem.

### Output Format
The output format is similar to the [MaxSAT evaluation output format](https://maxsat-evaluations.github.io/2022/rules.html).
The solver outputs one of the following lines:

- `s UNSATISFIABLE` (if the input instance is unsatisfiable);

- `s OPTIMUM FOUND` (if a leximax-optimal solution was found);

- `s SATISFIABLE` (if a feasible solution was found that is not guaranteed to be optimal);

- `s UNKNOWN` (in all other cases).

Moreover, whenever the instance is satisfiable, the solver outputs one or more lines (starting with the character 'v') showing the satisfying assignment, as in the MaxSAT evaluation output format.

**Note:** If the verbosity level is set to 1 or higher, the solver also prints lines starting with the character 'o' every time a leximax-better solution is found. The character 'o' is followed by the values of the objective functions for that feasible solution.

#### Signal Handling
The command-line tool, upon receiving one of the signals SIGUSR1, SIGHUP, SIGINT or SIGTERM, prints the solution status and the assignment, in the previously mentioned format, and terminates.

## Examples - Package Upgradeability
The folder `old_packup/examples` contains a package upgradeability benchmark (rand692.cudf). More benchmarks from the [Mancoosi International Solver Competition 2011](https://www.mancoosi.org/misc-2011/index.html) can be found [here](http://data.mancoosi.org/misc2011/problems/).

Example:
```
./packup -u '-removed,-changed' -v1 --leximax-opt core_merge --disjoint-cores examples/rand692.cudf &> solution.txt
```

- The option `-v` sets the verbosity of the output.
`-v1` prints some interesting information about the algorithm, like the CPU time of every SAT call, the values of the objective functions as soon as a leximax-better solution is found,... `-v2` is for debugging. `-v0` only prints the solution.

- The option `-u` sets the objective functions.

packup accepts the following objective functions:

| Objective Function | Description |
| --- | --- |
| `-removed` | Minimise the number of removed packages |
| `-changed` | Minimise the number of changed packages |
| `-notuptodate` | Minimise the number of not-up-to-date packages |
| `-unmet_recommends` | Minimise the number of packages with unsatisfied recommendations |
| `-new` | Minimise the number of newly installed packages |

For the package upgradeability solver mccs, the `unmet_recommends` objective function corresponds to `-nunsat[recommends:]`.

- The option `--leximax-opt` tells the solver to do leximax optimisation with a specific algorithm.

Without the option `--leximax-opt` or the option `--leximax-approx` (to do approximation instead of optimisation), packup optimises using the lexicographic criterion, and an external MaxSAT or PBO solver must be provided via the option `--external-solver`.

Run: `./packup -h` for more information.

The next example shows how to run mccs with Cbc to minimise the objective functions removed, notuptodate and new using the leximax criterion:
```
./mccs -v1 -i <instance> -lp './cbclp' -leximax[-removed,-notuptodate,-new] &> solution.txt
```
where `<instance>` is the input file (e.g. `../old_packup/examples/rand692.cudf`).
