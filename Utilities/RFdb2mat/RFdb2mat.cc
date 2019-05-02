/*
 * matcreat.cpp - MAT-file creation program
 *
 * See the MATLAB External Interfaces/API Guide for compiling information.
 *
 * Calling syntax:
 *
 *   matcreat
 *
 * Create a MAT-file which can be loaded into MATLAB.
 *
 * This program demonstrates the use of the following functions:
 *
 *  matClose
 *  matGetVariable
 *  matOpen
 *  matPutVariable
 *  matPutVariableAsGlobal
 *
 * Copyright 1984-2007 The MathWorks, Inc.
 */
#include <stdio.h>
#include <string.h> /* For strcmp() */
#include <stdlib.h> /* For EXIT_FAILURE, EXIT_SUCCESS */
#include <vector> /* For STL */
#include <fstream>
#include <list>
#include <algorithm>
#include "dmatrix.h"
#include "seispp.h"
#include "SeisppKeywords.h"
#include "dbpp.h"
#include "TimeSeries.h"
#include "ThreeComponentSeismogram.h"
#include "ensemble.h"
#include "Metadata.h"
#include "mat.h"

using namespace std;
using namespace SEISPP;

#define BUFSIZE 256
const string tracetype_3c("3C"), tracetype_1c("1C");
const string MDL_ENSEMBLE("mdlist_ensemble"), MDL_WFDISCIN("mdlist_wfdisc_in"), 
			MDL_WFDISCOUT("mdlist_wfdisc_out"), MDL_WFPROCESSIN("mdlist_wfprocess_in"),
			MDL_WFPROCESSOUT("mdlist_wfprocess_out");
const bool MYDEBUGMODE(true);
const string evidkey("eventid");   // not evid to avoid collision

bool SEISPP::SEISPP_verbose(false);

const string csversion("v1.0.0");

void version()
{
	cerr <<"< version "<<csversion<<" > 5/2/2019"<<endl;
}
void author()
{
	cerr <<endl<<"Xiaotao Yang @ UMass Amherst"<<endl<<endl;
}
void usage_message()
{
    version();
    cerr << "RFdb2mat dbin outdir [-laststa xx][-ss subset_condition][-v|V][-h|H]"<<endl;
    cerr << "** Use -h to print out detailed explanations on the options."<<endl;
    author();
}
void help()
{
	usage_message();
	cout<<"Options for running mode:"<<endl
    	<<"TBA    :"<<endl;
    exit(0);
}
void usage()
{
	usage_message();
    exit(-1);
}

/* Simple algorithm sets an int metadata item (evidkey defined in 
   RFeditorEngine) by a simple counts starting at 1.  
   The algorith is very simple.  It assumes the data were already sorted
   by sta:time:chan.  The approach then is to work through the ensemble
   and whenever t0 of successive traces are within one sample dt group 
   them together.   
   */
void set_eventids(TimeSeriesEnsemble& d)
{
	try
	{
		vector<TimeSeries>::iterator dptr;
		int i,evid;
		double t0test(0.0);
		/* Start with evid 0 to match vector index when not sorted.
		   confusing otherwise */
		for(evid=0,i=0,dptr=d.member.begin();dptr!=d.member.end();
				++i,++dptr)
		{ /*
			cout << "Member number "<<i<< "Metadata contens"<<endl
				<< dynamic_cast<Metadata &>(*dptr)<<endl;
				*/
			if(dptr->is_attribute_set((char *)"evid"))
					dptr->put(evidkey,dptr->get_int("evid"));
			else
			{
				if(i==0)
				{	dptr->put(evidkey,evid);
					t0test=dptr->t0;
				}
				else
				{	/* elegantly obscure C syntax - test if t0 of this member
					   is with dt 0 test value */
					if(fabs(t0test-(dptr->t0))<dptr->dt)
					{	dptr->put(evidkey,evid);}
					else
					{
						++evid;
						t0test=dptr->t0;
						dptr->put(evidkey,evid);
					}
				}
			}
			//DEBUG
			//cout << "Member number "<<i<<" assigned evid "<<evid<<endl;
		}
	}catch(...){throw;};
}
// Same functionality for ThreeComponentEnsemble object.
void set_eventids(ThreeComponentEnsemble& d)
{
    try
    {
		vector<ThreeComponentSeismogram>::iterator dptr3c;
		//vector<TimeSeries>::iterator dptr;
		int i,evid;
		//double t0test(0.0);
		/* Start with evid 0 to match vector index when not sorted.
		   confusing otherwise */
		for(evid=0,dptr3c=d.member.begin();dptr3c!=d.member.end();
				++evid,++dptr3c)
		{
			if(dptr3c->is_attribute_set((char *)"evid"))
				dptr3c->put(evidkey,dptr3c->get_int("evid"));
			else
				dptr3c->put(evidkey,evid);
			//DEBUG
			//cout<<"Set evid = "<<evid<<endl;
		}
	}catch(...){throw;};
}

