#include "SeisppKeywords.h"
#include "TimeSeries.h"
#include "ensemble.h"
#include "Metadata.h"
#include "TraceEditPlot.h"
#include "TraceEditOperator.h"
using namespace std;
using namespace SEISPP;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//start of RFeditorEngine
//changed to load Radial data and R window only.
/*
This class originally includes two TraceEditPlot members (T window and R window).
For P receiver function editing, only R window (radial)data is useful.
That is why I changed it to only include one trace (intend to be radial ensemble).
RFeditorEngine class is an interface between RFeditor main program 
and trace editing procedures, either graphically or automatically.
Xiaotao Yang
Jan-Mar 2015
Indiana University
*/
class RFeditorEngine
{
    public:
        //default constructor. does nothing but build the object.
        //WARNING: DO NOT USE THE OBJECT BIUILT BY THIS DEFAULT CONSTRUCTOR.
        //user must assign it to other constructors.
        /*
        //example:
        // RFeditorEngine *rfe;
        // under some conditions, overload it to object using other constructors.
        // e.g., rfe=new RFeditorEngine(metadata);
        */
        RFeditorEngine(){};
        // this constructor is the actual usable constructor.
        RFeditorEngine(Metadata& params, bool GUIoff=false);
        //default destructor. may not be necessary.
        //~RFeditorEngine(){delete RFwindow;delete teo;};
        /* \brief Loads ensembles of RFs and enable interactive editing.  

           Load a pair of ensembles for radial and transverse for 
           editing.   Edit procedure first makes the radial window 
           active.  Pick bad traces and exit.  Find matching transverse
           data and kills them before plotting the transverse data
           and making that window live for edits.

           \param r ensemble of radial receiver functions
           \param t ensemble of transverse receiver functions
           
           \return Number of traces loaded.
           */
        //int edit(TimeSeriesEnsemble& r, TimeSeriesEnsemble& d,
        //			set<long> *rkills,set<long> *tkills){return 0;}; 
        set<long> edit(TimeSeriesEnsemble& r);
        set<long> edit(TimeSeriesEnsemble& r,Metadata& auto_edit_params);
        //save statistical information to file: fname.
        void save_statistics(string fname);
        void save_statistics_summary(string fname, string rfe_version="-");
        void save_statistics_summary(DatascopeHandle& dbh, int treditversion=1, string rfe_version="-");
        void reset_statistics()
        {if(!GUIoff) RFwindow->teo_global.reset_statistics();
        	else this->teo->reset_statistics();};
        TraceEditOperator *teo;
    private:
        bool cutoff_editing;
        bool GUIoff;
        string station;
        int ntraces;
        int nkilled;
        int nkilled_auto;
        int nkilled_manual;
        //TraceEditPlot Twindow;
        TraceEditPlot *RFwindow;  // show R window later is better. Xiaotao Yang 1/22/2015
        //TimeWindow robust_twin;
        /* private method kills T if R marked or R if T marked bad */
        //void kill_sibling(); 
};