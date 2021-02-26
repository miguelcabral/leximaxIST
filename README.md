# LeximaxIST
## Description
This software is an implementation of an algorithm (citar paper?) to solve optimisation problems with Boolean (1-0) variables and propositional constraints, and multiple objective functions with the leximax criterium.

The algorithm works by solving a sequence of single-objective problems. Each problem is solved by calling an external MaxSAT or Pseudo-Boolean Optimisation solver.

The software can be used as a command line tool or as a C++ static library.

## Command line tool
Usage: `./leximaxIST [options] <constraints> <objective-function> [<objective-function>]... `
The input arguments are files in [DIMACS format](http://www.satcompetition.org/2009/format-benchmarks2009.html).
The first two files are mandatory. The first file contains the constraints. The second and subsequent files are the objective functions. Each objective function is assumed to be the sum of the **falsified** clauses in the DIMACS file.
### Options


### Output format
Similar to the MaxSAT solver's [output format](https://maxsat-evaluations.github.io/2020/rules.html#output), except in this case, we have multiple objective functions, hence we output an objective vector, instead of an objective value.
Lines can either start with one of the following characters: 'c', 's', 'v', 'o'.
Lines starting with 'c' are comments and can be ignored.
The line starting with 's' can be one of the following:
1. s SATISFIABLE
In case a satisfying assignment was found.
1. s UNSATISFIABLE
In case there does not exist a satisfying assignment to the problem.

In case the problem is satisfiable, a line starting with 'o' will be printed showing the



## C++ static library
The library contains a class called Leximax_encoder (see library/include/Leximax_encoder.h).
### How to use the Leximax_encoder class to solve problems
An object of this class can be created given the constraints and objective functions.
The variables of the problem are represented in [DIMACS format](http://www.satcompetition.org/2009/format-benchmarks2009.html).
That is, each variable is identified by a positive integer. Each literal is an integer. The absolute value is the id of a variable, and if the literal is negative, then it represents the negation of the variable, and if it is positive, then it represents the variable.
The constructor of the Leximax_encoder class receives two arguments:
* The constraints;

* The objective functions.

as a vector of vector of integers. Each constraint is a clause, stored as a vector of integers. Each entry of a clause is a literal (an integer).
The constructor of the Leximax_encoder class receives


## External Solvers
(Dividir por formalismos: wcnf (MaxSAT), opb (PBO), lp (Linear Programming) )
The software has been tested with the following external solvers:

* RC2
* MaxHS
* rounding-sat
* UwrMaxSAT??
* gurobi

# Gurobi
To use the Gurobi command line tool you must follow the steps in the [Software Installation Guide](https://www.gurobi.com/documentation/9.1/quickstart_linux/software_installation_guid.html) of Gurobi.
Note: Each time you want to execute a program compiled with the LeximaxIST library, using Gurobi as external solver, you must previously set the environment variables as explained in the guide.

By default the tool uses the MaxSAT solver RC2.

# RC2
To install this MaxSAT solver you must have python and pip installed and then run the following command `pip install python-sat`.
You can also get the code from the [MaxSAT Evaluation web page] (https://maxsat-evaluations.github.io/2020/descriptions.html).
Em cima foi instalação. Em baixo já tem a ver com a utilização. Dividir estes assuntos - instalação e utilização.
Para ser mais simples: mostrar os comandos dos solvers (com opções default) para o utilizador não ter de fazer essa pesquisa se não estiver familiarizado com os MaxSAT solvers. Senão a pessoa tem de ir ver quais são os comandos. Interessa ver os comandos quando se quer mudar por exemplo o algoritmo do solver ou certos parâmetros.
To execute the solver run the following command `rc2.py -vv`. For the list of options run `rc2.py --help`.

## Build
Instruções para compilar a ferramenta e a biblioteca.

### Dependencies
zlib (reading files), a sat solver that implements the IPASIR interface (finding MSSes to get upper bound of first optimum).

#### Example:
The objective function x_1 + x_2 + x_3 can be specified in a .cnf file as follows.

```
p cnf 3 3
-1 0
-2 0
-3 0
```

#### Commands:
- To compile: `make`

- To print usage: `./leximax --help` or `./leximax -h`

- To remove leximax: `make clean`

- enumerate models, use `wget http://sat.inesc-id.pt/~mikolas/sw/pienum/pienum && chmod u+w pienum`

- Compile statically mccs (with lpsolve ILP solver):
1. Compile lpsolve solver: 
    1. Go to directory `mccs-1.1/lp_solve_5.5/lp_solve` and run `sh ccc`
    1. Go to directory `mccs-1.1/lp_solve_5.5/lpsolve55` and run `sh ccc`
1. You will need to put the libraries in a place where the linker will find them: `cp mccs-1.1/lp_solve_5.5/lpsolve55/bin/ux64/liblpsolve55.* /usr/lib`
1. Finally compile mccs with `make clean && make mccs-static`

- Use mccs with trendy criterium (without the recommends property): `./mccs -v1 -i input.cudf -leximax[-removed,-notuptodate,-new] &> mccs-trendy.txt`

- Use packup with trendy criterium (possibly without the recommends property): `./packup -t --max-sat --leximax --external-solver 'rc2.py -vv' input.cudf &> packup-trendy.txt`

- Get Mancoosi benchmarks: wget -q -r -l 1 -nd http://data.mancoosi.org/misc2011/problems/dudf-random
