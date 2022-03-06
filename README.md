# LeximaxIST
## Description
C++ library for solving leximax optimisation using iterative SAT solving.
To use the library and test it on some examples, the library can be linked to the package upgradeability solver [packup](https://sat.inesc-id.pt/~mikolas/sw/packup/), located in the folder old_packup.
By default, packup uses the SAT solver [CaDiCaL](https://github.com/arminbiere/cadical).
We also provide a copy of the source code of [mccs](https://www.i3s.unice.fr/~cpjm/software.html)~[1], another package upgradeability solver based on Integer Linear Programming solving.

## Build
Run `bash install.sh` to set up and install the C++ library, the SAT solver CaDiCaL, the package upgradeability solvers packup and mccs, and the ILP solver [Cbc](https://github.com/coin-or/Cbc).
This command may take some time.

Dependencies: zlib library must be installed (if the packup compilation fails, this may be a reason why).

## Examples
In old_packup/examples there are a few package upgradeability benchmarks, from the [Mancoosi International Solver Competition 2011](https://www.mancoosi.org/misc-2011/index.html). These benchmarks are stored in files with extension .cudf, meaning Common Upgradeability Description Format.
Example:
```
./packup -u '-removed,-changed' --leximax -v1 --disjoint-cores --opt-mode 'core-merge' examples/rand477.cudf &> solution.txt
```
The option -v sets the verbosity of the output.
-v1 prints some interesting information about the algorithm, like the CPU time of every SAT call, the values of the objective functions as soon as a leximax-better solution is found,... -v2 is for debugging. -v0 prints nothing.
The option -u sets the objective functions. The objective functions can be removed, changed, notuptodate, unmet_recommends or new.
The options --disjoint-cores and --opt-mode refer to the leximax optimisation algorithm.
Without the option --leximax the solver will solve using the lexicographic criterium, and an external MaxSAT or PBO solver must be provided.
Run: `./packup -h` for more information.
To run mccs with Cbc to minimise the objective functions removed, notuptodate and new using the leximax criterium:
```
./mccs -v1 -i <instance> -lp './cbclp' -leximax[-removed,-notuptodate,-new] &> solution.txt
```
where `<instance>` is the input file (e.g. `../old_packup/examples/rand477.cudf`).

[1] Claude Michel, Michel Rueher. Handling software upgradeability problems with MILP solvers. In Inês Lynce and Ralf Treinen, editors, *Proceedings First International Workshop on Logics for Component Configuration, LoCoCo 2010, Edinburgh, UK, 10th July 2010,* volume 29 of EPTCS, pages 1–10, 2010.
