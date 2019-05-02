BIN=RFdb2mat
DATADIR=schemas/css3.0.ext
DATA=wfprocess evlink sclink
SUBDIR=/contrib

MATLABROOT = /Applications/MATLAB_R2014b.app

ldlibs = -lseispp -lbrttutil $(TRLIBS) $(DBLIBS) \
       -L$(BOOSTLIB) -lboost_thread -lboost_system -lboost_filesystem \
       -lperf -L$(MATLABROOT)/bin/maci64 -lmat -lmx 
ANTELOPEMAKELOCAL = $(ANTELOPE)/contrib/include/antelopemake.local
include $(ANTELOPEMAKE)  	
include $(ANTELOPEMAKELOCAL)

CXXFLAGS += -I$(BOOSTINCLUDE) #-static -v
CXXFLAGS += -I$(MATLABROOT)/extern/include 

# for valgrind only
CXXFLAGS += -g
OBJS=RFdb2mat.o dmatrix.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
