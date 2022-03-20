# leximaxIST
## Description
C++ library for solving leximax optimisation using iterative SAT solving.
To use the library and test it on some examples, the library can be linked to the package upgradeability solver [packup](https://sat.inesc-id.pt/~mikolas/sw/packup/) [1], located in the folder old_packup.
By default, packup solves leximax with the SAT solver [CaDiCaL](https://github.com/arminbiere/cadical).
We also provide a copy of the source code of [mccs](https://www.i3s.unice.fr/~cpjm/software.html) [2] version 1.1, another package upgradeability solver based on Integer Linear Programming.

## Build
Run `bash install.sh` to set up and install the C++ library, the SAT solver CaDiCaL, the package upgradeability solvers packup and mccs, and the ILP solver [Cbc](https://github.com/coin-or/Cbc) (release 2.10.7).
This command will take some time.

packup and leximaxIST dependencies: zlib library.

## Examples
The folder old_packup/examples contains a package upgradeability benchmark (rand692.cudf). More benchmarks from the [Mancoosi International Solver Competition 2011](https://www.mancoosi.org/misc-2011/index.html) can be found [here](http://data.mancoosi.org/misc2011/problems/).
Example:
```
./packup -u '-removed,-changed' --leximax -v1 --disjoint-cores --opt-mode 'core-merge' examples/rand692.cudf &> solution.txt
```
The option -v sets the verbosity of the output.
-v1 prints some interesting information about the algorithm, like the CPU time of every SAT call, the values of the objective functions as soon as a leximax-better solution is found,... -v2 is for debugging. -v0 only prints the solution.
The option -u sets the objective functions. The objective functions can be removed, changed, notuptodate, unmet_recommends or new.
The options --disjoint-cores and --opt-mode set the leximax optimisation algorithm.
Without the option --leximax the solver will solve using the lexicographic criterion, and an external MaxSAT or PBO solver must be provided.
Run: `./packup -h` for more information.
The next example shows how to run mccs with Cbc to minimise the objective functions removed, notuptodate and new using the leximax criterion:
```
./mccs -v1 -i <instance> -lp './cbclp' -leximax[-removed,-notuptodate,-new] &> solution.txt
```
where `<instance>` is the input file (e.g. `../old_packup/examples/rand692.cudf`).

## References
[1] Mikolás Janota, Inês Lynce, Vasco M. Manquinho, and João Marques-Silva. Packup: Tools for package upgradability solving. *J. Satisf. Boolean Model. Comput.*, 8(1/2):89–94, 2012.

[2] Claude Michel and Michel Rueher. Handling software upgradeability problems with MILP solvers. In Inês Lynce and Ralf Treinen, editors, *Proceedings First International Workshop on Logics for Component Configuration, LoCoCo 2010, Edinburgh, UK, 10th July 2010,* volume 29 of EPTCS, pages 1–10, 2010.
