#ifndef _TRACEEDITPLOT_H_
#define _TRACEEDITPLOT_H_
#include "SeismicPlot.h"
#include "TraceEditOperator.h"
#include "TimeWindow.h"
#include "stack.h"
#define SINGLE_TRACE_EDIT_MODE 1
#define CUTOFF_EDIT_MODE 2
//namespace SEISPP
//{
using namespace std;
using namespace SEISPP;
/*! Make a plot with trace edit functions enabled.

This object builds a seismic display plot identical to it's parent
called SeismicPlot.  The key difference is that in enables edit functions
in the seisw Widget.   The edit functions are of two form:  (1) single
trace kill and (2) a cutoff method kill.  The first is implemented by 
pointing at a trace and clicking MB2.  The second is implemented with
shift-MB2.  The cutoff editing is similar to that used in dbxcor where
the set of trace below the water mark picked will be killed.  Cutoff makes
sense only if the data are presorted by some metric that defines a level
of badness.  The seisw editor implements kills by setting the live boolean
of the family of time series objects used in my library. Because the 
SeismicPlot object from which this is derives caches a copy of an original
data set the primary method of this object is the report_edit function.
It returns a list of integer index values to the parent ensemble. The caller
must then use this list to implement the kills.  Since what that means is
application dependent it was a design decision to not provide methods
that use this info.  A template is provided below, however, to implement
this in the way I intend with the live variable of time series type objects.

Because this plot handle is a child of SeismicPlot the warnings
described for the SeismicPlot object about copies applies equally 
here.  See documentation for SeismicPlot for a longer explanation.

\author Gary L. Pavlis
*/
/*
This program has been heavily develped by Xiaotao Yang to acheive the following aims:
1. apply auto-editing procedures basically calling the procedures and utilites in 
	TraceEditOperator object;
2. apply thresholds for relavant parameters, e.g., decon parameters, stack weight,
	correltion with reference trace, etc;
3. plot stack trace in a seperate X window instead of adding it as the bottom trace
	of the displayed ensemble.
4. review and restore traces killed by auto-editing procedures. This is an useful 
	feature that show the interactivity of the program;
5. sort traces by different attributes, e.g., stackweight, etc. Sorting is important 
	since it accompanions the cutoff editing mode. It can achieve the same functionality
	as setting thresholds for thos parameters in the auto-editing procedures but shows
	more advantages of interactive editing;
6. log all editing methods used and the corresponding parameters. This is important when
	a series of methods been used with various parameters;
7. undo auto edits and restore original ensemble in case some editings are by mistakes.
8. pick reference trace and use it to sort the traces.

Xiaotao Yang
Jan-Mar 2015
Indiana University
*/
class TraceEditPlot : public SeismicPlot
{
public:
    TraceEditPlot();
    TraceEditPlot(Metadata& md);
    TraceEditOperator teo_global;
    void reset_local_variables();
    set<long> kills;
    set<long> restored_killed_traces_list;
    vector<int> trace_order;  //evid in order after sorting in the ensemble.
    /*! Toggle between single trace and cutoff mode editing. 
     
     The plot produced by this object always is initialized
     in single trace edit mode.  Calling this method changes
     the edit mode to cutoff where clicking on mb2 kills or 
     restores all data below a picked trace.*/
    void backup_original_tse();
    void toggle_edit_mode();
    void turn_on_single_trace_mode();
    void turn_on_cutoff_mode();
	void add_select_trace_callback();
	void edit_enable_public();
	void edit_disable_public();
	void restore_ensemble();
	void undo_auto_edits();
    void help();
    //auto-editing procedures
    void apply_all_autokill();
    void customize_kill_negative_FA_trace();
    void apply_kill_negative_FA_trace();
    void customize_kill_large_amplitude_trace();
    void apply_kill_large_amplitude_trace();
    void customize_kill_small_FA_trace();
    void apply_kill_small_FA_trace();
    void apply_kill_growing_PCoda();
    void customize_kill_growing_PCoda();
    void customize_kill_ClusteredArrivals();
    void apply_kill_ClusteredArrivals();
    void customize_kill_LFC_traces();
    void apply_kill_LFC_traces();
    void apply_kill_decon_by_PARNAME();
    void customize_decon_threshold_by_PARNAME();
    void apply_kill_low_stackweight();
    void customize_kill_low_stackweight();
    void apply_kill_low_dsi();
    void customize_kill_low_dsi();
    void apply_kill_low_ref_correlation();
    void apply_kill_low_ref_correlation_stacktrace();
    void customize_kill_low_ref_correlation();
    void apply_kill_low_RFQualityIndex();
    //sorting procedures.
    void sort_by_magnitude();
    void sort_by_ref_trace();
    void sort_by_xcor_to_stacktrace();
    void customize_time_window_by_WINDOWTYPE();
    void sort_by_stack_weight();
    void sort_by_DECONPARNAME();
    void sort_by_quality_index();
    void customize_weights_for_quality_index();
    void print_trace_order();
    void reverse_trace_order();
    void show_trace_metadata();
    void get_stack_trace();
    void do_beam_plot();
    void review_killed_traces();
    void restore_killed_traces();
    void exit_review_killed_traces();
    void exclude_killed_trace();
    bool killed_trace_excluded;
    ////////Caution: local varibles to store tempoarary data.
    //////// Do not try to call those values even though they are
    //////// in public domain. Those are designed to be used by traceeditplot
    //////// during trace editing. I haven't figured out a way to put them into
    //////// private. Xiaotao Yang
    //reference trace info.
    long ref_trace_number;
    long ref_trace_evid;  //evid from metadata.
    bool ref_trace_picked;
    bool stack_window_set;
    //bool robust_window_set;
	bool allow_edit;
	bool use_decon_in_editing;
	bool use_netmag_table;
	bool killed_review_mode;
	//
	string decon_kill_parname;
	string timewindow_typename;
	string select_trace_typename;
	string decon_sort_parname;
	string show_metadata_typename;
    void report_statistics();
	//Eventually, those functions/members should be put into private domain.
	//call before quiting and after every time changing 
	//the ensemble waveform data (number of traces).
    void report_kills();
    TimeSeriesEnsemble beam_tse;
    StackType stacktype;
	friend class SeismicPlot;
	friend class TraceEditOperator;
	
	
private:
    Metadata metadata; //used to save metadata used in ploting the stack trace.
    DisplayMarkerDataRec tracemarkers;
    string sort_method;
    double RF_quality_index_weights[3];
    set<long> last_autokill_list;
    int size_of_statistics;
    TimeSeries stack_trace;
    
