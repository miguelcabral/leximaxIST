
# make for all systems
# make USEGLPK=1 USELPSOLVE=1 USECPLEX=1 USEGUROBI=1

# WARNING: You should not modify this makefile. See make.local to define your own settings.

#----------------------------------------------------------------------------------

include make.local

#----------------------------------------------------------------------------------

OBJS=	$(OBJDIR)/cudf.o \
	$(OBJDIR)/constraint_generation.o \
        $(OBJDIR)/lp_solver.o $(OBJDIR)/pblib_solver.o \
	$(OBJDIR)/removed_criteria.o $(OBJDIR)/changed_criteria.o $(OBJDIR)/new_criteria.o $(OBJDIR)/notuptodate_criteria.o $(OBJDIR)/nunsat_criteria.o \
	$(OBJDIR)/count_criteria.o $(OBJDIR)/unaligned_criteria.o \
	$(OBJDIR)/lexicographic_combiner.o $(OBJDIR)/lexagregate_combiner.o $(OBJDIR)/agregate_combiner.o $(OBJDIR)/lexsemiagregate_combiner.o \
	$(OBJDIR)/leximin_combiner.o $(OBJDIR)/leximax_combiner.o $(OBJDIR)/lexleximin_combiner.o $(OBJDIR)/lexleximax_combiner.o \
	$(OBJDIR)/cudf_reductions.o

CCC+=


ifeq ($(USEGLPK),1)
  OBJS += $(OBJDIR)/glpk_solver.o
  CCC += -DUSEGLPK
else
  GLPKLD =
endif

ifeq ($(USELPSOLVE),1)
  OBJS += $(OBJDIR)/lpsolve_solver.o
  CCC += -DUSELPSOLVE
else
  LPSOLVELD =
endif

ifeq ($(USECPLEX),1)
  OBJS += $(OBJDIR)/cplex_solver.o
  CCC += -DUSECPLEX
else
  CPXLD =
endif

$(info GUROBILD : $(GUROBILD))

$(info USEGUROBI : $(USEGUROBI))
ifeq ($(USEGUROBI),1)
  OBJS += $(OBJDIR)/gurobi_solver.o
  CCC += -DUSEGUROBI
else
  GUROBILD =
endif
# gurobi build:
# miguel: eu retirei a opcao -lfl antes de -L.
mccs-gurobi:  $(OBJS) libccudf.a
	$(CCC) -o mccs $(OBJS) -L. -lccudf $(GUROBILD)

# to make mccs solver
mccs:  $(OBJS) libccudf.a
	$(CCC) -o mccs $(OBJS) -lfl -L. -lccudf \
		$(CPXLD) \
		$(GUROBILD) \
		$(LPSOLVELD) \
		$(GLPKLD)

# to make a light version of mccs solver which does not include libccudf
mccs-light:  $(OBJS) libccudf.so
	$(CCC) -o mccs $(OBJS) -lfl -L. -lccudf -Wl,-rpath,$(PWD) \
		$(CPXLD) \
		$(GUROBILD) \
		$(LPSOLVELD) \
		$(GLPKLD)

# to make a static version of mccs solver (cannot be done with Gurobi: no static library)
mccs-static: $(OBJS) libccudf.a
	$(CCC) -o mccs -static $(OBJS) -lfl -L. -lccudf \
		$(CPXLD) \
		$(GUROBILD) \
		$(LPSOLVELD) \
		$(GLPKLD) \
		-ldl

#----------------------------------------------------------------------------------

$(OBJDIR)/cudf.o: $(SRCDIR)/cudf.c $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/cudf.c -o $(OBJDIR)/cudf.o

$(OBJDIR)/constraint_generation.o: $(SRCDIR)/constraint_generation.c $(SRCDIR)/constraint_generation.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/constraint_generation.c -o $(OBJDIR)/constraint_generation.o

$(OBJDIR)/pblib_solver.o: $(SRCDIR)/pblib_solver.c $(SRCDIR)/pblib_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/pblib_solver.c -o $(OBJDIR)/pblib_solver.o

$(OBJDIR)/lp_solver.o: $(SRCDIR)/lp_solver.c $(SRCDIR)/lp_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lp_solver.c -o $(OBJDIR)/lp_solver.o

