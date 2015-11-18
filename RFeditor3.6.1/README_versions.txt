<<<< v2.5 >>>>
Include kill negative FA, and small FA

<<<< v3.0 >>>> 
Removed killing negative FA traces and small FA traces. Added killing low-frequency contaminated traces. The killing negative FA traces was replaced by killing negative first arrivals within the first several seconds (by giving the time window). Fixed bugs in automode when the ensemble is empty.

For Antelope5.5, changes needed in Makefile2.
Add: 
SUBDIR=/contrib
ANTELOPEMAKELOCAL = $(ANTELOPE)/contrib/include/antelopemake.local
CXXFLAGS += -I$(BOOSTINCLUDE)
LDLIBS += -L$(BOOSTLIB)

In TraceEditOperators.cc, add #include “perf.h”.

<<<< v3.5 >>>>
(1) Users may want to view other components instead of radial. There should have a option to let the user custimize the component they want to view. However, the user should be warned that the editing procedures are CURRENTLY based on radial trace characteristics.
Resolved: added edit_on_channel in pf file. User can now set the channel they want to edit on.

(2) Kill low-frequency contaminated traces is not recommended currently.


