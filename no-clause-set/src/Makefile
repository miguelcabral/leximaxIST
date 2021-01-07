CCFLAGS = -std=c++17
CCFLAGS += -I ../include
CSRCS1 = $(wildcard *.cc)
CSRCS2 = $(wildcard *.cpp)
COBJS1 = $(CSRCS1:.cc=.o)
COBJS2 = $(CSRCS2:.cpp=.o)
COBJS := $(COBJS1)
COBJS += $(COBJS2)

ifdef PROF
	CCFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean release debug

release : CCFLAGS += -O3 -w
release : libleximax.a

debug : CCFLAGS += -g -fsanitize=address -D_GLIBCXX_DEBUG -D DEBUG -Wall -Wextra -pedantic
debug : libleximax.a

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
	

