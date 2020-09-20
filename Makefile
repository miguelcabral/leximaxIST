-include makeenv

CCFLAGS=-Wall -Wextra -std=c++17 -pedantic
CSRCS1 = $(wildcard *.cc)
CSRCS2 = $(wildcard *.cpp)
COBJS1 = $(CSRCS1:.cc=.o)
COBJS2 = $(CSRCS2:.cpp=.o)
COBJS := $(COBJS1)
COBJS += $(COBJS2)

ifdef DBG
	CCFLAGS+=-g
	CCFLAGS+=-fsanitize=address
	CCFLAGS+=-D_GLIBCXX_DEBUG
else
	CCFLAGS+=-DNDEBUG
	CCFLAGS+=-O3
endif

.PHONY: all info clean

all: info

info: leximax
	$(info DBG Mode : $(DBG))

leximax: $(COBJS)
	@echo Linking: $@
	g++ -o leximax $(COBJS) -lz

$(COBJS1): %.o: %.cc
	@echo Compiling: $@
	g++ $(CCFLAGS) -c -o $@ $<

$(COBJS2): %.o: %.cpp
	@echo Compiling: $@
	g++ $(CCFLAGS) -c -o $@ $<
	
#leximax : main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp \
		printing.cpp solver_call.cpp ReadCNF.cc cl_registry.cc Options.cc
#	g++ $(CCFLAGS) -o leximax main.cpp verify.cpp sorting_net.cpp encoding.cpp reading.cpp \
		printing.cpp solver_call.cpp ReadCNF.cc cl_registry.cc Options.cc -lz
	
clean :
	rm -f leximax *.o
