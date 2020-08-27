leximax : main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp printing.cpp solver_call.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc
	g++ -Wall -Wextra -o leximax main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp printing.cpp solver_call.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc -lz
	
clean :
	rm -f leximax
