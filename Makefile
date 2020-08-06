leximax : main.cpp sorting_net.cpp Leximax_encoder.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc
	g++ -Wall -Wextra -o leximax main.cpp sorting_net.cpp Leximax_encoder.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc -lz
	
clean :
	rm -f leximax
