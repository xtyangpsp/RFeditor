LIB=libtreditoperator.a
SUBDIR=/contrib

INCLUDE=TraceEditOperator.h

ldlibs=-lseispp -lbrttutil $(TRLIBS) $(DBLIBS) \
       -L$(BOOSTLIB) -lboost_system -L/Users/xtyang/usr/local/lib -lgsl \
       -lperf -lgclgrid

ANTELOPEMAKELOCAL = $(ANTELOPE)/contrib/include/antelopemake.local
include $(ANTELOPEMAKE)  	
include $(ANTELOPEMAKELOCAL)

CXXFLAGS += -I$(BOOSTINCLUDE)
CXXFLAGS += -I/Users/xtyang/usr/local/include

#LDLIBS += -L$(BOOSTLIB) -L/Users/xtyang/usr/local/lib
# for valgrind only
CXXFLAGS += -g
OBJS=TraceEditOperator.o
$(LIB) : $(OBJS)
	$(RM) $@
	$(CXXAR) $(CXXARFLAGS) $@ $(OBJS)
	$(RANLIB) $@
