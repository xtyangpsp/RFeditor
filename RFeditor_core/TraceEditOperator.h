#ifndef _TRACEEDITOPERATOR_H
#define _TRACEEDITOPERATOR_H
#include <string>
#include <fstream>
#include "TimeSeries.h"
#include "seispp.h"
#include "SeisppKeywords.h"
#include "stack.h"
#include "MultichannelCorrelator.h"
#include "dbpp.h"
#include "Metadata.h"
#include "TimeWindow.h"
using namespace std;
using namespace SEISPP;
#define MYZERO 1e-15
const string evidkey("eventid");   // not evid to avoid collision
const string xcorcoekey("xcorcoe"); //x-correlation coefficience key.
const string ensemblesortkey("ensemble_sortkey");
const string killmethodkey("kill_by_method");
const string killed_trace_key("is_killed_trace");
const string FA_time_key("RF_FirstArrivalTime");
const string FA_amplitude_key("RF_FirstArrivalAmplitude");
const string magnitude_key("magnitude"),magtype_key("magtype");
const string seaz_key("assoc.seaz");
/*
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_histogram.h>
*/

/*
RF Quality Index:
/RF quality index is defined as: 
	QI=a1*StackWeight + a2*Xcorcoe + a3*DeconSuccessIndex (if decon is available).
a1, a2, a3 are empirical values. This index is introduced to get a reasonable overall
ranking of the traces. 
*/
const string RF_quality_index_key("RF_quality_index");
//definition of deconvolution success index (DSI)
/*
DSI=(1-(epsilon/100))*(1.0*nspike/niteration);
DSI has values ranging from 0.0 to 1.0. However, it should not reach 1.0,
which means no energy left in the deconvolved signal (receiver function).
The success rate (nspike/niteration) may reach 1 but that also means there
must be energy conrresponding to the spikes.
	This index indicates the rate of successful iterations combined with 
the energy (L2 norm) that has been deconvolved. However, it doesn't guarantee
the quality of the receiver function. So, other indices need to be used
to further refine the quality of the receiver functions.
*/
const string decon_success_index_key("decon_success_index");
const string sort_method_key("sort_method");//put as ensemble metadata after sorting.
const string NORMALIZE_BY_GLOBAL_MAX("GLOBAL_MAX"); //Global trace amplitude max
const string NORMALIZE_BY_GLOBAL_FA_MAX("GLOBAL_FA_MAX"); //Global FA amplitude max
const string NORMALIZE_BY_LOCAL_MAX("LOCAL_MAX"); //local trace amplitude max
//Auto-editing method codes.
const string TOTAL_NKILLED_AUTO("TotalNumberKilledAuto");
const string AUTOKILL_NegativeFA("NegativeFA");//Negative FA
const string AUTOKILL_LargeAmpTraces("LargeAmpTraces");  //Large Amplitude Traces
const string AUTOKILL_SmallFA("SmallFA");//Small FA
const string AUTOKILL_GrowingPCoda("GrowingPCoda"); //Growing PCoda
const string AUTOKILL_ClusteredArrivals("ClusteredArrivals");//Lack Zero Point
const string AUTOKILL_LowFrequencyContaminated("LFContaminated"); 
const string AUTOKILL_DeconNiteration("DeconNiteration"); 
const string AUTOKILL_DeconNspike("DeconNspike");
const string AUTOKILL_DeconEpsilon("DeconEpsilon");
const string AUTOKILL_DeconPeakamp("DeconPeakamp");
const string AUTOKILL_DeconAveramp("DeconAveramp");
const string AUTOKILL_DeconRawsnr("DeconRawsnr");
const string AUTOKILL_StackWeightCutoff("StackWeightCutoff");
const string AUTOKILL_DSICutoff("DSI_Cutoff");
const string AUTOKILL_RefXcorCutoff("RefXcorCutoff");
const string AUTOKILL_RFQICutoff("QualityIndexCutoff");
const string ManualRestore("ManualRestore");
const string STATISTICS_STRINGDEFAULT("-");
const double STATISTICS_DOUBLEDEFAULT(-999.9);
const int STATISTICS_INTDEFAULT(-9999);
const TimeWindow STATISTICS_TWDEFAULT(TimeWindow(0,1e15));
/*
//RFeditor Editing procedures/methods used in RFeditor.
//
// Author: Xiaotao Yang Jan - Mar 2015
// Indiana University
*/
/* TraceEditStatistics
This is a companion class with TraceEditOperator. It records the killing method and parameters
used by TraceEditOperator objects.
*/
class TraceEditStatistics
{
	public:
		//default constructor. set default values.
		TraceEditStatistics();
		//methods
		void set_defaults();
		void report();
		void report(string fname);
		void print_header(string fname);
		void print_header();
		//parameters
		string station;
		string method;
		TimeWindow FA_twin; //First Arrival (FA) search window.
		TimeWindow PCoda_twin;
		TimeWindow CodaCA_twin;
		TimeWindow XCor_twin;
		TimeWindow Robust_twin;
		TimeWindow Stack_twin;
		string AmpNormalizeMethod;
		string ref_trace_tag;
		double CodaCA_tolerance_twin_length;
		double PCoda_grow_tolerance;
		double FA_range[2]; //min, max. normalized FA
		double LowFrequency_range[2];
		double TraceAmp_range[2];
		double RefXcor_min;
		double StackW_min; //stackweight range
		double DSI_min;
		double QI_weights[3]; //QI: Quality Index.
		double QI_min;
		int DeconNiteration_range[2]; //2 column for min and max.
		int DeconNspike_range[2];
		double DeconEpsilon_range[2];
		double DeconPeakamp_range[2];
		double DeconAveramp_range[2];
		double DeconRawsnr_range[2];
		int nkilled;
		//
		friend class TraceEditOperator;
	//protected:

