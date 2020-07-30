This tool uses a MaxSAT-based algorithm to compute a leximax-optimal solution of a SAT problem with multiple objective functions.
The SAT constraints are assumed to be in a .cnf file in DIMACS format. Each objective function is also assumed to be in a .cnf file in DIMACS format. That is, each objective function is assumed to be the sum of the clauses in the .cnf file.

Example: The objective function x_1 + x_2 + (not(x_3) or x_4) is written in DIMACS as

p cnf 4 3
1 0
2 0
-3 4 0

Commands:
To compile: make
To run: ./leximax <SAT-constraints-file> <objective1-file> <objective2-file> ...
To remove leximax: make clean