/*
// input d has to be the ensemble including only one channel, i.e.,
// result by calling extract_by_chan(). Otherwise, this routine will
// return wrong result.
// assume the data has been sorted by evid.
*/
int set_duplicate_traces_to_false(TimeSeriesEnsemble& d, bool verbose)
{
	try{
	vector<TimeSeries>::iterator dptr;
	
    int i=0,evid; //,evid_tmp=-99;
    set<int> previous_evids;
    set<int>::iterator it;
    //double timeres(0.0);
    //dptr_tmp=d.member.begin();
    for(dptr=d.member.begin();dptr!=d.member.end();++dptr)
	{	
		evid=dptr->get_int(evidkey);
		if(previous_evids.find(evid)!=previous_evids.end())
		{
			if(verbose){
				cout<<"Duplicate evid = "<<evid
				<<" of chan = "<<dptr->get_string("chan")
				<<", set trace status to FALSE!"<<endl;}
			dptr->live=false;
			++i;
		}
		else
		{
		//	evid_tmp=evid;
			previous_evids.insert(evid);
		}
	}
	//DEBUG
	//exit(-1);
	return(i);
	}catch(...){throw;};
}

TimeSeriesEnsemble extract_by_chan(TimeSeriesEnsemble& d,string chankey)
{
    try {
        int nd=d.member.size();
        int ns0=d.member[0].s.size();
        if(nd==0) throw SeisppError(string("extract_by_chan:  ")
                + "input ensemble has no data");
        /* Inefficient to copy, but this way we get metadata copied.  Use
           until proven to be a barrier */
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();i<d.member.size();++i,++dptr)
        {
            string chan=dptr->get_string("chan");
            size_t pos=chan.find(chankey);
            if(pos!=chan.npos)
                result.member.push_back(*dptr);
        }
        //DEBUG
        //cout<<"extraced number of traces = "<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}

//mdltype: ensemble, wfprocessin, wfdiscin, wfprocessout, wfdiscout
MetadataList generate_mdlist(string mdltype, bool use_arrival_data=false, bool use_netmag_table=false,
			bool use_decon_in_editing=false)
{
	try
	{
		MetadataList mdlist;
		Metadata_typedef metadata;
	
		if(mdltype=="ensemble")
		{
			metadata.tag="sta";
			metadata.mdt=MDstring;
			mdlist.push_back(metadata);
		}
		else if(mdltype=="wfprocessin")
		{
			metadata.tag="pwfid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="dir"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="dfile"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="time"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="endtime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="nsamp"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="samprate"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="datatype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="timetype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="foff"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfprocess.algorithm"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="evid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="sta"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="chan"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			if(use_decon_in_editing)
			{
				metadata.tag="decon.nspike"; metadata.mdt=MDint; mdlist.push_back(metadata);
				metadata.tag="decon.rawsnr"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="decon.averamp"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="decon.epsilon"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="decon.niteration"; metadata.mdt=MDint; mdlist.push_back(metadata);
				metadata.tag="decon.peakamp"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="decon.chan"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			}
			if(use_arrival_data)
			{
				metadata.tag="atime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="arrival.sta"; metadata.mdt=MDstring; mdlist.push_back(metadata);
				metadata.tag="assoc.esaz"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="assoc.seaz"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			}
			if(use_netmag_table)
			{
				metadata.tag="magtype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
				metadata.tag="magnitude"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			}
		}
		else if(mdltype=="wfprocessout")
		{
			metadata.tag="pwfid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="time"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="endtime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="dir"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="dfile"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="datatype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="timetype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="samprate"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="nsamp"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="foff"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfprocess.algorithm"; metadata.mdt=MDstring; mdlist.push_back(metadata);
		}
		else if(mdltype=="wfdiscin")
		{
			metadata.tag="wfdisc.chan"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.time"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="wfdisc.wfid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfdisc.chanid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfdisc.jdate"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfdisc.endtime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="wfdisc.nsamp"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfdisc.samprate"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="wfdisc.calib"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="wfdisc.calper"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="wfdisc.instype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.segtype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.datatype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.clip"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.dir"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.dfile"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="wfdisc.foff"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="wfdisc.commid"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="sta"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="chan"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="time"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="nsamp"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="samprate"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			if(use_arrival_data)
			{
				metadata.tag="atime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="arrival.sta"; metadata.mdt=MDstring; mdlist.push_back(metadata);
				metadata.tag="assoc.esaz"; metadata.mdt=MDreal; mdlist.push_back(metadata);
				metadata.tag="assoc.seaz"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			}
			if(use_netmag_table)
			{
				metadata.tag="magtype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
				metadata.tag="magnitude"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			}
		}
		else if(mdltype=="wfdiscout")
		{
			metadata.tag="sta"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="chan"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="time"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="endtime"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="nsamp"; metadata.mdt=MDint; mdlist.push_back(metadata);
			metadata.tag="samprate"; metadata.mdt=MDreal; mdlist.push_back(metadata);
			metadata.tag="datatype"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="dir"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="dfile"; metadata.mdt=MDstring; mdlist.push_back(metadata);
			metadata.tag="foff"; metadata.mdt=MDint; mdlist.push_back(metadata);
		}
		else
		{
			cerr<<"ERROR in generate_mdlist(): wrong metadata type!"<<endl;
			exit(-1);
		}
	
		return(mdlist);
	}catch(...) {throw;};
}

void append_decon_attribute_keys(Metadata& md)
{
	try
	{
		// decon attribute key definition.
		if(!md.is_attribute_set((char *)"decon_nspike_key")) 
			md.put("decon_nspike_key", (char *)"decon.nspike");
		if(!md.is_attribute_set((char *)"decon_rawsnr_key")) 
			md.put("decon_rawsnr_key",(char *)"decon.rawsnr");
		if(!md.is_attribute_set((char *)"decon_averamp_key")) 
			md.put("decon_averamp_key",(char *)"decon.averamp");
		if(!md.is_attribute_set((char *)"decon_epsilon_key")) 
			md.put("decon_epsilon_key",(char *)"decon.epsilon");
		if(!md.is_attribute_set((char *)"decon_niteration_key")) 
			md.put("decon_niteration_key",(char *)"decon.niteration");
		if(!md.is_attribute_set((char *)"decon_peakamp_key")) 
			md.put("decon_peakamp_key",(char *)"decon.peakamp");
	}catch(...) {throw;};
}
/*
Save one ThreeComponentSeismogram object to MATLAB *.mat file.
*/
int save2mat(string outdir, string filename, ThreeComponentSeismogram& tcs,
			MetadataList& mdl) 
{
  MATFile *pmat;
  mxArray *pa1, *pa2, *pa3;
  std::vector<int> myInts;
  myInts.push_back(1);
  myInts.push_back(2);
  printf("Accessing a STL vector: %d\n", myInts[1]);

  double data[9] = { 1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0 };
  const char *file = "mattest.mat";
  char str[BUFSIZE];
  int status; 

  printf("Creating file %s...\n\n", file);
  pmat = matOpen(file, "w");
  if (pmat == NULL) {
    printf("Error creating file %s\n", file);
    printf("(Do you have write permission in this directory?)\n");
    return(EXIT_FAILURE);
  }

  pa1 = mxCreateDoubleMatrix(3,3,mxREAL);
  if (pa1 == NULL) {
      printf("%s : Out of memory on line %d\n", __FILE__, __LINE__); 
      printf("Unable to create mxArray.\n");
      return(EXIT_FAILURE);
  }

  pa2 = mxCreateDoubleMatrix(3,3,mxREAL);
  if (pa2 == NULL) {
      printf("%s : Out of memory on line %d\n", __FILE__, __LINE__);
      printf("Unable to create mxArray.\n");
      return(EXIT_FAILURE);
  }
  memcpy((void *)(mxGetPr(pa2)), (void *)data, sizeof(data));
  
  pa3 = mxCreateString("MATLAB: the language of technical computing");
  if (pa3 == NULL) {
      printf("%s :  Out of memory on line %d\n", __FILE__, __LINE__);
      printf("Unable to create string mxArray.\n");
      return(EXIT_FAILURE);
  }

  status = matPutVariable(pmat, "LocalDouble", pa1);
  if (status != 0) {
      printf("%s :  Error using matPutVariable on line %d\n", __FILE__, __LINE__);
      return(EXIT_FAILURE);
  }  

  status = matPutVariable(pmat, "LocalString", pa3);
  if (status != 0) {
      printf("%s :  Error using matPutVariable on line %d\n", __FILE__, __LINE__);
      return(EXIT_FAILURE);
  } 
  
  /* clean up */
  mxDestroyArray(pa1);
  mxDestroyArray(pa2);
  mxDestroyArray(pa3);

  if (matClose(pmat) != 0) {
    printf("Error closing file %s\n",file);
    return(EXIT_FAILURE);
  }

  printf("Done\n");
  return(EXIT_SUCCESS);
}

/*==================================================================
//==================================================================
//====================== Main program ==============================
//==================================================================
//==================================================================
*/
int main(int argc, char **argv)
{
    const string errorbase("Error in RFeditor: ");
    switch(argc)
    {
    	case 1:
    		usage();
    		break;
    	case 2:
    		string sarg(argv[1]);
    		if(sarg=="-h" || sarg=="-H")
    			help();
    		else 
    			usage();
    		break;
    }
    
    string dbin_name(argv[1]);
    string outdir(argv[2]);

    //string stacktype("r");  // set default stacktype as RobustSNR. Xiaotao Yang
    string ques;
    bool apply_subset(false);
    bool datatype3c(false);
    string subset_condition("");
    string filterspec;
	/* quotes needed around subsets because sometimes
	station names start with numbers that confuse datascope */
	const string quote("\"");
    int i;
    for(i=3;i<argc;++i)
    {
        string sarg(argv[i]);
        if(sarg=="-ss")
        {
            ++i;
            if(i>=argc) usage();
            apply_subset=true;
            if(subset_condition.length()> 0)
            	subset_condition=subset_condition+string("&&")+string(argv[i]);
            else
	            subset_condition=string(argv[i]);
        }
        else if(sarg=="-laststa")
        {
            ++i;
            if(i>=argc) usage();
            apply_subset=true;
            if(subset_condition.length()> 0)
            	subset_condition=subset_condition+string("&&")+string("sta > ")
            		+quote+string(argv[i])+quote;
            else
            	subset_condition=string("sta > ")+quote
                	+string(argv[i])+quote;
        }
        else if(sarg=="-v" || sarg=="-V")
        	SEISPP_verbose=true;
        else if(sarg=="-h" || sarg=="-H")
        	help();
        else
            usage();
    }

    try {
			/****************************************************************
			*****************************************************************
			*******<<<<<<<<<<<< READ IN CONTROL PARAMETERS >>>>>>>>>>********
			*****************************************************************
			*****************************************************************
			*/
//         Metadata control(pf);
        Metadata trace_edit_params;

        ///*Read in parameters from the pf*/
        MetadataList mdlens=generate_mdlist("ensemble"); //=pfget_mdlist(pf,"ensemble_mdl");
        
        MetadataList mdl;
        MetadataList mdlout, mdlout_wfd, mdlout_wfp;
        bool use_arrival_data=true; //control.get_bool("use_arrival_data");

        //read in preference for use of netmag table
        bool use_netmag_table=true;//control.get_bool("use_netmag_table");
        trace_edit_params.put("use_netmag_table",use_netmag_table);
		bool use_wfdisc_in=false;//control.get_bool("use_wfdisc_in");

        trace_edit_params.put("use_arrival_data",use_arrival_data);

        //this is used when converting from a time to r time.
        // this value is the FA time set to the trace for display. The
        //default FA time will be 0.
        double FA_reference_time=0; //=control.get_double("FA_reference_time");

        bool use_decon_in_editing(false);
        bool save_3C_data(false);
        bool save_decon_table(false);
        bool no_vertical_data(false);
        string rchan("I1"), tchan("I0"), zchan("I2");

        if(use_wfdisc_in)
        {
			mdl=generate_mdlist("wfdiscin",use_arrival_data, use_netmag_table);

        }
        else
		{			
			mdl=generate_mdlist("wfprocessin",use_arrival_data, 
        						use_netmag_table,use_decon_in_editing);

        }
        
        //put these two parameters into the metadata for trace-editing.
		if(use_decon_in_editing) append_decon_attribute_keys(trace_edit_params);
        trace_edit_params.put("FA_reference_time",FA_reference_time);
// 
//         string rchan=control.get_string("radial_channel_key");
//         string tchan=control.get_string("transverse_channel_key");
//         string zchan=control.get_string("vertical_channel_key");
// 

//         bool apply_prefilter=control.get_bool("apply_prefilter");
        bool save_vertical_channel(true);
//         if(!save_3C_data && !no_vertical_data) 
//         	save_vertical_channel=control.get_bool("save_vertical_channel");

        //string outdir=control.get_string("output_dir");
        string outdfile_base="RFs";//control.get_string("output_dfile_base");

				/****************************************************************
				*****************************************************************
				*******<<<<<<<<<<< BUILD WAVEFORM WORKING VIEWS >>>>>>>>>********
				*****************************************************************
				*****************************************************************
				*/
        AttributeMap am("css3.0");
        /* Open the in and out database */
        DatascopeHandle dbin(dbin_name,true);
        if(use_wfdisc_in) 
        {
        	//dbin.lookup("wfdisc"); 
        		//read in arrival for wfdisc need to be added later.
        	if(use_arrival_data)
        	{
				dbin.lookup("event");
				dbin.natural_join("origin");
				dbin.subset("orid==prefor");
				dbin.natural_join("assoc");
				dbin.natural_join("arrival");
				string phase_subset;
				phase_subset="phase=~/P/";
				dbin.subset(phase_subset);
				if(SEISPP_verbose) 
					cout<< "Size of catalog view ="<< dbin.number_tuples()<<endl;
				list<string> j1,j2;
				j1.push_back("sta");
				j1.push_back("wfdisc.time::wfdisc.endtime");
				j2.push_back("sta");
				j2.push_back("arrival.time");
				dbin.leftjoin(string("wfdisc"),j1,j2);
			
				if(SEISPP_verbose) 
					cout<< "Size of working view after joining with wfdisc ="
						<< dbin.number_tuples()<<endl;
			}
			else
			{
				dbin.lookup("wfdisc");
				if(SEISPP_verbose) 
					cout<< "Size of working view in wfdisc ="
						<< dbin.number_tuples()<<endl;
			}
        }
        else
        //evlink, sclink tables are required when using wfprocess table as input.
        //on the other hand, decon table is optional in case the user doesn't have decon table.
        {
        	if(SEISPP_verbose) cout<<"Building waveform view ..."<<endl;
        	dbin.lookup("wfprocess");
        	dbin.natural_join("evlink");
        	dbin.natural_join("sclink");
        	dbin.db.record=0;
        	string datatype=dbin.get_string("datatype");
        	if(datatype=="c3")	datatype3c=true;
        	else
        		save_3C_data=false; //overwrite the value read from pf. 
        	if(use_decon_in_editing)
        	{	
        		dbin.natural_join("decon");
				/*
				//3c datatype uses one signle row to store 3 component data.
				//not the correct way but reasonable for trace editing (since we only edit radial
				//traces) here we set the subset condition to subset the view after join with decon
				//resulting in only R chan. decon attributes for R chan will be read in as the 
				//metadata for all three components.
				*/
// 				
// 				if(datatype3c)
// 				{
// 					string sstring=string("decon.chan==")+quote+string(edit_chan_code)+quote;
// 					dbin.subset(string(sstring));
// 				}
			}
			
			if(dbin.number_tuples()<=0)
			{
				cerr<<"Waveform view has no data after joining: "<<
					"wfprocess+evlink+sclink+decon (if applicable)."<<endl;
				exit(-1);
			}
			
			if(use_arrival_data)
			{
				if(SEISPP_verbose) cout<<"Building catalog view ..."<<endl;
				DatascopeHandle ljhandle(dbin);
				try{
					ljhandle.lookup("event");
					ljhandle.natural_join("origin");
					ljhandle.natural_join("assoc");
					ljhandle.natural_join("arrival");
					//ljhandle.subset("sta==arrival.sta");
					ljhandle.subset("orid==prefor");
					ljhandle.subset("iphase=~/P/");
				}catch(SeisppError& serr)
				{
					cerr<<"Error in building: origin+assoc+arrival and subset with "
						<<"sta=arrival.sta & orid==prefor & phase=~/P/"<<endl;
					serr.log_error();
				}
				list<string> jk;
				jk.push_back("evid");
				jk.push_back("sta");
				dbin.join(ljhandle,jk,jk);
				if(SEISPP_verbose) cout<<"Number of rows after joining with catalog view: "
										<<dbin.number_tuples()<<endl;
				if(dbin.number_tuples()<=0)
				{
					cerr<<"Working view has no data after joining with: "
						<<"origin+assoc+arrival and subset with sta=arrival.sta "
						<<"& orid==prefor &phase=~/P/"<<endl;
					exit(-1);
				}
			}
        }
        			
		//read in netmag information.
		if(use_netmag_table)
		{
			try{
				if(SEISPP_verbose) cout<<"Joining with netmag table ..."<<endl;
				dbin.natural_join("netmag");
			}catch(SeisppError& serr)
			{
				cerr<<"Error in joining with netmag table!"<<endl;
				serr.log_error();
			}
		}
        
        //cerr<<"test"<<endl;
        if(dbin.number_tuples()<=0)
        {
            cerr << "No rows in input wfdisc/wfprocess table for database "
            	<<dbin_name<<endl;
            exit(-1);
        }

        /* Prep the input wfdisc table*/
        /* WARNING:  not adequate.  This needs to be changed as we 
           need evid.  May be able to fake this by searching for matching
           start times  */
        if(apply_subset)
        {
            cout << "Applying subset condition = [ "<<subset_condition<<" ]."<<endl;
            dbin.subset(subset_condition);
            cout << "Subset view number of rows = "<<dbin.number_tuples()<<endl;
        }
        list<string> sortkeys, groupkeys;
        sortkeys.push_back("sta");
        sortkeys.push_back("time");
        sortkeys.push_back("chan");
        groupkeys.push_back("sta");
        dbin.sort(sortkeys);
        dbin.group(groupkeys);
        cout << "Number of ensembles to process (grouped by sta)="
            << dbin.number_tuples()<<endl;

        		/****************************************************************
				*****************************************************************
				******<<<<<<<<<<<< BUILD GLOBAL OBJECTS >>>>>>>>>>*******
				*****************************************************************
				*****************************************************************
				*/

        int nsta=dbin.number_tuples(),ntrace(0),nradial(0);
        string sta,tracetype;
        		/****************************************************************
				*****************************************************************
				*************<<<<<<<<<<<< START MAIN LOOP >>>>>>>>>>*************
				*****************************************************************
				*****************************************************************
				*/
        vector<TimeSeries>::iterator im;
        TimeSeriesEnsemble radial,transverse,vertical,tse_edit0,tse_edit; //radial0

// 		for(i=0,dbin.rewind();i<nsta;++i,++dbin)
       for(i=0,dbin.rewind();i<1;++i,++dbin)
        {   
            cout <<">>++++++++++++++++++++++++++++++"<<endl
            	<<"Calling data reader for ensemble number ["<<i+1<<" / "<<nsta<<"]"<<endl;
            TimeSeriesEnsemble dall;
            ThreeComponentEnsemble dall_3c;//,dall_3c_bkp;
            /*
			*****************************************************************
			*****************************************************************
			<<<<<<<<<<<<< Preparing Working TimeSeries Ensemble >>>>>>>>>>>>>
			*****************************************************************
			*****************************************************************
			*/
            if(!datatype3c)  //TimeSeriesEnsemble input table
            {
            	TimeSeriesEnsemble dall0(dbin,mdl,mdlens,am);
            	dall=dall0;
            	dall0.member.clear();
            	sta=dall.get_string("sta");
            	ntrace=dall.member.size();
            	if(no_vertical_data) ntrace=ntrace/2;
            	else ntrace=ntrace/3;
            	tracetype=const_cast<char *>(tracetype_1c.c_str());
				cout << "Read "<<dall.member.size()<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;

				/* This routine sets a metadata item eventid based
				   on start time only.  Not a bulletproof approach but one 
				   that should work for RF data */
				if(SEISPP_verbose) cout<<"Setting event IDs ..."<<endl;
				set_eventids(dall);
				/* use start time as 0.  Also set moveout keyword
				to allow stacking in engine */
				//use user given data_start_time.
				double atime, t0, moveout;
				for(im=dall.member.begin();im!=dall.member.end();++im)
				{
					if(use_arrival_data)
        			{
						atime=im->get_double("atime");
						im->ator(atime-FA_reference_time);
						//DEBUG
						//cout<<"t0="<<im->t0<<", endtime="<<im->endtime()<<endl;
					}
					else
					{
						t0=im->t0;
						im->ator(t0);
					}
						//moveout=atime-t0;
						//cout<<"arrival time = "<<strtime(atime)<<endl;
						//cout<<im2->get_int("evid")<<"   "<<moveout<<endl; 
					
					im->put(moveout_keyword,0.0);
				}

				/* Could do this with the database, but I chose this 
				   algorithm because I think it will be more robust.
				   Main reason is I can use only a string fragment for
				   a match instead of demanding a full match */

				try 
				{
					radial=extract_by_chan(dall,rchan);
					cout << "Found "<<radial.member.size()
						<<" radial component RFs"<<endl;
					transverse=extract_by_chan(dall,tchan);
					cout << "Found "<<transverse.member.size()
						<<" transverse component RFs"<<endl;
					if(!no_vertical_data)
					{	if(save_vertical_channel) 
						{
							vertical=extract_by_chan(dall,zchan);
							cout << "Found "<<vertical.member.size()
								<<" vertical component RFs"<<endl;
						}
					}
					else
					{
						vertical=radial;
						for(im=vertical.member.begin();im!=vertical.member.end();++im)
						{	
							im->put("chan",zchan);
							std::fill(im->s.begin(),im->s.end(),0.0);
						}
					}	
				}catch(SeisppError& serr)
				{
					cerr << "Problems in extract_by_chan.  Message "
						<<serr.what()<<endl
						<<"Skipping ensemble for station = "<<sta
						<<endl;
					continue;
				}
				
				dall.member.clear();	
            }
            // load 3 component data from the view.
            //
            else
            {
            	//try{
            	ThreeComponentEnsemble dall0_3c(dbin, mdl,mdlens,am);

            	dall_3c=dall0_3c;
            	dall0_3c.member.clear();
            	sta=dall_3c.get_string("sta");
            	ntrace=dall_3c.member.size();
            	//nradial=ntrace;
            	tracetype=const_cast<char *>(tracetype_3c.c_str());
				cout << "Read "<<ntrace<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;

				/* This routine sets a metadata item eventid based
				   on start time only.  Not a bulletproof approach but one 
				   that should work for RF data */
				if(SEISPP_verbose) cout<<"Setting event IDs ..."<<endl;
				set_eventids(dall_3c);
				/* use start time as 0.  Also set moveout keyword
				to allow stacking in engine */
				vector<ThreeComponentSeismogram>::iterator im2;
				double atime, t0, moveout;
				for(im2=dall_3c.member.begin();im2!=dall_3c.member.end();++im2)
				{
					//cout<<strtime(im2->t0)<<endl;
					if(use_arrival_data)
        			{
						atime=im2->get_double("atime");
						im2->ator(atime-FA_reference_time);
					}
					else
					{
						t0=im2->t0;
						im2->ator(t0);
					}
					//moveout=atime-t0;
					//cout<<"arrival time = "<<strtime(atime)<<endl;
					//cout<<im2->get_int("evid")<<"   "<<moveout<<endl; 
					
					im2->put(moveout_keyword,0.0);
				}		
				
            }
			
            cout<<"-- Excluding duplicates and false traces ..."<<endl;

			cout<<"Saving to MAT files ..."<<endl;  
			dall_3c.member.clear();    
        }
        radial.member.clear();
        transverse.member.clear();
        vertical.member.clear();

        if(SEISPP_verbose) cout<<"RFdb2mat is finished."<<endl;

    }catch(SeisppError& serr)
    {
        serr.log_error();
    }
    catch(exception& stdexcept)
    {
        cerr << "Exception thrown:  "<<stdexcept.what()<<endl;
    }
}
//END