	private:
		FILE * fh;
		int count_parameters();
		void print_parameters();
		void print_parameters(FILE * fh);
};
/* TraceEditOperator
This class wraps a series of auto trace-editing procedures and other relavant utilities.
It uses a vector of type TraceEditStatistics to save all of the killing/editing procedures
applied for each station. It is recommended that the user reset statistics after finishing 
each station, although it is okay to put all station statistics in this vector. The station
keyword in TraceEditStatistics object is sufficient to let the user post-extract statistical
information for each station.
*/
class TraceEditOperator
{
	public:
		//default constructor. does nothing just construct a object.
		TraceEditOperator();
		//driven by Metadata.
		/* parameters required.
		//decon key definition
		decon_nspike_key    "decon.nspike"
		decon_rawsnr_key "decon.rawsnr"
		decon_averamp_key "decon.averamp"
		decon_epsilon_key "decon.epsilon"
		decon_niteration_key "decon.niteration"
		decon_peakamp_key "decon.peakamp"
		*/
		TraceEditOperator(Metadata& md);
		//destructor.
		~TraceEditOperator(){};
		double compute_decon_success_index(TimeSeries& ts);
		void compute_decon_success_index(TimeSeriesEnsemble& tse);
		//RF quality index is defined as: 
		//QI=a1*StackWeight + a2*Xcorcoe + a3*DeconSuccessIndex (if decon is available).
		double compute_RF_quality_index(TimeSeries& ts, double a1, double a2, double a3);
		bool compute_RF_quality_index(TimeSeriesEnsemble& tse, double a1, 
					double a2, double a3, bool normalize);
		//do sparse convolution. same as in SEISPP but here we process only timeseries data.
		TimeSeries sparse_convolve(TimeSeries& wavelet, TimeSeries& d);
		//ensemble version of the sparse_convolve.
		void convolve_ensemble(TimeSeries& wavelet,TimeSeriesEnsemble& tse, 
					bool trimdata,TimeWindow *twin);
		void convolve_ensemble(TimeSeries& wavelet,ThreeComponentEnsemble& tce, 
					bool trimdata,TimeWindow *twin);
		//reverse order. This is used to reverse the trace order in the TimeSeriesEnsemble object.
		vector<int> reverse_order(TimeSeriesEnsemble& tse);
		// extract and print out metadata for given trace (with evid).
		//if use_decon is true, it also extracts decon parameters from the metadata.
		bool show_metadata(TimeSeriesEnsemble& tse, long evid, bool use_decon);
		//show metadata for all traces with given attribute;
		bool show_metadata(TimeSeriesEnsemble& tse, string mdtag,MDtype mdt);
		// extract and save metadata for given trace (with evid) to FILE handle fh.
		// by default, the metadata version is 1 for this routine.
		////void save_metadata(TimeSeries& ts, FILE * fh, bool use_decon);
		// extract and save metadata for given trace (with evid) to FILE handle fh.
		// mdversion: metadata version, currently supports 1, 2. default is 1
		void save_metadata(TimeSeries& ts, FILE * fh, bool use_decon, int mdversion=1);
		//those two routines find the common time window of all of the members.
		//This is useful when the members don't share the same length.
		TimeWindow find_common_timewindow(TimeSeriesEnsemble& tse);
		TimeWindow find_common_timewindow(ThreeComponentEnsemble& tse);
		//find false traces and returns the evid list of those traces.
		//it can be used after trace editing to extract killed traces (evid list).
		set<long> find_false_traces(TimeSeriesEnsemble& tse);
		bool compute_trace_xcorcoe(TimeSeriesEnsemble& tse, 
					int ref_evid, TimeWindow xcor_twin);
		bool compute_trace_xcorcoe(TimeSeriesEnsemble& tse, 
					TimeSeries& ref_trace, TimeWindow xcor_twin);
		//compute correlation coefficience of the reference trace and the members in the ensemble.
		//this could be modified to read in the trace number of the reference trace instead of
		// the evid, but not neccessary. Then sort by less correlation coefficience.
		vector<int> sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, 
					int ref_evid, TimeWindow xcor_twin);
		//or can be driven directyly by reference trace.
		vector<int> sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, 
					TimeSeries& ref_trace, TimeWindow xcor_twin);
		// this is used to sort the data by correlation with the reference trace (ref).
		// returns the rank/evid order of all of the traces.
		//this is the overload version of the above function. It internally find the common
		// timewindow and use it as the xcor_twin as above. Finding common window is safer than
		// just use an arbitrary window from one of the tse members, since the traces may not have 
		// the same length.
		vector<int> sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, int ref_evid);
		vector<int> sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, 
					TimeSeries& ref_trace);
		//Sort by event magnitude extracted from trace metadata
		vector<int> sort_by_ascend_magnitude(TimeSeriesEnsemble& tse);
		//do robust stacking and sort the ensemble members.
		//it reads in the stacktype. It is able to, potentially, handle different stack type.
		// at the time of it was wrote, only the RobustSNR stacking works.
		vector<int> stack_and_sort(TimeSeriesEnsemble& d,TimeWindow stw, 
					TimeWindow rtw, StackType stacktype);
		//get stack trace simply by calling stack function in SEISPP lib.
		//this is a wrap providing a friendly interface suitable for this class.
		TimeSeries get_stack(TimeSeriesEnsemble& d,TimeWindow stw, 
					TimeWindow rtw, StackType stacktype);
		//sort by decon parameters.
		vector<int> sort_by_decon_parameters(
					TimeSeriesEnsemble& tse,string decon_par);
		//sort by RF quality index;
		vector<int> sort_by_RF_quality_index(TimeSeriesEnsemble& tse,
					double a1, double a2, double a3, bool normalize=true);
		//remove duplicates by evidkey.
		int remove_duplicates(TimeSeriesEnsemble& d, bool verbose=false);
		//this is useful when the user just wants to deal with the good traces.
		//this reduces the data size.
		TimeSeriesEnsemble exclude_false_traces(TimeSeriesEnsemble& d);
		//subset ensemble by extracting traces with match evids.
		TimeSeriesEnsemble extract_traces(TimeSeriesEnsemble& d, set<long> evids_to_extract);
		//this overload version set the extracted trace metadata 'is_killed_trace'.
		TimeSeriesEnsemble extract_traces(TimeSeriesEnsemble& d, 
					set<long> evids_to_extract,bool is_killed_trace);
		//this version force the live flag to be 'live_status' for all extracted traces.
		TimeSeriesEnsemble extract_traces(TimeSeriesEnsemble& d, 
					set<long> evids_to_extract,bool live_status, bool is_killed_trace);
		//extract evids of d2 from d1, and copy metadata of mdlist from d2 to d1.
		TimeSeriesEnsemble extract_traces(TimeSeriesEnsemble& d1,
					TimeSeriesEnsemble& d2, MetadataList& mdlist);
		//extract false traces.
		TimeSeriesEnsemble extract_false_traces(TimeSeriesEnsemble& d,bool is_killed_trace);
		//add members (d2) to the ensemble (d1).
		void merge_ensembles(TimeSeriesEnsemble& d1, TimeSeriesEnsemble& d2);
		//force d2 status to 'trace_status'.
		void merge_ensembles(TimeSeriesEnsemble& d1, TimeSeriesEnsemble& d2, bool trace_status);
		ThreeComponentSeismogram trim_data(ThreeComponentSeismogram& tcs, TimeWindow twin);
		TimeSeries trim_data(TimeSeries& d, TimeWindow twin);
		/* Helpers for edit method.  */ //copied from original RFeditorEngine.cc
		//written by Gary L. Pavlis. Add this to TraceEditOperator is reasonable.
		/* Followup companion helper to apply_kills.  This procedure takes the
		set of event ids returned by apply_edits and uses them to kill any
		members of tse passed to this function with matching evid.  
		In this code this is called to the complement of radial or transverse
		depending on which was being edited. */
		void apply_kills(TimeSeriesEnsemble& tse, set<long> evids_to_kill);
		void apply_kills(ThreeComponentEnsemble& tce, set<long> evids_to_kill);
		TimeSeriesEnsemble reorder_ensemble(TimeSeriesEnsemble& t,vector<int> ordering);
		/*
		// Kill traces with extremely large amplitude phases 
		//(doesn't have to be the first arrival).
		// Amp_max is absolute amplitude value, not normalized.
		*/
		set<long> kill_large_amplitude_traces(TimeSeriesEnsemble& tse, double Amp_max);
		/*
		Kill traces with negative first arrivals. For P-receiver funtions, the FA amplitude 
		should be positive and should also be the peak amplitude of the whole trace.
		From examples in the OIINK dataset, there are a lot of traces with clear negative FAs.
		By checking the raw waveform data (only checked some of them), looks like the raw data
		is bad with noisy records and low S/N ratio.
		*/
		set<long> kill_negative_FA_traces(TimeSeriesEnsemble& tse,double FA_detect_length);
		//Overload version, which only search the FA within the given time window. It uses the
		//same criterion but only scans the trimed data set.
		set<long> kill_negative_FA_traces(TimeSeriesEnsemble& tse,
					TimeWindow FA_search_window,double FA_detect_length);
		set<long> kill_negative_FAs_traces(TimeSeriesEnsemble& tse,
					TimeWindow NFA_tolerance_window);
		//search small FA amplitude within given timewindow. 
		// Amp_min: minimum amplitude normalized by chosen normalization method.
		//
		set<long> kill_small_FA_traces(TimeSeriesEnsemble& tse, 
					double Amp_min,
					TimeWindow FA_search_window,
					string normalization_method,double FA_detect_length);
		//kill traces with growing p-coda phases within given p-coda search window.
		//This routine find the FA amplitude first, then find the maximum amplitude within
		//the P-coda search window (vmax). It kills the traces with vmax>FA_amplitude.
		//Potentially, it should have a overlaod version, which can use the tolerance
		//value set by the user. It kills the traces with vmax-FA_amplitude>tolerance.
		//It works well with this simple version for now.
		set<long> kill_growing_PCoda_traces(TimeSeriesEnsemble& tse,
					TimeWindow FA_search_window,TimeWindow PCoda_search_window,
					double PCoda_grow_tolerance,double FA_detect_length);
		//Overload version: kill traces with vmax > than max of the data prior to the
		//p-coda window.
		set<long> kill_growing_PCoda_traces(TimeSeriesEnsemble& tse,
					TimeWindow PCoda_search_window,double PCoda_grow_tolerance);
		
		//kill traces lacking zero points within the tolerance length of a timewindow.
		//It looks for the point where two adjacent samples have opposite signs (+ to - or - to +)
		//within the moving window with length of 'tlength'. If there is at least one
		// zero point, it kills the trace. To improve the efficiency, it stops the scanning
		// once it find the fist zero point and go to the next trace. It only searchs within the 
		//given CodaCA_search_window. 
		set<long> kill_ClusteredArrivals_traces(TimeSeriesEnsemble& tse,
					TimeWindow CodaCA_search_window,double tlength);
		//kill low frequency Contaminated traces.
		//looking for the max from the spectrum. kill traces that the frequency
		//of the max amplitude falls into the low-frequency window (lf_min, lf_max).
		//the lower bound is needed since some traces with ultra low frequency background
		//may not be bad traces. e.g., 100s singnal may have no effect on the receiver functions.
		set<long> kill_LowFrequencyContaminated_traces(TimeSeriesEnsemble& tse,
					double lf_min,double lf_max);
		
		//simply kill traces with decon parameters fall outside of the given 
		//threshold (lower and upper limits).
		set<long> kill_by_decon_ALL(TimeSeriesEnsemble& tse,
					int niteration_min,int niteration_max,
					int nspike_min,int nspike_max,
					double epsilon_min,double epsilon_max,
					double peakamp_min,double peakamp_max,
					double averamp_min,double averamp_max,
					double rawsnr_min,double rawsnr_max);
		set<long> kill_by_decon_niteration(TimeSeriesEnsemble& tse,
					int niteration_min,int niteration_max);
		set<long> kill_by_decon_nspike(TimeSeriesEnsemble& tse,
					int nspike_min,int nspike_max);
		set<long> kill_by_decon_epsilon(TimeSeriesEnsemble& tse,
					double epsilon_min,double epsilon_max);
		set<long> kill_by_decon_peakamp(TimeSeriesEnsemble& tse,
					double peakamp_min,double peakamp_max);
		set<long> kill_by_decon_averamp(TimeSeriesEnsemble& tse,
					double averamp_min,double averamp_max);
		set<long> kill_by_decon_rawsnr(TimeSeriesEnsemble& tse,
					double rawsnr_min,double rawsnr_max);
		//kill by setting minimum stackweight.
		set<long> kill_low_stackweight_traces(TimeSeriesEnsemble& tse, 
					double sw_min);
		//kill by setting minimum DSI.
		set<long> kill_low_dsi_traces(TimeSeriesEnsemble& tse, 
					double dsi_min);
		//kill by setting minimum correlation coefficience with reference trace.
		//preassume xcorcoe is computed prior to this call.
		set<long> kill_low_ref_correlation_traces(
					TimeSeriesEnsemble& tse,double xcorcoe_min);
		//internally call sort_by_less_xcorcoef();
		set<long> kill_low_ref_correlation_traces(
					TimeSeriesEnsemble& tse,int ref_evid, 
					TimeWindow xcor_twin,double xcorcoe_min);
		set<long> kill_low_ref_correlation_traces(
					TimeSeriesEnsemble& tse,TimeSeries ref_trace, 
					TimeWindow xcor_twin,double xcorcoe_min);
		set<long> kill_low_RFQualityIndex_traces(TimeSeriesEnsemble& tse,
					double a1, double a2, double a3, 
					double qi_min,bool normalize=true);
		//set all traces back to live.
		//Currently, for some unknown reason, this causes segment faults in TraceEditPlot.
		// haven't tried it in other places.
		//Changed its functionality to set live_status for the whole traces. However, although
		//it does change the live status, it cannot be plotting in seisw plot if I change the live
		//status to true from false. the other way (from true to false might work but i hven't tried).
		void set_ensemble_live_status(TimeSeriesEnsemble& tse,bool live_status);
		
		//undo kills to traces with given evid list to restore.
		void undo_kills_to_trace(TimeSeriesEnsemble& tse, set<long> evids_to_kill);
		//report statistics about TraceEditOperator to std or to file.
		//following is a series of methods operating on the 'statistics' vector.
		void report_statistics(); //report detailed information for each method.
		void report_statistics(string fname); //report to file.
		Metadata get_statistics_summary(); //
		void reset_statistics(){statistics.clear();};
		void remove_last_statistics(){statistics.pop_back();};
		void add_to_statistics(TraceEditStatistics tes){statistics.push_back(tes);};
		int sizeof_statistics(){return(statistics.size());};
		friend class TraceEditPlot;
	protected:
		//Don't want to put this as a public member since caller should
		//not work directly on each member of this vector.
		//Methods above can be used to accomplish basic functions.
		vector<TraceEditStatistics> statistics;
		
	private:
		void set_operator_defaults();
		double FA_sensitivity;
		string data_shaping_wavelet_type;
		string decon_nspike_key;
		string decon_rawsnr_key;
		string decon_averamp_key;
		string decon_epsilon_key;
		string decon_niteration_key;
		string decon_peakamp_key;
};
//look for first arrival amplitude
/*
Algorithm: this routine scans from the begining of the trace,
looks for points where the sampe 1st derivative changes sign (+ to - or - to +). It writes
the amplitude of this point back to FA_amp and returns the time value 
of this point as the First Arrial time.
FA time and amplitude are written into the Metadata of the timeseries data.
*/
double find_FirstArrival(TimeSeries& ts, double *FA_amp);
/*
this overload version uses a different algorithm to detect the FA.
Algorithm:
(1)For data_shaping_wavelet_type == RICKER, search for the max.
It searches for the max abs point within the detect_length window
after the first non-zero value (>= FA_sensitivity).
(2) For data_shaping_wavelet_type == GAUSSIAN, searches for the turning
point
(3) For data_shaping_wavelet_type ==SPIKE, searches for the first non-zero point.
it writes the FA time and amplitude into the metadata of the TimeSeries data.
 it returns the FA type: BEGIN (of the search window), PEAK, TROUGH, END (of
 the search window). For SPIKE wavelet_type, it returns the FA_type as SPIKE.
*/
string find_FirstArrival(TimeSeries& ts, double FA_sensitivity, 
			double detect_length, string data_shaping_wavelet_type);
