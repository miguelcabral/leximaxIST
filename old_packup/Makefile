BUILDDATE = 2011-Jun-05 23:23
DISTDATE  = 2011-06-05
CURRDATE  = 20110605
CHANGESET = e09be74b0d54
RELDATE   = 2011605

RAGEL?=ragel
BISON?=bison
CCC?=g++
XTRGT = packup
CSRCS = $(wildcard *.cc) 
COBJS = $(CSRCS:.cc=.o)
LIBD = 
LIBS =
INCD = 
CXX?=g++
CFLAGS += -DEXTERNAL_SOLVER
CFLAGS += -DNDEBUG -DNCHECK
CFLAGS += -std=c++17
#CFLAGS += -Wno-deprecated-register
CFLAGS += -Wall -Wextra -Wno-parentheses 
CFLAGS += -DBUILDDATE='"$(BUILDDATE)"' -DDISTDATE='"$(DISTDATE)"'
CFLAGS += -DCHANGESET='"$(CHANGESET)"' -DRELDATE='"$(RELDATE)"'
# CFLAGS+=-D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -Wno-parentheses -Wno-deprecated
LIBS +=-lz
#LNFLAGS+=-static

# change LEXIMAXDIR to the root directory of the leximax library
LEXIMAXDIR = ~/thesis/no-clause-set
INCD += $(LEXIMAXDIR)/include
LIBD += $(LEXIMAXDIR)/lib
LIBS += -lleximaxIST

# comment out if arbitrary precision not needed
# LIBS+=-lgmpxx -lgmp
# CFLAGS+=-DGMPDEF
# CFLAGS+=-DCONV_DBG

.PHONY: build release objs debug

ifdef PROF
CFLAGS+=-ftest-coverage
CFLAGS+=-fprofile-arcs
CFLAGS+=-pg
CFLAGS+=-g
LNFLAGS+=-fprofile-arcs
LNFLAGS+=-ftest-coverage
LNFLAGS+=-pg
LNFLAGS+=-g
endif


# Miguel: should we put LNFLAGS += -O3 as well?
#release : LNFLAGS += -O3
release : CFLAGS += -O3
release : $(XTRGT)

debug : LNFLAGS += -g
debug : CFLAGS += -g
debug : $(XTRGT)

$(XTRGT): objs 
	@echo Linking: $@
	$(CXX) $(COBJS) $(LNFLAGS) -L $(LIBD) $(LIBS) -o $@ 

objs: Lexer.o p.tab.o $(COBJS) 

depend:
	makedepend -- -I. $(CFLAGS) -- *.cc

## Build rule
%.o:	%.cc *.hh
	@echo Compiling: $@
	$(CXX) -I $(INCD) $(CFLAGS) -c -o $@ $<

##  This  needs bison and ragel, comment in if parsing changes
#Lexer.cc: p.tab.cc  l.rl Lexer.hh
#	$(RAGEL) l.rl -o Lexer.cc

#p.tab.cc: p.bison
#	$(BISON) --defines=p.tab.hh -o p.tab.cc p.bison

clean:
	rm -f $(XTRGT) $(COBJS)      
