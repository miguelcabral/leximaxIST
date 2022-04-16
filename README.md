# leximaxIST
## Description
Boolean Leximax Optimisation solver based on iterative SAT solving.
The solver can either be used as a C++ library or as a command line tool.
By default, the leximax solver uses the SAT solver [CaDiCaL](https://github.com/arminbiere/cadical), but it can easily be replaced by another SAT solver that implements the well-known [IPASIR interface](https://baldur.iti.kit.edu/sat-race-2015/index.php?cat=rules#api).

This repository also contains the package upgradeability solver [packup](https://sat.inesc-id.pt/~mikolas/sw/packup/) [1] (located in the folder `old_packup`). The leximax solver can be linked to packup to solve the Multi-Objective Package Upgradeability Optimisation problem using the leximax criterion.
We also provide a (slightly modified) copy of the source code of [mccs](https://www.i3s.unice.fr/~cpjm/software.html) [2] version 1.1, another package upgradeability solver based on Integer Linear Programming.

## Build
Run `bash install.sh` to set up and install the leximax solver, the SAT solver CaDiCaL, the package upgradeability solvers packup and mccs, and the ILP solver [Cbc](https://github.com/coin-or/Cbc) (release 2.10.7). The executable leximaxIST is in the folder bin and the library is in the folder lib.

packup and leximaxIST dependencies: zlib library.

## Examples - Package Upgradeability
The folder `old_packup/examples` contains a package upgradeability benchmark (rand692.cudf). More benchmarks from the [Mancoosi International Solver Competition 2011](https://www.mancoosi.org/misc-2011/index.html) can be found [here](http://data.mancoosi.org/misc2011/problems/).
Example:
```
./packup -u '-removed,-changed' -v1 --leximax-opt core_merge --disjoint-cores examples/rand692.cudf &> solution.txt
```
The option `-v` sets the verbosity of the output.
`-v1` prints some interesting information about the algorithm, like the CPU time of every SAT call, the values of the objective functions as soon as a leximax-better solution is found,... `-v2` is for debugging. `-v0` only prints the solution.
The option `-u` sets the objective functions. The objective functions can be removed, changed, notuptodate, unmet_recommends or new.
The options `--disjoint-cores` and `--leximax-opt` set the leximax optimisation algorithm.
Without the option `--leximax-opt` or the option `--leximax-approx` (to do approximation instead of optimisation), packup uses the lexicographic criterion, and an external MaxSAT or PBO solver must be provided.
Run: `./packup -h` for more information.
The next example shows how to run mccs with Cbc to minimise the objective functions removed, notuptodate and new using the leximax criterion:
```
./mccs -v1 -i <instance> -lp './cbclp' -leximax[-removed,-notuptodate,-new] &> solution.txt
```
where `<instance>` is the input file (e.g. `../old_packup/examples/rand692.cudf`).

## Examples - PBMO
The command line tool receives as input an instance file in the pbmo format, which is the same as the Pseudo-Boolean input [opb format](https://www.cril.univ-artois.fr/PB12/format.pdf), but with multiple objective functions. The solver converts the Pseudo-Boolean constraints to CNF (using encodings taken from [Open-WBO](https://github.com/sat-group/open-wbo)) and then runs the SAT-based algorithm.

The output format is similar to the [MaxSAT solver format](https://maxsat-evaluations.github.io/2022/rules.html).
The solver outputs the line
```
s UNSATISFIABLE
```
if the input instance is unsatisfiable. It outputs the line
```
s OPTIMUM FOUND
```
if a leximax-optimal solution was found. It outputs the line
```
s SATISFIABLE
```
if the instance is satisfiable but the solver does not know if the best solution so far is optimal. In all other cases, the solver outputs
```
s UNKNOWN
```
Whenever the instance is satisfiable, the solver outputs one or more lines (starting with the character 'v') with the model.
The folder `examples` contains a benchmark of the set covering problem (put here name of file).
Example:
```
./bin/leximaxIST --optimise core_merge --dcs <input-file>
```
Run `./bin/leximaxIST --help` for more information.
## References
[1] Mikolás Janota, Inês Lynce, Vasco M. Manquinho, and João Marques-Silva. Packup: Tools for package upgradability solving. *J. Satisf. Boolean Model. Comput.*, 8(1/2):89–94, 2012.

[2] Claude Michel and Michel Rueher. Handling software upgradeability problems with MILP solvers. In Inês Lynce and Ralf Treinen, editors, *Proceedings First International Workshop on Logics for Component Configuration, LoCoCo 2010, Edinburgh, UK, 10th July 2010,* volume 29 of EPTCS, pages 1–10, 2010.
