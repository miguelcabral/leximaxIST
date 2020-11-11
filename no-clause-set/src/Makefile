-include makeenv

CCFLAGS = -std=c++17
CCFLAGS += -I ../include
CSRCS1 = $(wildcard *.cc)
CSRCS2 = $(wildcard *.cpp)
COBJS1 = $(CSRCS1:.cc=.o)
COBJS2 = $(CSRCS2:.cpp=.o)
COBJS := $(COBJS1)
COBJS += $(COBJS2)

ifndef RELEASE
	CCFLAGS += -Wall -Wextra -pedantic
endif

ifdef DBG
	CCFLAGS+=-g
	CCFLAGS+=-fsanitize=address
	CCFLAGS+=-D_GLIBCXX_DEBUG
else
	CCFLAGS+=-DNDEBUG
	CCFLAGS+=-O3
endif

ifdef PROF
	CCFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean

libleximax.a : $(COBJS)
	@echo "Archiving: libleximax.a"
	@ar -rcs ../lib/libleximax.a $(COBJS)
	@rm -f *.o

$(COBJS1): %.o: %.cc
	@echo "Compiling: $@"
	@g++ $(CCFLAGS) -c -o $@ $<

$(COBJS2): %.o: %.cpp
	@echo "Compiling: $@"
	@g++ $(CCFLAGS) -c -o $@ $<
	
clean :
	@rm -f ../lib/libleximax.a *.o
	@echo "Removing: libleximax.a $(COBJS)"
	