$(OBJDIR)/cplex_solver.o: $(SRCDIR)/cplex_solver.c $(SRCDIR)/cplex_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -I$(CPXINC) $(SRCDIR)/cplex_solver.c -o $(OBJDIR)/cplex_solver.o

$(OBJDIR)/gurobi_solver.o: $(SRCDIR)/gurobi_solver.c $(SRCDIR)/gurobi_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -I$(GUROBIINC) $(SRCDIR)/gurobi_solver.c -o $(OBJDIR)/gurobi_solver.o

$(OBJDIR)/lpsolve_solver.o: $(SRCDIR)/lpsolve_solver.c $(SRCDIR)/lpsolve_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -I$(LPSOLVEINC) $(SRCDIR)/lpsolve_solver.c -o $(OBJDIR)/lpsolve_solver.o

$(OBJDIR)/glpk_solver.o: $(SRCDIR)/glpk_solver.c $(SRCDIR)/glpk_solver.h \
	$(SRCDIR)/abstract_solver.h $(SRCDIR)/scoeff_solver.h  $(SRCDIR)/cudf_types.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -I$(GLPKINC) $(SRCDIR)/glpk_solver.c -o $(OBJDIR)/glpk_solver.o

$(OBJDIR)/removed_criteria.o: $(SRCDIR)/removed_criteria.c $(SRCDIR)/removed_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/removed_criteria.c -o $(OBJDIR)/removed_criteria.o

$(OBJDIR)/changed_criteria.o: $(SRCDIR)/changed_criteria.c $(SRCDIR)/changed_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/changed_criteria.c -o $(OBJDIR)/changed_criteria.o

$(OBJDIR)/new_criteria.o: $(SRCDIR)/new_criteria.c $(SRCDIR)/new_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/new_criteria.c -o $(OBJDIR)/new_criteria.o

$(OBJDIR)/notuptodate_criteria.o: $(SRCDIR)/notuptodate_criteria.c $(SRCDIR)/notuptodate_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/notuptodate_criteria.c -o $(OBJDIR)/notuptodate_criteria.o

$(OBJDIR)/nunsat_criteria.o: $(SRCDIR)/nunsat_criteria.c $(SRCDIR)/nunsat_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/nunsat_criteria.c -o $(OBJDIR)/nunsat_criteria.o

$(OBJDIR)/count_criteria.o: $(SRCDIR)/count_criteria.c $(SRCDIR)/count_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/count_criteria.c -o $(OBJDIR)/count_criteria.o

$(OBJDIR)/unaligned_criteria.o: $(SRCDIR)/unaligned_criteria.c $(SRCDIR)/unaligned_criteria.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/unaligned_criteria.c -o $(OBJDIR)/unaligned_criteria.o

$(OBJDIR)/lexicographic_combiner.o: $(SRCDIR)/lexicographic_combiner.c $(SRCDIR)/lexicographic_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lexicographic_combiner.c -o $(OBJDIR)/lexicographic_combiner.o

$(OBJDIR)/lexagregate_combiner.o: $(SRCDIR)/lexagregate_combiner.c $(SRCDIR)/lexagregate_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lexagregate_combiner.c -o $(OBJDIR)/lexagregate_combiner.o

$(OBJDIR)/agregate_combiner.o: $(SRCDIR)/agregate_combiner.c $(SRCDIR)/agregate_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCDIR)/abstract_criteria.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/agregate_combiner.c -o $(OBJDIR)/agregate_combiner.o

$(OBJDIR)/lexsemiagregate_combiner.o: $(SRCDIR)/lexsemiagregate_combiner.c $(SRCDIR)/lexsemiagregate_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lexsemiagregate_combiner.c -o $(OBJDIR)/lexsemiagregate_combiner.o

$(OBJDIR)/leximin_combiner.o: $(SRCDIR)/leximin_combiner.c $(SRCDIR)/leximin_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/leximin_combiner.c -o $(OBJDIR)/leximin_combiner.o

$(OBJDIR)/leximax_combiner.o: $(SRCDIR)/leximax_combiner.c $(SRCDIR)/leximax_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/leximax_combiner.c -o $(OBJDIR)/leximax_combiner.o

$(OBJDIR)/lexleximin_combiner.o: $(SRCDIR)/lexleximin_combiner.c $(SRCDIR)/lexleximin_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lexleximin_combiner.c -o $(OBJDIR)/lexleximin_combiner.o

