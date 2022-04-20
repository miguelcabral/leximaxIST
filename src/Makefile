CFLAGS = -std=c++17
CFLAGS += -I ../include -I ./cmd_line -I ./encodings
SRCSLIB = $(wildcard *.cpp)
SRCSLIB += $(wildcard encodings/*.cpp)
SRCSTOOL = $(SRCSLIB)
SRCSTOOL += $(wildcard cmd_line/*.cpp)
OBJSLIB = $(SRCSLIB:.cpp=.o)
OBJSTOOL = $(SRCSTOOL:.cpp=.o)
#OBJSTOOL := $(filter-out cmd_line/encodings/Enc_KPA.o,$(OBJSTOOL))
LIB = ../lib/libleximaxIST.a
TOOL = ../bin/leximaxIST

# path to sat solver library and to leximaxIST library
LN_PATHS = -L ../lib -L ../cadical/build

# libraries' names
LN_NAMES = -lz -lcadical -lleximaxIST

# path to sat solver include directory containing ipasir.h
CFLAGS += -I ../cadical/src

ifdef PROF
	CFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
	LNFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean release debug

release : CFLAGS += -O3 -w
release : LNFLAGS += -O3
release : $(LIB) $(TOOL)

debug : CFLAGS += -g -Wall -Wextra -pedantic
debug : LNFLAGS += -g
debug : $(LIB) $(TOOL)

$(LIB) : $(OBJSLIB)
	@echo "Archiving: $(LIB)"
	ar -rcs $(LIB) $(OBJSLIB)
	
$(TOOL) : $(OBJSTOOL) $(LIB)
	@echo "Linking: $@"
	g++ $(OBJSTOOL) $(LNFLAGS) $(LN_PATHS) $(LN_NAMES) -o $@

$(OBJSTOOL) : %.o: %.cpp
	@echo "Compiling: $@"
	g++ $(CFLAGS) -c -o $@ $<
	
clean :
	@echo "Removing: $(LIB) $(TOOL) $(OBJS)"
	rm -f $(LIB) $(TOOL) $(OBJSTOOL)
	
