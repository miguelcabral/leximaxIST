CFLAGS = -std=c++17
CFLAGS += -I ../include
SRCS1 = $(wildcard *.cc)
SRCS2 = $(wildcard *.cpp)
OBJS1 = $(SRCS1:.cc=.o)
OBJS2 = $(SRCS2:.cpp=.o)
OBJS := $(OBJS1)
OBJS += $(OBJS2)
LIBNAME = libleximaxIST.a

# Edit to change the path to sat solver include directory containing ipasir.h
CFLAGS += -I ~/solvers/sat/cadical/src

ifdef PROF
	CFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean release debug

release : CFLAGS += -O3 -w
release : $(LIBNAME)

debug : CFLAGS += -g -Wall -Wextra -pedantic
debug : $(LIBNAME)

$(LIBNAME) : $(OBJS)
	@echo "Archiving: $(LIBNAME)"
	ar -rcs ../lib/$(LIBNAME) $(OBJS)
	rm -f *.o

$(OBJS1) : %.o: %.cc
	@echo "Compiling: $@"
	g++ $(CFLAGS) -c -o $@ $<

$(OBJS2) : %.o: %.cpp
	@echo "Compiling: $@"
	g++ $(CFLAGS) -c -o $@ $<
	
clean :
	@echo "Removing: $(LIBNAME) $(OBJS)"
	rm -f ../lib/$(LIBNAME) *.o
	

