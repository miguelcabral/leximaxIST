CFLAGS = -std=c++17
CFLAGS += -I ../include -I ./cmd_line -I ./cmd_line/encodings
SRCSLIB = $(wildcard *.cpp)
SRCSTOOL = $(SRCSLIB)
SRCSTOOL += $(wildcard cmd_line/encodings/*.cpp)
SRCSTOOL += $(wildcard cmd_line/*.cpp)
OBJSLIB = $(SRCSLIB:.cpp=.o)
OBJSTOOL = $(SRCSTOOL:.cpp=.o)
LIBNAME = libleximaxIST.a
TOOLNAME = leximaxIST

# path to sat solver library and to leximaxIST library
LIBDIR = -L ../lib -L ../cadical/build

# libraries' names
LIBS = -lz -lcadical -lleximaxIST

# path to sat solver include directory containing ipasir.h
CFLAGS += -I ../cadical/src

ifdef PROF
	CFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
	LFLAGS+= -g -pg -fprofile-arcs -ftest-coverage
endif

.PHONY: clean release debug

release : CFLAGS += -O3 -w
release : LFLAGS += -O3
release : $(LIBNAME) $(TOOLNAME)

debug : CFLAGS += -g -Wall -Wextra -pedantic
debug : LFLAGS += -g
debug : $(LIBNAME) $(TOOLNAME)

$(LIBNAME) : $(OBJSLIB)
	@echo "Archiving: $(LIBNAME)"
	ar -rcs ../lib/$(LIBNAME) $(OBJSLIB)
	
$(TOOLNAME) : $(OBJSTOOL) $(LIBNAME)
	@echo "Linking: $@"
	g++ $(OBJSTOOL) $(LFLAGS) $(LIBDIR) $(LIBS) -o $@

$(OBJSTOOL) : %.o: %.cpp
	@echo "Compiling: $@"
	g++ $(CFLAGS) -c -o $@ $<
	
clean :
	@echo "Removing: $(LIBNAME) $(OBJS)"
	rm -f ../lib/$(LIBNAME) ../bin/$(TOOLNAME) $(OBJSTOOL)
	