//look for first arrival amplitude within given timewindow.
//double find_FirstArrival(TimeSeries& ts,float twstart, float twend);

//find global max absolute FA values from all traces.
//
double find_max_abs_FirstArrival(TimeSeriesEnsemble& tse);
/*
Algorithm:
1. get Robust stack
2. detect first peak from stack trace
3. set FA search window based on FA time of the stack trace based 
	on given delta value (-dealta to +delta relative to stack FA time)
4. detect first peak within the above FA search window
*/
vector<double> find_FirstArrival(TimeSeriesEnsemble& tse,double FA_search_delta);


//find global max absolute amplitude values from all traces.
//
double find_max_abs_amplitude(TimeSeriesEnsemble& tse);

//simply compute dot products of the of two TimeSeries traces normalized
// by the product of the L2 norm of the two vectors.
//it assumes that the data have same length. this is a zerolag version.
// originally from: http://paulbourke.net/miscellaneous/correlate/
double compute_xcor_zerolag(vector<double> x, vector<double> y);
/*
compute fft forward transform using gsl function: gsl_fft_real_transform()
*/
vector<double> compute_fft_real_transform(vector<double> data);
//this function was in RFeditorEngine.h.
/* This is the sort order function used for stack sorting */
template <class T> struct less_stackwt
                : public binary_function<T,T,bool> 
{
        bool operator()(T x, T y)
        {
                double valx,valy;
                try {
                        valx=x.get_double(SEISPP::stack_weight_keyword);
                }catch(...) {return true;};
                try {
                        valy=y.get_double(SEISPP::stack_weight_keyword);
                }catch(...) {return false;};

                if(valx<valy)
                        return true;
                else
                        return false;
        }
};
// this is used to sort the data by correlation with the reference trace (ref).
// returns the rank/order of all of the traces.
/* This is the sort order function used for sorting by less x-correlation coefficience*/
template <class T> struct less_xcorcoe
                : public binary_function<T,T,bool> 
{
        bool operator()(T x, T y)
        {
                double valx,valy;
                try {
                        valx=x.get_double(xcorcoekey);
                }catch(...) {return true;};
                try {
                        valy=y.get_double(xcorcoekey);
                }catch(...) {return false;};

                if(valx<valy)
                        return true;
                else
                        return false;
        }
};

//a series of template used in sorting by decon parameters.
template <class T> struct sort_less_double
                : public binary_function<T,T, bool> 
{
        bool operator()(T x, T y)
        {
                double valx,valy;
                string key;
                try{
                	key=x.get_string(ensemblesortkey);
                }catch(...) 
                {
                	cerr<<"Error getting sortkey"<<endl;
                	return false;
                };
                try {
                        valx=x.get_double(key);
                }catch(...) {return true;};
                try {
                        valy=y.get_double(key);
                }catch(...) {return false;};

                if(valx<valy)
                        return true;
                else
                        return false;
        }
};

template <class T> struct sort_less_int
                : public binary_function<T,T, bool> 
{
        bool operator()(T x, T y)
        {
                int valx,valy;
                string key;
                try{
                	key=x.get_string(ensemblesortkey);
                }catch(...) 
                {
                	cerr<<"Error getting sortkey"<<endl;
                	return false;
                };
                try {
                        valx=x.get_int(key);
                }catch(...) {return true;};
                try {
                        valy=y.get_int(key);
                }catch(...) {return false;};

                if(valx<valy)
                        return true;
                else
                        return false;
        }
};

#endif