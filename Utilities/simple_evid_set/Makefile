BIN=simple_evid_set
cflags=-g
ldflags=$(DBLIBS)

include $(ANTELOPEMAKE)
OBJS=	simple_evid_set.o
$(BIN) : $(OBJS)
	$(RM) $@
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)
