-include makeenv

CCFLAGS=-Wall -Wextra -std=c++17 -pedantic

ifdef DBG
	CCFLAGS+=-g
	CCFLAGS+=-fsanitize=address
	CCFLAGS+=-D_GLIBCXX_DEBUG
else
	CCFLAGS+=-DNDEBUG
	CCFLAGS+=-O3
endif

all: info

info: leximax
	$(info DBG Mode : $(DBG))

leximax : main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp printing.cpp solver_call.cpp ReadCNF.cc cl_registry.cc Options.cc
	g++ $(CCFLAGS) -o leximax main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp printing.cpp solver_call.cpp ReadCNF.cc cl_registry.cc Options.cc -lz
	
clean :
	rm -f leximax
