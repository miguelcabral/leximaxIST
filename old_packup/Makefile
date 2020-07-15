BUILDDATE = 2011-Jun-05 23:23
DISTDATE  = 2011-06-05
CURRDATE  = 20110605
CHANGESET = e09be74b0d54
RELDATE   = 2011605

RAGEL?=ragel
BISON?=bison
CCC?=g++
XTRGT = packup
CSRCS      = $(wildcard *.cc) 
COBJS      = $(CSRCS:.cc=.o)
LIBD =
LIBS =
CXX?=g++
CFLAGS+=-DEXTERNAL_SOLVER
CFLAGS+=-DNDEBUG -DNCHECK
CFLAGS+=-O3
CFLAGS += -Wall -DBUILDDATE='"$(BUILDDATE)"' -DDISTDATE='"$(DISTDATE)"'
CFLAGS += -DCHANGESET='"$(CHANGESET)"' -DRELDATE='"$(RELDATE)"'
CFLAGS+=-D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -Wno-parentheses -Wno-deprecated
LIBS+=-lz
#LNFLAGS+=-static

# comment out if arbitrary precision not needed
# LIBS+=-lgmpxx -lgmp
# CFLAGS+=-DGMPDEF
# CFLAGS+=-DCONV_DBG

.PHONY: build all objs

all: $(XTRGT)

$(XTRGT): objs 
	@echo Linking: $@
	@$(CXX) $(COBJS) $(LNFLAGS) $(LIBD) $(LIBS) -o $@ 

objs: Lexer.o p.tab.o $(COBJS) 

depend:
	makedepend -- -I. $(CFLAGS) -- *.cc

## Build rule
%.o:	%.cc *.hh
	@echo Compiling: $@
	@$(CXX)  -I. $(CFLAGS) -c -o $@ $<

##  This  needs bison and ragel, comment in if parsing changes
#Lexer.cc: p.tab.cc  l.rl Lexer.hh
#	$(RAGEL) l.rl -o Lexer.cc

#p.tab.cc: p.bison
#	$(BISON) --defines=p.tab.hh -o p.tab.cc p.bison

clean:
	rm -f $(XTRGT) $(COBJS)      
