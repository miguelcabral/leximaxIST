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

- Compile statically mccs (with lpsolve ILP solver):
1. Compile lpsolve solver: 
    1. Go to directory `mccs-1.1/lp_solve_5.5/lp_solve` and run `sh ccc`
    1. Go to directory `mccs-1.1/lp_solve_5.5/lpsolve55` and run `sh ccc`
1. You will need to put the libraries in a place where the linker will find them: `cp mccs-1.1/lp_solve_5.5/lpsolve55/bin/ux64/liblpsolve55.* /usr/lib`
1. Finally compile mccs with `make clean && make mccs-static`

- Use mccs with trendy criterium (without the recommends property): `./mccs -v1 -i input.cudf -leximax[-removed,-notuptodate,-new] &> mccs-trendy.txt`

- Use packup with trendy criterium (possibly without the recommends property): `./packup -t --max-sat --leximax --external-solver 'rc2.py -vv' input.cudf &> packup-trendy.txt`

- Get Mancoosi benchmarks: wget -q -r -l 1 -nd http://data.mancoosi.org/misc2011/problems/dudf-random