$(OBJDIR)/lexleximax_combiner.o: $(SRCDIR)/lexleximax_combiner.c $(SRCDIR)/lexleximax_combiner.h $(SRCDIR)/abstract_combiner.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/lexleximax_combiner.c -o $(OBJDIR)/lexleximax_combiner.o

$(OBJDIR)/cudf_reductions.o: $(SRCDIR)/cudf_reductions.c $(SRCDIR)/cudf_reductions.h $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) $(SRCDIR)/cudf_reductions.c -o $(OBJDIR)/cudf_reductions.o



#----------------------------------------------------------------------------------

libccudf.a: $(OBJDIR)/cudf.tab.o $(OBJDIR)/cudf.l.o $(OBJDIR)/cudf_hash_table.o $(OBJDIR)/cudf_tools.o
	ar rcs libccudf.a $(OBJDIR)/cudf.tab.o $(OBJDIR)/cudf.l.o $(OBJDIR)/cudf_hash_table.o $(OBJDIR)/cudf_tools.o

libccudf.so: $(OBJDIR)/cudf.tab.o $(OBJDIR)/cudf.l.o $(OBJDIR)/cudf_hash_table.o $(OBJDIR)/cudf_tools.o
	$(CCC) -shared -W1,-soname,libccudf.so -o libccudf.so \
		$(OBJDIR)/cudf.tab.o $(OBJDIR)/cudf.l.o $(OBJDIR)/cudf_hash_table.o $(OBJDIR)/cudf_tools.o

$(OBJDIR)/cudf_hash_table.o: $(SRCLIB)/cudf_hash_table.c $(SRCLIB)/cudf_hash_table.h
	$(CCC) -c $(INC) -fPIC $(SRCLIB)/cudf_hash_table.c -o $(OBJDIR)/cudf_hash_table.o

$(OBJDIR)/cudf_tools.o: $(SRCLIB)/cudf_tools.c $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -fPIC $(SRCLIB)/cudf_tools.c -o $(OBJDIR)/cudf_tools.o

$(OBJDIR)/cudf.l.o: $(OBJDIR)/cudf.l.c
	$(CCC) -c $(INC) -fPIC $(OBJDIR)/cudf.l.c -o $(OBJDIR)/cudf.l.o

$(OBJDIR)/cudf.tab.o: $(OBJDIR)/cudf.tab.c $(SRCLIB)/cudf.h
	$(CCC) -c $(INC) -fPIC $(OBJDIR)/cudf.tab.c -o $(OBJDIR)/cudf.tab.o

$(OBJDIR)/cudf.l.c: $(SRCLIB)/cudf.l $(OBJDIR)/cudf.tab.h
#	flex -f -8 --prefix=cudf --outfile=$(OBJDIR)/cudf.l.c $(SRCLIB)/cudf.l
	flex -f -8 --prefix=cudf --outfile=$(OBJDIR)/cudf.l.c -i $(SRCLIB)/cudf.l

$(OBJDIR)/cudf.tab.c: $(SRCLIB)/cudf.y
#	bison --verbose --report=state --debug --name-prefix=cudf --file-prefix=$(OBJDIR)/cudf $(SRCLIB)/cudf.y
	bison -v --name-prefix=cudf --file-prefix=$(OBJDIR)/cudf $(SRCLIB)/cudf.y

$(OBJDIR)/cudf.tab.h: $(SRCLIB)/cudf.y
#	bison --verbose --report=state --debug --name-prefix=cudf --defines --file-prefix=$(OBJDIR)/cudf $(SRCLIB)/cudf.y
	bison --name-prefix=cudf --defines --file-prefix=$(OBJDIR)/cudf $(SRCLIB)/cudf.y




clean:
	rm -f *~
	rm -f $(SRCLIB)/*~
	rm -f $(SRCDIR)/*~
	rm -f $(OBJDIR)/*
	rm -f mccs
	rm -f libccudf.a
	rm -f libccudf.so
	rm -f *.lp
	rm -f pblib.opb
	rm -f ctpblib.opb
	rm -f lppbs.out
	rm -f pbsolver.out
	rm -f lpsolver.out
	rm -f installed.txt removed.txt
	rm -f sol
	rm -f cplex.log clone0.log
	rm -f */*~


