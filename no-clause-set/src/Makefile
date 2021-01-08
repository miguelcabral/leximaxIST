CFLAGS = -std=c++17
CFLAGS += -I ../include
SRCS1 = $(wildcard *.cc)
SRCS2 = $(wildcard *.cpp)
OBJS1 = $(SRCS1:.cc=.o)
OBJS2 = $(SRCS2:.cpp=.o)
OBJS := $(OBJS1)
OBJS += $(OBJS2)

ifdef PROF
	CFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean release debug

release : CFLAGS += -O3 -w
release : SEDSTR=s/m_debug(true)/m_debug(false)/g
release : libleximax.a

debug : CFLAGS += -g -Wall -Wextra -pedantic
debug : SEDSTR=s/m_debug(false)/m_debug(true)/g
debug : libleximax.a

libleximax.a : $(OBJS)
	@echo "Archiving: libleximax.a"
	@ar -rcs ../lib/libleximax.a $(OBJS)
	@rm -f *.o

$(OBJS1) : %.o: %.cc
	@echo "Compiling: $@"
	@g++ $(CFLAGS) -c -o $@ $<

$(OBJS2) : %.o: %.cpp
	@echo "Compiling: $@"
	@sed -i '$(SEDSTR)' constructors.cpp
	@g++ $(CFLAGS) -c -o $@ $<
	
clean :
	@rm -f ../lib/libleximax.a *.o
	@echo "Removing: libleximax.a $(OBJS)"
	

