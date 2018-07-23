LIB=libtreditoperator.a
SUBDIR=/contrib

####INCLUDE=TraceEditOperator.h

ldlibs=-lseispp -lbrttutil $(TRLIBS) $(DBLIBS) \
       -L$(BOOSTLIB) -lboost_system -lperf

ANTELOPEMAKELOCAL = $(ANTELOPE)/contrib/include/antelopemake.local
include $(ANTELOPEMAKE)  	
include $(ANTELOPEMAKELOCAL)

CXXFLAGS += -I$(BOOSTINCLUDE)

# for valgrind only
CXXFLAGS += -g
OBJS=TraceEditOperator.o
$(LIB) : $(OBJS)
	$(RM) $@
	$(CXXAR) $(CXXARFLAGS) $@ $(OBJS)
	$(RANLIB) $@