    //container stores killed traces.
    //insert members every time call 'exclude false traces'.
    TimeSeriesEnsemble killed_traces;
	bool tse_backup_set[3];
    //TimeSeriesEnsemble original_ensemble[3];
    void backup_undo_tse();
    bool undo_for_kill; //false if undo is just for viewing options.
    bool undo_for_live;
    TimeSeriesEnsemble *tse_bkp_for_undo0,*tse_bkp_for_undo1,*tse_bkp_for_undo2;
    //those are used to save the traces before entering the review killed trace mode.
    //when existing this mode, the users have the choice to save restored traces. in
    //that case, the restored traces will be pushed back to those backup pointers.
    TimeSeriesEnsemble *tse_bkp_trace_edit0,*tse_bkp_trace_edit1,*tse_bkp_trace_edit2;
    //StackType stacktype;
    void print_prefix();
    void set_defaults();
    void set_defaults(Metadata& md);
    Widget menu_edit;
    Widget menu_view;
    Widget menu_sort;
    //Widget menu_filter;
    Widget menu_help;
    Widget menu_tools;
    /* These two private methods are used to allow common code for
       constructors. */
    void edit_enable();
    void edit_disable();
    void build_edit_menu();
    void build_view_menu();
    void build_sort_menu();
    //void build_filter_menu();
    void build_help_menu();
    void build_tools_menu();
    /* This is used by Seisw widget.  1 for single trace edit mode
       set 2 for cutoff mode 
       NOTE by Xiaotao Yang: other values mean to disable edit.
       */
    int edit_mode;
    bool initialize_by_metadata;
    //default values for trace edit operator object.
		//for kill_small_FA_amplitude procedure.
	//RFeditor 3.0 removed/hided kill small FA method
	double min_normalized_FA_amplitude;
	string amplitude_normalization_method;
			//#could only be either GLOBAL_MAX, GLOBAL_FA_MAX or LOCAL_MAX
	double FA_detect_length;
	double LowFrequency_min, LowFrequency_max;
	//double FA_search_TW_start;
	//double FA_search_TW_end;
	//for killing large amplitude traces.
	double max_trace_abs_amplitude;
	double CodaCA_tolerance_twin_length;
	double PCoda_grow_tolerance;
    TimeWindow FA_search_window, NFA_tolerance_window;
    
    TimeWindow PCoda_search_window;
    TimeWindow CodaCA_search_window;
    TimeWindow robust_twin;
    TimeWindow ref_trace_xcor_twin;
    bool ref_trace_xcor_twin_set;
    //edting by decon parameter threshold
    int nspike_min; // 10
	int nspike_max; // 400
	int niteration_min;// 10
	int niteration_max;// 400
	double rawsnr_min;// 0.1
	double rawsnr_max;// 100
	double averamp_min;// 0.01
	double averamp_max;// 10.0
	double epsilon_min;// 10.0
	double epsilon_max;// 80.0
	double peakamp_min;// 0.01
	double peakamp_max;// 100
	double min_stackweight;
	double min_xcorcoe;
	double min_dsi; //decon success index.
//decon keywords
	string decon_nspike_key;
	string decon_rawsnr_key;
	string decon_averamp_key;
	string decon_epsilon_key;
	string decon_niteration_key;
	string decon_peakamp_key;
};

template <class T> void killmembers(T& ensemble,set<int> kills)
{
    if(kills.empty()) return;
    int nm=ensemble.member.size();
    set<int>::iterator kptr;
    for(kptr=kills.begin();kptr!=kills.end();++kptr)
    {
        int ik=(*kptr);
        if(ik>=nm) 
            throw SeisppError(string("killmembers:  ")
                + "string index is larger than ensemble size\n");
        else
            ensemble.member[ik].live=false;
    }
}
//} // End SEISPP namespace encapsulation
#endif
