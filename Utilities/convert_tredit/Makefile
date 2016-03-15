BIN=convert_tredit
SUBDIR=/contrib

ldlibs=-lseispp -lbrttutil $(TRLIBS) $(DBLIBS) \
		-L$(BOOSTLIB) -L../../libtreditoperator -ltreditoperator

ANTELOPEMAKELOCAL = $(ANTELOPE)/contrib/include/antelopemake.local
include $(ANTELOPEMAKE)  	
include $(ANTELOPEMAKELOCAL)
CXXFLAGS += -I$(BOOSTINCLUDE)

# for valgrind only
CXXFLAGS += -g
OBJS=convert_tredit.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
