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

.PHONY: clean

libleximax.a : $(COBJS)
	@echo Archiving: $(COBJS)
	ar -rcs libleximax.a $(COBJS)
	rm -f *.o

$(COBJS1): %.o: %.cc
	@echo Compiling: $@
	g++ $(CCFLAGS) -c -o $@ $<

$(COBJS2): %.o: %.cpp
	@echo Compiling: $@
	g++ $(CCFLAGS) -c -o $@ $<
	
clean :
	rm -f libleximax.a *.o
	

