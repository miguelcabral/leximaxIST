This tool uses a MaxSAT-based algorithm to compute a leximax-optimal solution of a SAT problem with multiple objective functions.
The SAT constraints are assumed to be in a .cnf file in DIMACS format. Each objective function is also assumed to be in a .cnf file in DIMACS format. Each objective function is assumed to be the sum of the **falsified** clauses in the .cnf file.
The tool finds a leximax-optimal solution by calling successively a MaxSAT or PBO solver. By default the tool uses the MaxSAT solver RC2. To install this solver one must have python and pip installed and then run the following command `pip install python-sat`. It is also possible to use other solvers. For more information run `./leximax -h`.

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
