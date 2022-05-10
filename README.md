# leximaxIST
## Description
Boolean Leximax Optimisation solver.
Given a Multi-Objective Boolean Optimisation formula, the solver can be used to find a leximax-optimal solution or to approximate a leximax-optimal solution.
The solver can either be used as a C++ library or as a command-line tool.

The C++ library currently only solves Multi-Objective MaxSAT (i.e. no Pseudo-Boolean constraints).
The command-line tool reads PBMO format, which is the same as the Pseudo-Boolean input [OPB format](https://www.cril.univ-artois.fr/PB12/format.pdf), but with multiple objective functions.
The tool converts the Pseudo-Boolean constraints to clauses and then calls the library.
Although leximaxIST accepts weighted objective functions, it currently performs best in unweighted instances.

The solver implements several algorithms based on iterative SAT solving, and an algorithm based on iterative ILP solving described in this [report](https://www.mancoosi.org/reports/d4.3.pdf), adapted from a CP algorithm by Bouveret and Lemaître [3].

By default, leximaxIST uses the SAT solver [CaDiCaL](https://github.com/arminbiere/cadical), but it can easily be replaced by another SAT solver that implements the well-known [IPASIR interface](https://baldur.iti.kit.edu/sat-race-2015/index.php?cat=rules#api).
leximaxIST supports the ILP solvers [Gurobi](https://www.gurobi.com/) and [CPLEX](https://www.ibm.com/analytics/cplex-optimizer).

This repository also contains the package upgradeability solver [packup](https://sat.inesc-id.pt/~mikolas/sw/packup/) [1] (located in the folder `old_packup`). The leximax solver can be linked to packup to solve the Multi-Objective Package Upgradeability Optimisation problem using the leximax criterion.
We also provide a (slightly modified) copy of the source code of [mccs](https://www.i3s.unice.fr/~cpjm/software.html) [2] version 1.1, another package upgradeability solver based on Integer Linear Programming.

## Build
Run `bash install.sh` to set up and install the leximax solver, the SAT solver CaDiCaL, the package upgradeability solvers packup and mccs, and the ILP solver (for mccs) [Cbc](https://github.com/coin-or/Cbc) (release 2.10.7). The executable `leximaxIST` is in the folder `bin` and the library `libleximaxIST.a` is in the folder `lib`.

packup and leximaxIST dependencies: zlib library.

## Tutorial
A small guide on how to use the solver is provided in the file tutorial.md.

## References
[1] Mikolás Janota, Inês Lynce, Vasco M. Manquinho, and João Marques-Silva. Packup: Tools for package upgradability solving. *J. Satisf. Boolean Model. Comput.*, 8(1/2):89–94, 2012.

[2] Claude Michel and Michel Rueher. Handling software upgradeability problems with MILP solvers. In Inês Lynce and Ralf Treinen, editors, *Proceedings First International Workshop on Logics for Component Configuration, LoCoCo 2010, Edinburgh, UK, 10th July 2010,* volume 29 of EPTCS, pages 1–10, 2010.

[3] Sylvain Bouveret and Michel Lemaître. Computing leximin-optimal solutions in constraint networks. *Artificial Intelligence*, 173(2):343–364, 2009.
