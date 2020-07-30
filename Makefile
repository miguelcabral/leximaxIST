leximax : leximax.cpp sorting_net.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc
	g++ -o leximax leximax.cpp sorting_net.cpp old_packup/ReadCNF.cc old_packup/cl_registry.cc -lz
	
clean :
	rm -f leximax
