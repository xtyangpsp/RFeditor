#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include "seispp.h"
#include "SeisppKeywords.h"
#include "dbpp.h"
#include "RFeditorEngine.h"
using namespace std;
using namespace SEISPP;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
RFeditorEngine::RFeditorEngine(Metadata& params, bool guioff_mode) 
//, Twindow(params)
{
    GUIoff=guioff_mode;
    if(!GUIoff)
	{
		try 
		{
			RFwindow=new TraceEditPlot(params);
		}catch(...)
		{
			cerr<<"RFeditorEngine: error in main constructor with graphics."<<endl;
			throw;
		};
	}
	else
	{
		//cerr<<"This is a test for guioff."<<endl;
		teo=new TraceEditOperator(params);
		RFwindow=NULL;
	}
}
void RFeditorEngine::save_statistics(string fname)
{
	TraceEditStatistics tes;
	tes.print_header(fname);
	if(!GUIoff)
		RFwindow->teo_global.report_statistics(fname);
	else
		teo->report_statistics(fname);
}
void RFeditorEngine::save_statistics_summary(string fname, string rfe_version)
{
	FILE * fh=fopen(fname.c_str(),"a");
	Metadata summary;
	if(!this->GUIoff)
	{	
		//DEBUG
		summary=RFwindow->teo_global.get_statistics_summary();
	}
	else
	{	summary=this->teo->get_statistics_summary();
	}
	nkilled_auto=summary.get_int(TOTAL_NKILLED_AUTO);
	nkilled_manual=nkilled - nkilled_auto;
	fprintf(fh,"$--- Summary for station [ %s ] -< %s >--$\n",station.c_str(),rfe_version.c_str());
	fprintf(fh,"Input-Traces                %10d\n",ntraces);
	fprintf(fh,"Auto-Killed                %10d\n",nkilled_auto);
	fprintf(fh,"Manual-Killed              %10d\n",nkilled_manual);
	fprintf(fh,"After-Editing              %10d\n",ntraces - nkilled);
	fprintf(fh,"Acceptance-Rate (%%)        %10.1f\n",100.0*(ntraces - nkilled)/ntraces);
	fprintf(fh,"NegativeFA                 %10d\n",summary.get_int(AUTOKILL_NegativeFA));//Negative FA
	fprintf(fh,"LargeAmpTraces             %10d\n",summary.get_int(AUTOKILL_LargeAmpTraces));  //Large Amplitude Traces
	fprintf(fh,"SmallFA                    %10d\n",summary.get_int(AUTOKILL_SmallFA));//Small FA
	fprintf(fh,"GrowingPCoda               %10d\n",summary.get_int(AUTOKILL_GrowingPCoda)); //Growing PCoda
	fprintf(fh,"ClusteredArrivals          %10d\n",summary.get_int(AUTOKILL_ClusteredArrivals));//Lack Zero Point
	fprintf(fh,"LowFrequencyContaminated   %10d\n",summary.get_int(AUTOKILL_LowFrequencyContaminated));
	fprintf(fh,"StackWeightCutoff          %10d\n",summary.get_int(AUTOKILL_StackWeightCutoff));
	fprintf(fh,"RefXcorCutoff              %10d\n",summary.get_int(AUTOKILL_RefXcorCutoff));
	fprintf(fh,"DSI_Cutoff                 %10d\n",summary.get_int(AUTOKILL_DSICutoff));
	fprintf(fh,"RFQI-Cutoff                %10d\n",summary.get_int(AUTOKILL_RFQICutoff));
	fprintf(fh,"DeconNiteration            %10d\n",summary.get_int(AUTOKILL_DeconNiteration)); 
	fprintf(fh,"DeconNspike                %10d\n",summary.get_int(AUTOKILL_DeconNspike));
	fprintf(fh,"DeconEpsilon               %10d\n",summary.get_int(AUTOKILL_DeconEpsilon));
	fprintf(fh,"DeconPeakamp               %10d\n",summary.get_int(AUTOKILL_DeconPeakamp));
	fprintf(fh,"DeconAveramp               %10d\n",summary.get_int(AUTOKILL_DeconAveramp));
	fprintf(fh,"DeconRawsnr                %10d\n",summary.get_int(AUTOKILL_DeconRawsnr));
	fprintf(fh,"ManualRestore              %10d\n",summary.get_int(ManualRestore));
	fprintf(fh,"\n");
	//delete summary;
	fclose(fh);
}
void RFeditorEngine::save_statistics_summary(DatascopeHandle& dbh, 
		int treditversion, string rfe_version)
{
	DatascopeHandle dbtredit(dbh);
	dbtredit.lookup("tredit");
	Metadata summary;
	if(!GUIoff)
		summary=RFwindow->teo_global.get_statistics_summary();
	else
		summary=teo->get_statistics_summary();
	nkilled_auto=summary.get_int(TOTAL_NKILLED_AUTO);
	nkilled_manual=nkilled - nkilled_auto;
	double acceptance_rate=100.0*(ntraces - nkilled)/ntraces;
	
	switch(treditversion)
	{
		case 1:
			/* attributes in tredit table.
			sta ntracein nkilled acptrate aknfa aklat aksfa akgpc 
			akca klsw klxcor kdnitn kdnspike kdepsilon kdpkamp kdavamp kdsnr kmanual nrestored
			*/
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("nkilled",nkilled);
			dbtredit.put("acptrate",acceptance_rate);
			dbtredit.put("aknfa",summary.get_int(AUTOKILL_NegativeFA));
			dbtredit.put("aklat",summary.get_int(AUTOKILL_LargeAmpTraces));
			dbtredit.put("aksfa",summary.get_int(AUTOKILL_SmallFA));
			dbtredit.put("akgpc",summary.get_int(AUTOKILL_GrowingPCoda));
			dbtredit.put("akca",summary.get_int(AUTOKILL_ClusteredArrivals));
			dbtredit.put("aklfc",summary.get_int(AUTOKILL_LowFrequencyContaminated));
			dbtredit.put("klsw",summary.get_int(AUTOKILL_StackWeightCutoff));
			dbtredit.put("klxcor",summary.get_int(AUTOKILL_RefXcorCutoff));
			dbtredit.put("kldsi",summary.get_int(AUTOKILL_DSICutoff));
			dbtredit.put("klrfqi",summary.get_int(AUTOKILL_RFQICutoff));
			dbtredit.put("kdnitn",summary.get_int(AUTOKILL_DeconNiteration));
			dbtredit.put("kdnspike",summary.get_int(AUTOKILL_DeconNspike));
			dbtredit.put("kdepsilon",summary.get_int(AUTOKILL_DeconEpsilon));
			dbtredit.put("kdpkamp",summary.get_int(AUTOKILL_DeconPeakamp));
			dbtredit.put("kdavamp",summary.get_int(AUTOKILL_DeconAveramp));
			dbtredit.put("kdsnr",summary.get_int(AUTOKILL_DeconRawsnr));
			dbtredit.put("kmanual",nkilled_manual);
			dbtredit.put("nrestored",summary.get_int(ManualRestore));
			break;
		case 2:
			/* attributes in tredit table.
			sta ntracein ntraceout acceptancerate nkilled procedure
			//procedure: name of the editing method.
			*/
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_LargeAmpTraces));
			dbtredit.put("procedure",AUTOKILL_LargeAmpTraces);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconNiteration));
			dbtredit.put("procedure",AUTOKILL_DeconNiteration);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconNspike));
			dbtredit.put("procedure",AUTOKILL_DeconNspike);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconEpsilon));
			dbtredit.put("procedure",AUTOKILL_DeconEpsilon);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconPeakamp));
			dbtredit.put("procedure",AUTOKILL_DeconPeakamp);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconAveramp));
			dbtredit.put("procedure",AUTOKILL_DeconAveramp);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DeconRawsnr));
			dbtredit.put("procedure",AUTOKILL_DeconRawsnr);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_DSICutoff));
			dbtredit.put("procedure",AUTOKILL_DSICutoff);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_NegativeFA));
			dbtredit.put("procedure",AUTOKILL_NegativeFA);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_GrowingPCoda));
			dbtredit.put("procedure",AUTOKILL_GrowingPCoda);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_ClusteredArrivals));
			dbtredit.put("procedure",AUTOKILL_ClusteredArrivals);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_SmallFA));
			dbtredit.put("procedure",AUTOKILL_SmallFA);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_LowFrequencyContaminated));
			dbtredit.put("procedure",AUTOKILL_LowFrequencyContaminated);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_StackWeightCutoff));
			dbtredit.put("procedure",AUTOKILL_StackWeightCutoff);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_RefXcorCutoff));
			dbtredit.put("procedure",AUTOKILL_RefXcorCutoff);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(AUTOKILL_RFQICutoff));
			dbtredit.put("procedure",AUTOKILL_RFQICutoff);
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",nkilled_manual);
			dbtredit.put("procedure","ManualEdit");
			dbtredit.put("version",rfe_version);
			
			dbtredit.append();
			dbtredit.put("sta",station);
			dbtredit.put("ntracein",ntraces);
			dbtredit.put("ntraceout",ntraces - nkilled);
			dbtredit.put("acceptancerate",acceptance_rate);
			dbtredit.put("nkilled",summary.get_int(ManualRestore));
			dbtredit.put("procedure",ManualRestore);
			dbtredit.put("version",rfe_version);

			break;
		default:
			cerr<<"ERROR in save_statistics_summary(): unknown treditversion, should only be 1 or 2."<<endl;
			exit(-1);
	}
	//delete summary;
}
//int RFeditorEngine::edit(TimeSeriesEnsemble& rd, TimeSeriesEnsemble& td)
set<long> RFeditorEngine::edit(TimeSeriesEnsemble& rd)
//string stacktype for stack type. Xiaotao Yang 01/08/2015
{
    try{
        if(!GUIoff)
		{
			//DEBUG
			//int ntraces;
			set<long> rkills0;
			ntraces=rd.member.size();
			cout << "Loaded ensemble size="<<ntraces<<endl;
			/* This changes the plot titles for each window.  Assumes sta
			   is defined in ensemble metadata */
			station=rd.get_string("sta");
			//string title=string("Transverse RF data for station ")+sta;
			//Twindow.put("title",title);
			string title=string("< ")+rd.get_string("chan")+
                    string(" >")+string(" channel data for station ")+station;
			RFwindow->put("title",title);
		
			RFwindow->plot(rd,true);
			
			rkills0=RFwindow->kills;
			RFwindow->kills.clear();
			nkilled=rkills0.size();

        	return(rkills0);
        }
        else
        {
        	cerr<<"ERROR in RFeditorEngine::edit(). This is for GUI mode only!"<<endl;
        	exit(-1);
        }
    }catch(...){throw;};
}
set<long> RFeditorEngine::edit(TimeSeriesEnsemble& tse,Metadata& md)
{
    try{
        if(GUIoff)
		{
			//DEBUG
			//int ntraces;
			set<long> rkills0, killtmp;
			ntraces=tse.member.size();
			cout << "GUIoff mode: Loaded ensemble size="<<ntraces<<endl;
			//    <<"transverse ensemble size="<<td.member.size()<<endl;
			station=tse.get_string("sta");
			bool use_decon_in_editing=md.get_bool("use_decon_in_editing");
			//load auto_edit_parameters
			//see tredit table description for definitions of those abbreviations.
			bool apply_knfa, apply_klat, apply_kgpc, apply_kca, apply_klfc;
			bool apply_klsw, apply_klxcor, apply_kldsi,apply_klrfqi,apply_kdnitn, apply_kdnspike;
			bool apply_kdepsilon,apply_kdpkamp,apply_kdavamp, apply_kdsnr, apply_decon_ALL;
			try
			{
				apply_knfa=md.get_bool("apply_knfa");
				apply_klat=md.get_bool("apply_klat");
				//apply_ksfa=md.get_bool("apply_ksfa");
				apply_kgpc=md.get_bool("apply_kgpc");
				apply_kca=md.get_bool("apply_kca");
				apply_klfc=md.get_bool("apply_klfc");
				apply_klsw=md.get_bool("apply_klsw");
				apply_klxcor=md.get_bool("apply_klxcor");
				apply_kldsi=md.get_bool("apply_kldsi");
				apply_kdnitn=md.get_bool("apply_kdnitn");
				apply_kdnspike=md.get_bool("apply_kdnspike");
				apply_kdepsilon=md.get_bool("apply_kdepsilon");
				apply_kdpkamp=md.get_bool("apply_kdpkamp");
				apply_kdavamp=md.get_bool("apply_kdavamp");
				apply_kdsnr=md.get_bool("apply_kdsnr");
				apply_decon_ALL=md.get_bool("apply_decon_ALL");
				apply_klrfqi=md.get_bool("apply_klrfqi");
			}catch(SeisppError& serr)
			{
				cerr<<"Error in getting choices for editing procedures: "<<endl
					<<serr.what()<<endl;
				exit(-1);
			}
			
			if(apply_klfc)
				cerr << "** WARNING: procedure of killing Low-Frequency Contaminated traces" <<endl
					 << "   are NOT recommended in this version! It is under development." <<endl;
			
			if((apply_kdnitn || apply_kdnspike || apply_kdepsilon || apply_kdpkamp ||
				apply_kdavamp || apply_kdsnr || apply_kldsi) && !use_decon_in_editing)
			{
				cerr << "*** ERROR in RFeditorEngine (GUIoff mode): can't apply decon" <<endl
					 << "parameter related procedures when use_decon_in_editing is set "<<endl
					 << "to false!"<<endl;
				exit(-1);
			}
						
			/*
			double min_normalized_FA_amplitude;
			string amplitude_normalization_method;
					//#could only be either GLOBAL_MAX, GLOBAL_FA_MAX or LOCAL_MAX
			//for killing large amplitude traces.
			TimeWindow FA_search_window;
			*/
			double max_trace_abs_amplitude;
			TimeWindow NFA_tolerance_window;
			TimeWindow PCoda_search_window;
			TimeWindow CodaCA_search_window;
			TimeWindow robust_twin;
			TimeWindow ref_trace_xcor_twin;
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
			double min_dsi;
			double CodaCA_tolerance_twin_length;
			double PCoda_grow_tolerance;
			double RFQI_weights[3];
			double min_rfqi;
			double LowFrequency_min, LowFrequency_max;
			string st_tmp;
			StackType stacktype;
			double FA_reference_time=md.get_double("FA_reference_time");
			//double FA_detect_length=md.get_double("FA_detect_length");

            //for killing large amplitude traces.
            if(apply_klat)
            {
                if(tse.member.size()<=0)
                {
                    cerr<<"** Ensemble is empty. Skipped this kill procedure: klat."<<endl;
                }
                else
                {
                    max_trace_abs_amplitude=md.get_double("max_trace_abs_amplitude");
                    killtmp=teo->kill_large_amplitude_traces(tse,max_trace_abs_amplitude);
                    cerr<<"Number of traces killed by applying klat = "<<killtmp.size()<<endl;
                    if(killtmp.size()>0)
                        rkills0.insert(killtmp.begin(),killtmp.end());
                    killtmp.clear();
                }
            }
/*
***********************************************************************************
********************** SECTION-1: Decon thresholds *************************************
***********************************************************************************
*/
			if(use_decon_in_editing)
			{
				//default decon thresholds.
				if(apply_decon_ALL)
				{
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: decon_ALL."<<endl;
					}
					else
					{
						niteration_min=md.get_int("niteration_min"); //10
						niteration_max=md.get_int("niteration_max"); //400
						nspike_min=md.get_int("nspike_min"); //10
						nspike_max=md.get_int("nspike_max"); //400
						epsilon_min=md.get_double("epsilon_min"); //10.0
						epsilon_max=md.get_double("epsilon_max"); //80.0
						peakamp_min=md.get_double("peakamp_min"); //0.01
						peakamp_max=md.get_double("peakamp_max"); //100
						averamp_min=md.get_double("averamp_min"); //0.01
						averamp_max=md.get_double("averamp_max"); //10.0
						rawsnr_min=md.get_double("rawsnr_min"); //0.1
						rawsnr_max=md.get_double("rawsnr_max"); //100
				
						killtmp=teo->kill_by_decon_ALL(tse,
							niteration_min,niteration_max,
							nspike_min,nspike_max,
							epsilon_min,epsilon_max,
							peakamp_min,peakamp_max,
							averamp_min,averamp_max,
							rawsnr_min,rawsnr_max);
						cerr<<"Number of traces killed by applying decon_ALL = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				}
				else if(apply_kdnitn)
				{
					niteration_min=md.get_int("niteration_min"); //10
					niteration_max=md.get_int("niteration_max");
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdnitn."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_niteration(tse,
												niteration_min,niteration_max);
						cerr<<"Number of traces killed by applying decon_niteration = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				} //400
				else if(apply_kdnspike)
				{
					nspike_min=md.get_int("nspike_min"); //10
					nspike_max=md.get_int("nspike_max"); 
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdnspike."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_nspike(tse,
												nspike_min,nspike_max);
						cerr<<"Number of traces killed by applying decon_nspike = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				}//400
				else if(apply_kdepsilon)
				{
					epsilon_min=md.get_double("epsilon_min"); //10.0
					epsilon_max=md.get_double("epsilon_max");
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdepsilon."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_epsilon(tse,
												epsilon_min,epsilon_max);
						cerr<<"Number of traces killed by applying decon_epsilon = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				} //80.0
				else if(apply_kdpkamp)
				{
					peakamp_min=md.get_double("peakamp_min"); //0.01
					peakamp_max=md.get_double("peakamp_max");
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdpkamp."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_peakamp(tse,
												peakamp_min,peakamp_max);
						cerr<<"Number of traces killed by applying decon_peakamp = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				} //100
				else if(apply_kdavamp)
				{
					averamp_min=md.get_double("averamp_min"); //0.01
					averamp_max=md.get_double("averamp_max");
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdavamp."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_averamp(tse,
												averamp_min,averamp_max);
						cerr<<"Number of traces killed by applying decon_averamp = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				} //10.0
				else if(apply_kdsnr)
				{
					rawsnr_min=md.get_double("rawsnr_min"); //0.1
					rawsnr_max=md.get_double("rawsnr_max");
					if(tse.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kdsnr."<<endl;
					}
					else
					{
						killtmp=teo->kill_by_decon_rawsnr(tse,
												rawsnr_min,rawsnr_max);
						cerr<<"Number of traces killed by applying decon_rawsnr = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
							rkills0.insert(killtmp.begin(),killtmp.end());
						killtmp.clear();
					}
				} //100
				
				// Kill by DSI threshold
				if(apply_kldsi)
				{
					if(!use_decon_in_editing)
					{
						cerr<<"** Fatal ERROR: can't apply kldsi, use_decon_in_editing is set to false."<<endl;
						exit(-1);
					}
					min_dsi=md.get_double("dsi_min");
				
					TimeSeriesEnsemble tse_tmp=teo->exclude_false_traces(tse);
					if(tse_tmp.member.size()<=0)
					{
						cerr<<"** Ensemble is empty. Skipped this kill procedure: kldsi."<<endl;
						tse_tmp.member.clear();
					}
					else
					{
						killtmp=teo->kill_low_dsi_traces(tse_tmp,min_dsi);
						cerr<<"Number of traces killed by applying ldsi = "<<killtmp.size()<<endl;
						if(killtmp.size()>0)
						{
							rkills0.insert(killtmp.begin(),killtmp.end());
							teo->apply_kills(tse,killtmp);
						}
						tse_tmp.member.clear();
						killtmp.clear();
					}
				}
			}

/*
***********************************************************************************
********************** SECTION-2: Trace-Based QC *************************************
***********************************************************************************
*/
			if(apply_knfa)
			{
				if(tse.member.size()<=0)
				{
					cerr<<"** Ensemble is empty. Skipped this kill procedure: knfa."<<endl;
				}
				else
				{
					NFA_tolerance_window=TimeWindow(md.get_double("NFA_tolerance_TW_start")+FA_reference_time,
									md.get_double("NFA_tolerance_TW_end")+FA_reference_time);
					killtmp=teo->kill_negative_FAs_traces(tse,NFA_tolerance_window);
					cerr<<"Number of traces killed by applying knfa = "<<killtmp.size()<<endl;
					if(killtmp.size()>0)
						rkills0.insert(killtmp.begin(),killtmp.end());
					killtmp.clear();
				}
			}
			
			/*
			if(apply_ksfa)
			{
				//for kill_small_FA_amplitude procedure.
				min_normalized_FA_amplitude=
					md.get_double("min_normalized_FA_amplitude");
				amplitude_normalization_method=
					md.get_string("amplitude_normalization_method");
					//#could only be either GLOBAL_MAX, GLOBAL_FA_MAX or LOCAL_MAX
				killtmp=teo->kill_small_FA_traces(tse,min_normalized_FA_amplitude,
							FA_search_window, amplitude_normalization_method,
							FA_detect_length);
				cerr<<"Number of traces killed by applying ksfa = "<<killtmp.size()<<endl;
				if(killtmp.size()>0)
					rkills0.insert(killtmp.begin(),killtmp.end());
			}
			*/
			if(apply_kgpc)
			{	
				if(tse.member.size()<=0)
				{
					cerr<<"** Ensemble is empty. Skipped this kill procedure: kgpc."<<endl;
				}
				else
				{
					PCoda_search_window=TimeWindow(md.get_double("PCoda_search_TW_start")+FA_reference_time,
											md.get_double("PCoda_search_TW_end")+FA_reference_time);
					PCoda_grow_tolerance=md.get_double("PCoda_grow_tolerance");
					killtmp=teo->kill_growing_PCoda_traces(tse,
								PCoda_search_window,PCoda_grow_tolerance);
					cerr<<"Number of traces killed by applying kgpc = "<<killtmp.size()<<endl;
					if(killtmp.size()>0)
						rkills0.insert(killtmp.begin(),killtmp.end());
					killtmp.clear();
				}
			}
			if(apply_kca)
			{	
				if(tse.member.size()<=0)
				{
					cerr<<"** Ensemble is empty. Skipped this kill procedure: kca."<<endl;
				}
				else
				{
					CodaCA_search_window=TimeWindow(md.get_double("CodaCA_search_TW_start")+FA_reference_time,
											md.get_double("CodaCA_search_TW_end")+FA_reference_time);
					CodaCA_tolerance_twin_length=md.get_double("CodaCA_tolerance_twin_length");
					killtmp=teo->kill_ClusteredArrivals_traces(tse,
								CodaCA_search_window, CodaCA_tolerance_twin_length);
					cerr<<"Number of traces killed by applying kca = "<<killtmp.size()<<endl;
					if(killtmp.size()>0)
						rkills0.insert(killtmp.begin(),killtmp.end());
					killtmp.clear();
				}
			}
			if(apply_klfc)
			{
				////*****Commented out/blocked for now ********
				cerr<<"Warning: klfc is blocked in the current version of RFeditor. Skipped!"<<endl;
				/*
				LowFrequency_min=md.get_double("LowFrequency_min");
    			LowFrequency_max=md.get_double("LowFrequency_max");
    			
    			if(tse.member.size()<=0)
				{
					cerr<<"** Ensemble is empty. Skipped this kill procedure: klfc."<<endl;
				}
				else
				{
					killtmp=teo->kill_LowFrequencyContaminated_traces(
								tse,LowFrequency_min, LowFrequency_max);
					cerr<<"Number of traces killed by applying klfc = "<<killtmp.size()<<endl;
					if(killtmp.size()>0)
						rkills0.insert(killtmp.begin(),killtmp.end());
					killtmp.clear();
				}
				*/
			}

/*
***********************************************************************************
******************* SECTION-3: Statistics-Based procedures ***********************
***********************************************************************************
*/

			if(apply_klxcor ||apply_klsw || apply_klrfqi)
			{
				st_tmp=md.get_string("stacktype");
				if(st_tmp=="RobustSNR")
				{
					/*
					//use only RobustSNR stacking right now.
					case "BasicStack":
						stacktype=BasicStack;
					case "MedianStack":
						stacktype=MedianStack;
			*/
					stacktype=RobustSNR;
				}
				else
				{
					cerr<<"ERROR: unsupport stacktype in parameter file."<<endl;
					exit(-1);
				}
				robust_twin=TimeWindow(md.get_double("robust_window_start")+FA_reference_time,
									md.get_double("robust_window_end")+FA_reference_time);
				
			}
			if(apply_klxcor || apply_klrfqi)
			{
				cerr<<"**Caution: in GUIoff mode, stack trace is used as the reference trace."<<endl;
				
				ref_trace_xcor_twin=TimeWindow(md.get_double("RefXcor_search_TW_start")+FA_reference_time,
										md.get_double("RefXcor_search_TW_end")+FA_reference_time);
				//DEBUG
				//cout<<"ref_trace_xcor_twin set."<<endl;
			}

			if(apply_klsw)
			{
				min_stackweight=md.get_double("stackweight_min");
				//iteratively apply klsw
				long nkill_sw(tse.member.size());// initialize nkill as the ensemble size.
				int itn(1);
				while (nkill_sw>0)
				{
					TimeSeriesEnsemble tse_tmp=teo->exclude_false_traces(tse);
					if(tse_tmp.member.size()<=1)
					{
						cerr<<"** Only 1 trace or less left in ensemble. Skipped this Statistics-Based kill procedure: klsw."<<endl;
						tse_tmp.member.clear();
						nkill_sw=0;
						break;
					}
					else
					{
						TimeWindow stw=teo->find_common_timewindow(tse_tmp);
						//DEBUG
						//cout<<stw.start<<", "<<stw.end<<endl;
						if(robust_twin.start < stw.start || robust_twin.end > stw.end)
						{
							cerr<<"**Error in applying klsw (GUIoff): robust timewindow is bigger than stack timewindow."<<endl;
							exit(-1);
						}
						try{
						teo->get_stack(tse_tmp,stw, 
							robust_twin, stacktype);
						}catch(SeisppError& serr)
						{cerr<<"**Error when getting stack trace!"<<endl; serr.what();exit(-1);}
						killtmp=teo->kill_low_stackweight_traces(tse_tmp,min_stackweight);
						nkill_sw=killtmp.size();
							cerr<<"Number of traces killed by applying sw (iteration "<<itn
								<<" ) = "<<nkill_sw<<endl;
						if(nkill_sw>0)
						{
							rkills0.insert(killtmp.begin(),killtmp.end());
							teo->apply_kills(tse,killtmp);
						}
						itn++;
						tse_tmp.member.clear();
						killtmp.clear();
					}
				}
			}

			if(apply_klxcor)
			{
				cerr<<"**Caution: in GUIoff mode, stack trace is used as the reference trace."<<endl;
				min_xcorcoe=md.get_double("xcorcoe_min");
				
				//iteratively apply klxcor
				long nkill_xcor(tse.member.size());// initialize nkill as the ensemble size.
				int itn(1);
				while (nkill_xcor>0)
				{
					TimeSeriesEnsemble tse_tmp=teo->exclude_false_traces(tse);
					if(tse_tmp.member.size()<=1)
					{
						cerr<<"** Only 1 trace or less left in ensemble. Skipped this Statistics-Based kill procedure: klxcor."<<endl;
						tse_tmp.member.clear();
						nkill_xcor=0;
						break;
					}
					else
					{
						TimeWindow stw=teo->find_common_timewindow(tse);
						//DEBUG
						//cout<<stw.start<<", "<<stw.end<<endl;
						//check if robust window is bigger than stackwindow.
						if(robust_twin.start < stw.start || robust_twin.end > stw.end)
						{
							cerr<<"**Error in applying klxcor (GUIoff): robust timewindow is bigger than stack timewindow."<<endl;
							exit(-1);
						}
						TimeSeries ts_tmp;
						try{
						ts_tmp=teo->get_stack(tse_tmp,stw, 
							robust_twin, stacktype);
						}catch(...)
						{cerr<<"**Error when getting stack trace!"<<endl; throw;exit(-1);}
						killtmp=teo->kill_low_ref_correlation_traces(tse_tmp,ts_tmp,
							ref_trace_xcor_twin,min_xcorcoe);
						nkill_xcor=killtmp.size();
							cerr<<"Number of traces killed by applying xcor (iteration "<<itn
								<<" ) = "<<nkill_xcor<<endl;
						if(nkill_xcor>0)
						{
							rkills0.insert(killtmp.begin(),killtmp.end());
							teo->apply_kills(tse,killtmp);
						}
						itn++;
						tse_tmp.member.clear();
						ts_tmp.s.clear();
						killtmp.clear();
					}	
				}
			}
			if(apply_klrfqi)
			{
				try{
                min_rfqi=md.get_double("rfqi_min");
				RFQI_weights[0]=md.get_double("RFQI_weight_stackweight");
				RFQI_weights[1]=md.get_double("RFQI_weight_refxcorcoe");
				RFQI_weights[2]=md.get_double("RFQI_weight_successindex");
				}catch(SeisppError& serr)
						{cerr<<"**Error when getting RFQI parameters!"<<endl; serr.what();exit(-1);}
				if(use_decon_in_editing)
				{
					//compute decon success index.
					teo->compute_decon_success_index(tse);
				}
				else
				{
					RFQI_weights[0]=RFQI_weights[0]/(RFQI_weights[0]+RFQI_weights[1]);
					RFQI_weights[1]=RFQI_weights[1]/(RFQI_weights[0]+RFQI_weights[1]);
					RFQI_weights[2]=0.0;
					cout<<"** Warning: use_decon_in_editing is false. decon_success_index "<<endl
						<<"   weight is set to 0.0. Other weights are scaled to have sum of 1.0."<<endl;
				}
				//compute stackweight.
				//DEBUG		
				//iteratively apply klrfqi
				long nkill_rfqi(tse.member.size());// initialize nkill as the ensemble size.
				int itn(1);
				while (nkill_rfqi>0)
				{
					TimeSeriesEnsemble tse_tmp=teo->exclude_false_traces(tse);
					if(tse_tmp.member.size()<=1)
					{
						cerr<<"** Only 1 trace or less left in ensemble. Skipped this Statistics-Bsed procedure: klrfqi."<<endl;
						tse_tmp.member.clear();
						nkill_rfqi=0;
						break;
					}
					else
					{
						//DEBUG
						//cout<<"Getting common timewindow ..."<<endl;
						TimeWindow stw=teo->find_common_timewindow(tse);
						//DEBUG
                    	//cout<<stw.start<<", "<<stw.end<<endl;
						if(robust_twin.start < stw.start || robust_twin.end > stw.end)
						{
							cerr<<"**Error in applying klrfqi (GUIoff): robust timewindow is bigger than stack timewindow."<<endl;
							exit(-1);
						}
						//DEBUG
						//cout<<"Getting stackweight ..."<<endl;
						//cout<<"ensemble size="<<tse_tmp.member.size()<<endl;
						TimeSeries ts_tmp;
						try{
						ts_tmp=teo->get_stack(tse_tmp,stw, 
							robust_twin, stacktype);
						}catch(SeisppError& serr)
						{cerr<<"**Error when getting stack trace!"<<endl; serr.what();exit(-1);}
						//compute xcorcoe using stacktrace.
						//DEBUG
						//cout<<"Computing xcorcoe ..."<<endl;
						bool btmp=teo->compute_trace_xcorcoe(tse_tmp,ts_tmp, ref_trace_xcor_twin);
						if(!btmp) 
							cerr<<"Error in computing refxcorcoe. Skipped this kill procedure."<<endl;
						else
						{
							//DEBUG
							//cout<<"Applying klrfqi ..."<<endl;
							killtmp=teo->kill_low_RFQualityIndex_traces(tse_tmp,RFQI_weights[0],
									RFQI_weights[1],RFQI_weights[2],min_rfqi,true);
							nkill_rfqi=killtmp.size();
							cerr<<"Number of traces killed by applying rfqi (iteration "<<itn
								<<" ) = "<<nkill_rfqi<<endl;
							if(nkill_rfqi>0)
							{
								rkills0.insert(killtmp.begin(),killtmp.end());
								teo->apply_kills(tse,killtmp);
							}
						}	
						itn++;
						tse_tmp.member.clear();
						ts_tmp.s.clear();
						killtmp.clear();
					}
				}
			}
			nkilled=rkills0.size();

			/*
			set<long> tkills0;
			//Twindow.plot(td,true);
			//tkills0=Twindow.kills;
			if(tkills0.size()>0)
			{
				teo.apply_kills(rd,tkills0);
				teo.apply_kills(td,tkills0);
			}
			*/
			//*rkills=rkills0;
        //*tkills=tkills0;
        	return(rkills0);
        }
        else
        {
        	cerr<<"ERROR in RFeditorEngine::edit(). This is for GUIoff mode only!"<<endl;
        	exit(-1);
        }
    }catch(SeisppError &serr){serr.what(); throw;}catch(...){throw;};
}
//end of RFeditorEngine
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++