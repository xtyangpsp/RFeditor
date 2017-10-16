#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>
#include "stack.h"
#include "seispp.h"
#include "SeisppKeywords.h"
#include "dbpp.h"
#include "TimeSeries.h"
#include "ThreeComponentSeismogram.h"
#include "ensemble.h"
#include "Metadata.h"
#include "SimpleWavelets.h"
#include "RFeditorEngine.h"
#include "TraceEditPlot.h"
#include "TraceEditOperator.h"
#include "filter++.h"
using namespace std;
using namespace SEISPP;
//const string evidkey("eventid");   // not evid to avoid collision
const string tracetype_3c("3C"), tracetype_1c("1C");
const string MDL_ENSEMBLE("mdlist_ensemble"), MDL_WFDISCIN("mdlist_wfdisc_in"), 
			MDL_WFDISCOUT("mdlist_wfdisc_out"), MDL_WFPROCESSIN("mdlist_wfprocess_in"),
			MDL_WFPROCESSOUT("mdlist_wfprocess_out");
/* Modification History */
void history_old()
{
	cout<<"***** Full Detailed Modification History (older versions) *****"<<endl
<<">> Jan 2015-Feb 2015 Xiaotao Yang "<<endl
	<<"1. add option to continue or terminate after each event following Gary's suggestion."<<endl
	<<"2. add option in augument for stack type: -st b|m|r. only r is working for now. "<<endl
	<<"	Debugging is needed for b & m."<<endl
	<<"3. added prefilter before stacking for both radial and transverse components;"<<endl
	<<"4. added verbose option."<<endl
	<<"5. added option to skip transverse componnets in pf file. gave up(incomplete)!!"<<endl
	<<"6. added option in argument to turn on the review mode by setting review_mode to true; "<<endl
	<<"	This is useful when quickly reviewing large dataset without doing edits."<<endl
	<<"7. output data into a new dir with name of dfile. read in those values from the pf."<<endl
	<<"8. added ability to save only the metadata into a new wfdisc table. "<<endl
	<<"	This option (set save_metadata_only in pf to true to turn this on) is useful when we want to avoid duplicating the waveform data. This saves the good traces after editing. When this is on, the ouput dir and dfile_base set in pf are ignored."<<endl
	<<"9. added option to turn on continue mode when the output db is not empty."<<endl
	<<"10. added option to save either filtered or unfiltered data (set save_filtered_data in pf)."<<endl
	<<"11. added option to do auto kill by removing negative FA traces, and/or small FA traces within give time window."<<endl
	<<"12. bug: duplicate evid."<<endl
	<<"13. added auto-killing option to kill traces with extremely large amplitude."<<endl
	<<"14. added option to save vertical data. This is useful if the edited data will be rotated back to NEZ."<<endl
	<<"15. added the auto-editing procedures to TraceEditPlot functionalities."<<endl
	<<"16. removed stacktype option in argument. move it to pf parameter."<<endl
	<<"17. added kill by setting decon parameter thresholds."<<endl
<<">> Mar 2015 Xiaotao Yang"<<endl
	<<"1. Added option to run in GUI off mode. This mode reads in auto-editing parameters and do editing without graphics."<<endl
	<<"2. Added kill by RF quality index."<<endl
<<">> April 2015 Xiaotao Yang"<<endl
	<<"1. add option to save wfdisc table when reading from wfprocess table. Eventually may add the option to save wfprocess table when reading from wfdisc table."<<endl
	<<"2. added the requirement for arrival information. thus additional tables are needed including arrival, assoc, event, origin."<<endl
	<<"3. removed ricker wavelet option for filter type. ricker wavelet is not suitable for RFeditor purpose."<<endl
		<<endl
<<">> May 2015 Xiaotao Yang"<<endl
	<<"1. This RFeditor3.0 package has some big changes made compared to 2.5 version. Those	changes are:"<<endl
	<<"	(1). from kill negative FA to kill negative first arrivals within the given timewindow, named NFA_tolerance_window. That means if all the first arrivals within that window are <=0.0, the trace will be killed/dropped."<<endl
	<<"	(2). from comparing P-coda max with FA to comparing P-coda_max with the max within the timewindow before/prior to the P-coda search window."<<endl
	<<"	(3). Coda Zero Point method is replaced by Coda Clustered Arrivals (CodaCA) method. The focus is refined to detect arrival clusters with given tolerance."<<endl
	<<"	(4). Kill Low-frequency-contaminated traces is added."<<endl
<<">> Fixed bugs (July 5 - July 12, 2015):"<<endl
	<<"	(1) in RFeditorEngine.cc, kill lsw and kill lxcor don't apply kills to master tse. Bug fixed by adding apply_kills() to the master tse. Otherwise, there will be duplicate kills if both procedures are applied. The tredit summary may be wrong when rfqi is applied together with kill lsw and/or kill lxcor."<<endl
	<<"	(2) some traces may have t0 > FA_reference_time. This makes the samplenumber negative for some samples. Don't know exactly why and how this happends. Temporarily fixed by comparing t0 and FA_reference_time and set the trace to FALSE if t0 > FA_reference_time. Here all times are in reference time frame."<<endl
	<<"	(3) when traces are not the same length, program will crash in the step of convolve_ensemble() when the data need to be trimed. The original code pass timewindow as length of the first member in the ensemble. Whe other traces have shorter length. This is a problem. This has been fixed by pass a timewindow from find_common_timewindow()."<<endl
<<">> (7/15/2015) "<<endl
	<<"Reordered editing procedures to be: decon first, trace_based second, statistical method the third. "<<endl
	<<"	Decon threshold: 6 decon parameters + DSI;"<<endl
	<<"	Trace based: NFAs, GPC, and Coda CAs, LFC is categorized into this section but is not recommended in this version (still to be improved). User will get a warning message if the apply_klfc is turned on."<<endl
	<<"	Statistical methods: cutoff of SW, CORR, RFQI."<<endl
<<">> 7/16/2015 Xiaotao Yang"<<endl
	<<"	(1) read in edit_on_channel from pf. the editing channel may not be radial."<<endl
<<">> 7/22/2015 XT Yang"<<endl
	<<"	(1) following bugs fixed: can't customize NFA window, read chan code from ensemble but chan is not defined there (fixed by putting chan code to the ensemble). "<<endl
<<">> 7/28/2015 XT Yang"<<endl
	<<"	(1) fixed bugs of: stack weight not set for single-trace ensemble, some errors relating to get_stack() were not catched and displayed, seisw can't handle empty ensemble (keep current view for fixing this bug)."<<endl
	<<"	(2) put 'Manual' killmethodkey for all traces before any editing. this is required since otherwise, we will get error when we want to review manually kill traces."<<endl
<<">> 8/3/2015 XT Yang"<<endl
    <<" (1) put apply_lat as the first one if it is on."<<endl
    <<" (2) add checking for robust tw and stack tw and comparing them. exit with error if robust twin is bigger than stack tw."<<endl
    <<" (3) version 3.6.0: iteratively apply klrfqi."<<endl
<<">> 8/19/2015 XT Yang"<<endl
	<<" (1) read optional outdir and summary file name from command line arguments."<<endl
<<">> 12/04/2015 XT Yang"<<endl
	<<" (1) fixed the bug that when arrival table is not used, the program still was trying to read arrival when computing post edit FA information."<<endl
	<<endl;
}

void history_current()
{
	cout<<"***** Modification History for version: 3.7.x *****"<<endl
<<">> 12/31/2015 XT Yang"<<endl
	<<" (1) added option to use netmag table in sorting traces."<<endl
	<<" (2) upgraded metadata version/speficier to 2 (added magnitude attributes) when saving trace to file."<<endl
<<">> 02/26/2016 XT Yang"<<endl
	<<" (1) fixed the bug that when ensemble has only 1 trace left, get_stack() will throw error and exit."<<endl
	<<"     The stack weight is set to 1.0. The only one trace is assigned to the stacked trace."<<endl
<<">> 03/10/2016 XT Yang"<<endl
	<<" (1) klsw and klxcor are now iterative."<<endl
<<">> 03/11/2016 XT Yang"<<endl
	<<" (1) option of read xcor window from pf in GUI mode."<<endl
	<<" (2) show seaz in metadata."<<endl
	<<" (3) built Tools menu, moved Statistics to Tools menu from Help menu."<<endl
	<<" (4) renamed a few menu labels."<<endl
	<<" (5) turn on continue mode in argument: -continue."<<endl
<<">> 03/13/2016 XT Yang"<<endl
	<<" (1) use tredit table v2 for output table with one line for each method."<<endl
	<<" (2) moved klsw up one level before klxcor."<<endl
<<">> 03/14/2016 XT Yang"<<endl
	<<" (1) updated tredit v2 table and corresponding routines by adding 'version' entry."<<endl
<<">> 03/15-17/2016 XT Yang"<<endl
	<<" (1) blocked LowFrequencyContaminated procedure. Removed dependency on GSL library."<<endl
	<<" (2) cleaned up lines of old debries. tested wfdisc in and out."<<endl
	<<" (3) read metadata lists from pf is optional. they are built in now. values in pf will overwrite the built-in values."<<endl
<<">> 03/21/2016 XT Yang"<<endl
	<<" (1) fixed a bug in setting beam_plot metadata."<<endl
<<">> 05/02/2016 - 5/4/2016 XT Yang"<<endl
    <<" (1) added throw details when there is an error in GUIOFF edit."<<endl
    <<" (2) fixed a bug in saving to wfprocess table."<<endl
<<">> 10/16/2017 XT Yang"<<endl
	<<" (1) added catch errors when getting RFQI parameters in RFeditorEngine.cc."<<endl
	<<endl;
}

const string csversion("v3.7.1");

void version()
{
	cerr <<"< version "<<csversion<<" > 10/16/2017"<<endl;
}
void author()
{
	cerr <<endl<<"Xiaotao Yang & Gary Pavlis, Indiana University"<<endl<<endl;
}
void usage_message()
{
    version();
    cerr << "RFeditor dbin dbout [-d outdir][-tredit filename][-rm][-go][-continue]"<<endl
    	 << "[-fa fa_filename][-pf pffile][-laststa xx][-ss subset_condition][-v|V][-h|H]"<<endl;
    cerr << "** Use -h to print out detailed explanations on the options."<<endl;
    author();
}
void help()
{
	usage_message();
	cout<<"Options for running mode:"<<endl
    	<<"--review-mode|rm    :"<<endl
    	<<"    Edits will be dropped without saving to the database."<<endl
    	<<"--gui-off|-go       :"<<endl
    	<<"    In GUI-off mode, the program does editings without plotting the data."<<endl
    	<<"    The editing parameters are read in from the pf file. This can be also"<<endl
    	<<"    called auto-mode."<<endl
    	<<"--first-arrival|-fa fa_filename :"<<endl
    	<<"    In this mode, the first arrival information will be written out into"<<endl
    	<<"    fa_filename as a plain text file"<<endl
    	<<"-d outdir:"<<endl
    	<<"    Save edited data into outdir, default is RFDateEdited."<<endl;
    	cout<<endl
    	<<"Other options:"<<endl
    	<<"-tredit filename:"<<endl
    	<<"    Save copy of the editing summary to file: filename (plain text file). "<<endl
    	<<"    Otherwise only save to db table tredit."<<endl
    	<<"-continue"<<endl
    	<<"    Force to turn on continue mode when output db is not empty. "<<endl
    	<<"    PLEASE check outdb carefully to avoid duplicates."<<endl
    	<<"-laststa xx:"<<endl
    	<<"    Start from the station after station xx."<<endl
    	<<"-pf pffile:"<<endl
    	<<"    Use alternate pf instead of the default: RFeditor.pf."<<endl
    	<<"-v|V:"<<endl
    	<<"    Run program under verbose mode."<<endl
    	<<"-history:"<<endl
    	<<"    Modification history for current major version."<<endl
    	<<"-history2"<<endl
    	<<"    Full modification history for all versions including current and older."<<endl;
    exit(0);
}
void usage()
{
	usage_message();
    exit(-1);
}

bool SEISPP::SEISPP_verbose(false);
/* Simple algorithm sets an int metadata item (evidkey defined in 
   RFeditorEngine) by a simple counts starting at 1.  
   The algorith is very simple.  It assumes the data were already sorted
   by sta:time:chan.  The approach then is to work through the ensemble
   and whenever t0 of successive traces are within one sample dt group 
   them together.   
   */
void set_eventids(TimeSeriesEnsemble& d)
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

/* This function is used by both the TimeSeries and Three_Component
versions of dbsave below.  It builds a database row from a metadata
object, which is produced in both cases by  casting up to Metadata,
and pushing attributes out driven by the list, mdl, and the 
namespace map, am.  

Arguments:
	md = Metadata object containing attributes to be written to 
		database
	db = Antelope Dbptr.  It MUST point at a valid single row that
		is to contain the attributes to be copied there.  
	table = name of table these attributes are being written to
		(needed for consistency check).
	mdl = defines names to be extracted from metadata to
		write to database output
	am = AttributeMap object defining internal to external namespace
		mapping

The basic algorithm is:
	for each element of mdl
		if mdl-> am -> dbtable_name == table
			save
		else
			skip
	end foreach
*/
//Copied from readwrite.cc of SEISPP libs by Xiaotao Yang.
void save_metadata_for_object2(Metadata& md,
	Dbptr db,
		string table,
			MetadataList& mdl, 
				AttributeMap& am)
		throw(SeisppError)
{
	MetadataList::iterator mdli;
	map<string,AttributeProperties>::iterator ami,amie=am.attributes.end();
	map<string,AttributeProperties> aliasmap;
	string cval;
	const string base_message("dbsave->save_metadata_for_object2:  ");
//DEBUG
	for(mdli=mdl.begin();mdli!=mdl.end();++mdli)
	{
		double dval;
		long ival;
		string mdkey;
		if(am.is_alias((*mdli).tag))
		{
			try {
				aliasmap=am.aliases((*mdli).tag);
			} catch(SeisppError& serr){throw serr;};
			ami=aliasmap.find(table);
			if(ami==aliasmap.end())
			{
				dbmark(db);
				throw SeisppError(base_message
				 + string("Alias name=")
				 + (*mdli).tag
				 + string(" is not associated with table=")
				 + table
				 + string("\nVerify output specification against schema") );
			}
			mdkey=(*mdli).tag;   // in this case the alias is the key 
		}
		else
		{
			mdkey=(*mdli).tag;
			ami = am.attributes.find(mdkey);
			if(ami==amie) 
			{
				dbmark(db);
				throw SeisppError(
					string("Required attribute ")
					+(*mdli).tag
					+string(" cannot be mapped to output namespace"));
			}
			if( (ami->second.db_table_name) != table)
			{
				dbmark(db);
				throw SeisppError( 
					string("dbsave (database table mismatch): attribute ")
					+ ami->second.db_attribute_name
					+ string(" is tagged with table name ")
					+ ami->second.db_table_name
					+ string("expected to find ")
					+ table);
			}
			/* In this case the key we use the name from the Attribute map as the key */
			mdkey=ami->second.internal_name;
		}
		try {
			switch(ami->second.mdt)
			{
			case MDint:
				if(ami->second.is_key)
				{
					ival = dbnextid(db,
					  const_cast<char *>
					   (ami->second.db_attribute_name.c_str()) );
					if(ival<0)throw SeisppError(
					  	string("dbsave:  ")
						+ ami->second.db_attribute_name
						+ string(" is defined as integer key for table ")
						+ ami->second.db_table_name
						+ string(" but dbnextid failed") );
					
				}
				else
					ival = md.get_long(mdkey);
				dbputv(db,0,ami->second.db_attribute_name.c_str(),
					ival,NULL);
				// In this case we need to push this back to metadata
				// so it can be used downstream
				md.put(ami->second.db_attribute_name,ival);
				break;
			case MDreal:
				dval = md.get_double(mdkey);
				dbputv(db,0,ami->second.db_attribute_name.c_str(),
					dval,NULL);
				break;
			case MDstring:
				cval = md.get_string(mdkey);
				dbputv(db,0,ami->second.db_attribute_name.c_str(),
					cval.c_str(),NULL);
				break;
			case MDboolean:
				// treat booleans as ints for external representation
				// This isn't really necessary as Antelope
				// doesn't support boolean attributes
				if(md.get_bool(mdkey))
					ival = 1;
				else
					ival = 0;
				dbputv(db,0,ami->second.db_attribute_name.c_str(),
					ival,NULL);
				break;
				
			case MDinvalid:
				cerr << "dbsave: database attribute "
					<< ami->second.db_attribute_name
					<< " was marked as invalid\n"
					<< "Data for this attribute not saved"
					<< endl;
				break;
			
			default:
				cerr << "dbsave: database attribute "
					<< ami->second.db_attribute_name
					<< " has unknown data type\n"
					<< "Data for this attribute not saved"
					<< endl;
			}
	
		}
		catch (MetadataGetError& mderr)
		{
			mderr.log_error();
			throw SeisppError(
			    string("dbsave object failure from problem in metadata components"));
		}
	}
}

/*
// Save metadata only for Antelope database. Modified from dbsave()
// by removing waveform data saving ability.
// Xiaotao Yang 1/22/2015
*/
long dbsave_metadata(TimeSeries& ts, 
	Dbptr db,
		string table, 
			MetadataList& mdl, 
				AttributeMap& am)
		throw(SeisppError)
{
	int recnumber;
	string field_name;

	if(!ts.live) return(-1);  // return immediately if this is marked dead
	
	db = dblookup(db,0,const_cast<char *>(table.c_str()),0,0);
	recnumber = dbaddnull(db);
	if(recnumber==dbINVALID) throw SeisppError(string("dbsave_metadata:  dbaddnull failed on table "+table));
	db.record=recnumber;
	try {
		save_metadata_for_object2(dynamic_cast<Metadata&>(ts),
			db,table,mdl,am);
	} catch (SeisppError& serr)
	{
		dbmark(db);
		throw serr;
	}
	// Even if they were written in the above loop the contents 
	// of the object just override the metadata versions.  
	// This is safer than depending on the metadata
	double etime;
	etime = ts.endtime();
	dbputv(db,0,"time",ts.t0,
		"endtime",etime,
		"samprate",1.0/ts.dt,
		"nsamp",ts.ns,NULL);
		
	return(recnumber);
}

long dbsave_metadata(ThreeComponentSeismogram& tcs, 
	Dbptr db,
		string table, 
			MetadataList& mdl, 
				AttributeMap& am)
{
	long recnumber;
	string field_name;

	if(!tcs.live) return(-1);  // return immediately if this is marked dead
	if(table!="wfprocess")
		throw SeisppError(string("dbsave:  Using wrong dbsave function ")
			+string("for ThreeComponentSeismogram object.\n")
			+string("Can only save to wfprocess with this function.\n"));
	
	db = dblookup(db,0,const_cast<char *>(table.c_str()),0,0);
	recnumber = dbaddnull(db);
	if(recnumber==dbINVALID) 
		throw SeisppError(string("dbsave:  dbaddnull failed on table "+table));
	db.record=recnumber;
	try {
		/* post this in case the user tries to save it */
		string sdtype;
		if(IntelByteOrder())
			sdtype=string("c3");
		else
			sdtype=string("3c");
		tcs.put("datatype",sdtype);
		save_metadata_for_object2(dynamic_cast<Metadata&>(tcs),
			db,table,mdl,am);
		// Even if they were written in the above loop the contents 
		// of the object just override the metadata versions.  
		// This is safer than depending on the metadata
		double etime;
		etime = tcs.endtime();
		dbputv(db,0,"time",tcs.t0,
			"endtime",etime,
			"samprate",1.0/tcs.dt,
			"nsamp",tcs.ns,
			"datatype",sdtype.c_str(),NULL);
		return(recnumber);
	}
	catch (SeisppError& serr)
	{
		// delete this database row if we had an error
		dbmark(db);
		throw serr;
	}
}
//WARNING: if read in as 3c data from wfprocess and save_decon_table is on
//, do not use this routine to save to db!
// this warning is applicable since decon saves three components seperately
// while wfprocess saves 3c data as one row. In this case, resulting db would 
//have duplicates (all same as R chan, 3 times of real R chan lines) in decon table.
int save_to_db(TimeSeriesEnsemble& r, TimeSeriesEnsemble& t,
				TimeSeriesEnsemble& z,MetadataList& mdl, 
				AttributeMap& am, DatascopeHandle& dbh, 
        		string outdir,string outdfile_base,
        		bool save_metadata_only,bool save_vertical_channel, 
        		bool save_decon_table, string rchan, string tchan,
        		string zchan, string outtable)
        		//decon table is saved only when using wfprocess table as output.
{
    try {
        vector<TimeSeries>::iterator dptr;
        int nlive(0);
        string table=outtable;
        string outdfile;
		string dirkey;
		string dfilekey;
		DatascopeHandle dbsclink(dbh);
		DatascopeHandle dbevlink(dbh);
		DatascopeHandle dbdecon(dbh);
		dbsclink.lookup("sclink");
		dbevlink.lookup("evlink");
		if(save_decon_table) dbdecon.lookup("decon");
        //save radial.		 
        for(dptr=r.member.begin();dptr!=r.member.end();++dptr)
        {
            string sta=dptr->get_string("sta");
            if(dptr->live)
            {       	
				long rec;  
				outdfile=outdfile_base+"_"+dptr->get_string("sta")+".R";
				if(outtable=="wfprocess")
				{
					dirkey="dir";
					dfilekey="dfile";
					dptr->put(dirkey,outdir);
					dptr->put(dfilekey,outdfile);
					if(save_metadata_only)
						rec=dbsave_metadata(*dptr,dbh.db,table,mdl,am);
					else
						rec=dbsave(*dptr,dbh.db,table,mdl,am);
					rec++;
					dbevlink.append();
					dbevlink.put("evid",dptr->get_int("evid"));
					dbevlink.put("pwfid",rec);
					dbsclink.append();
					dbsclink.put("sta",dptr->get_string("sta"));
					dbsclink.put("chan",rchan);
					dbsclink.put("pwfid",rec);
					if(save_decon_table) 
					{
						dbdecon.append();
						dbdecon.put("pwfid",rec);
						dbdecon.put("sta",dptr->get_string("sta"));
						dbdecon.put("chan",rchan);
						dbdecon.put("niteration",dptr->get_int("decon.niteration"));
						dbdecon.put("nspike",dptr->get_int("decon.nspike"));
						dbdecon.put("epsilon",dptr->get_double("decon.epsilon"));
						dbdecon.put("peakamp",dptr->get_double("decon.peakamp"));
						dbdecon.put("averamp",dptr->get_double("decon.averamp"));
						dbdecon.put("rawsnr",dptr->get_double("decon.rawsnr"));
					}
				}
				else
				{
					if(save_metadata_only)
						dbsave_metadata(*dptr,dbh.db,table,mdl,am);
					else
					{
						dirkey="dir";
						dfilekey="dfile";
						dptr->put(dirkey,outdir);
						dptr->put(dfilekey,outdfile);
						rec=dbsave(*dptr,dbh.db,table,mdl,am);
					}
				}
                ++nlive;
            }
        }
		//cerr << "Number of radial traces saved = "<<nlive<<endl;
		//save transverse.
		nlive=0;
        for(dptr=t.member.begin();dptr!=t.member.end();++dptr)
        {
        	string sta=dptr->get_string("sta");
            if(dptr->live)
            {
                long rec;  
				outdfile=outdfile_base+"_"+dptr->get_string("sta")+".T";
				if(outtable=="wfprocess")
				{
					dirkey="dir";
					dfilekey="dfile";
					dptr->put(dirkey,outdir);
					dptr->put(dfilekey,outdfile);
					if(save_metadata_only)
						rec=dbsave_metadata(*dptr,dbh.db,table,mdl,am);
					else
						rec=dbsave(*dptr,dbh.db,table,mdl,am);
					rec++;
					dbevlink.append();
					dbevlink.put("evid",dptr->get_int("evid"));
					dbevlink.put("pwfid",rec);
					dbsclink.append();
					dbsclink.put("sta",dptr->get_string("sta"));
					dbsclink.put("chan",tchan);
					dbsclink.put("pwfid",rec);
					if(save_decon_table) 
					{
						dbdecon.append();
						dbdecon.put("pwfid",rec);
						dbdecon.put("sta",dptr->get_string("sta"));
						dbdecon.put("chan",tchan);
						dbdecon.put("niteration",dptr->get_int("decon.niteration"));
						dbdecon.put("nspike",dptr->get_int("decon.nspike"));
						dbdecon.put("epsilon",dptr->get_double("decon.epsilon"));
						dbdecon.put("peakamp",dptr->get_double("decon.peakamp"));
						dbdecon.put("averamp",dptr->get_double("decon.averamp"));
						dbdecon.put("rawsnr",dptr->get_double("decon.rawsnr"));
					}
				}
				else
				{
					if(save_metadata_only)
						dbsave_metadata(*dptr,dbh.db,table,mdl,am);
					else
					{
						dirkey="dir";
						dfilekey="dfile";
						dptr->put(dirkey,outdir);
						dptr->put(dfilekey,outdfile);
						rec=dbsave(*dptr,dbh.db,table,mdl,am);
					}
				}
                ++nlive;
            }
        }
        //cerr << "Number of transverse traces saved = "<<nlive<<endl;
        //save vertical is turned on.
        if(save_vertical_channel)
        {
        	nlive=0;
			for(dptr=z.member.begin();dptr!=z.member.end();++dptr)
			{
				string sta=dptr->get_string("sta");
				if(dptr->live)
				{
					long rec;  
					outdfile=outdfile_base+"_"+dptr->get_string("sta")+".Z";
					if(outtable=="wfprocess")
					{
						dirkey="dir";
						dfilekey="dfile";
						dptr->put(dirkey,outdir);
						dptr->put(dfilekey,outdfile);
						if(save_metadata_only)
							rec=dbsave_metadata(*dptr,dbh.db,table,mdl,am);
						else
							rec=dbsave(*dptr,dbh.db,table,mdl,am);
						rec++;
						dbevlink.append();
						dbevlink.put("evid",dptr->get_int("evid"));
						dbevlink.put("pwfid",rec);
						dbsclink.append();
						dbsclink.put("sta",dptr->get_string("sta"));
						dbsclink.put("chan",zchan);
						dbsclink.put("pwfid",rec);
						if(save_decon_table) 
						{
							dbdecon.append();
							dbdecon.put("pwfid",rec);
							dbdecon.put("sta",dptr->get_string("sta"));
							dbdecon.put("chan",zchan);
							dbdecon.put("niteration",dptr->get_int("decon.niteration"));
							dbdecon.put("nspike",dptr->get_int("decon.nspike"));
							dbdecon.put("epsilon",dptr->get_double("decon.epsilon"));
							dbdecon.put("peakamp",dptr->get_double("decon.peakamp"));
							dbdecon.put("averamp",dptr->get_double("decon.averamp"));
							dbdecon.put("rawsnr",dptr->get_double("decon.rawsnr"));
						}
					}
					else
					{
						if(save_metadata_only)
							dbsave_metadata(*dptr,dbh.db,table,mdl,am);
						else
						{
							dirkey="dir";
							dfilekey="dfile";
							dptr->put(dirkey,outdir);
							dptr->put(dfilekey,outdfile);
							rec=dbsave(*dptr,dbh.db,table,mdl,am);
						}
					}
					nlive++;
				}
			}
			//cerr << "Number of vertical traces saved = "<<nlive<<endl;
        }
        return(nlive);
    }catch(...){throw;};
}
//save 3c data to db.
int save_to_db(ThreeComponentEnsemble& tce,MetadataList& mdl, 
				AttributeMap& am, DatascopeHandle& dbh, 
        		string outdir,string outdfile_base,
        		bool save_metadata_only,bool save_decon_table,
        		string rchan="R", string tchan="T",string zchan="Z",
        		string outtable="wfprocess")
{
	//const string outtable("wfprocess");

	string outdfile;
    DatascopeHandle dbsclink(dbh);
    DatascopeHandle dbevlink(dbh);
    DatascopeHandle dbdecon(dbh);
	int nsaved(0);
	vector<string> output_channels;
	output_channels.clear();				
	output_channels.push_back(tchan);
	output_channels.push_back(rchan);
	output_channels.push_back(zchan);
	
	dbsclink.lookup("sclink");
	dbevlink.lookup("evlink");
	if(save_decon_table) dbdecon.lookup("decon");
	vector<ThreeComponentSeismogram>::iterator d;
	int i;
	for(d=tce.member.begin(),i=0;d!=tce.member.end();++d,++i)
	{
		if(d->live)
		{
			try {
				d->put("dir",outdir);
				//debug
				//cout<<"db: save to dir: "<<outdir<<endl;
				//cout<<"db: dir in db: "<<d->get_string("dir")<<endl;
				if(outtable=="wfprocess")
				{
					outdfile=outdfile_base+"_"+d->get_string("sta")+".3C";
					d->put("dfile",outdfile);
					//debug
					//cout<<"db: save to dfile: "<<outdfile<<endl;
					//cout<<"db: outdfile in db: "<<d->get_string("dfile")<<endl;
					//save db.
					long rnum;
					if(save_metadata_only)
						rnum=dbsave_metadata(*d,dbh.db,outtable,mdl,am);
					else
						rnum=dbsave(*d,dbh.db,outtable,mdl,am);
					rnum++;
					dbevlink.append();
					dbevlink.put("evid",d->get_int("evid"));
					dbevlink.put("pwfid",rnum);
					dbsclink.append();
					dbsclink.put("sta",d->get_string("sta"));
					dbsclink.put("chan","3C");
					dbsclink.put("pwfid",rnum);
					if(save_decon_table) 
					{
						dbdecon.append();
						dbdecon.put("pwfid",rnum);
						dbdecon.put("sta",d->get_string("sta"));
						dbdecon.put("chan",d->get_string("decon.chan"));
						dbdecon.put("niteration",d->get_int("decon.niteration"));
						dbdecon.put("nspike",d->get_int("decon.nspike"));
						dbdecon.put("epsilon",d->get_double("decon.epsilon"));
						dbdecon.put("peakamp",d->get_double("decon.peakamp"));
						dbdecon.put("averamp",d->get_double("decon.averamp"));
						dbdecon.put("rawsnr",d->get_double("decon.rawsnr"));
					}
					nsaved++;
				}
				else if(outtable=="wfdisc")
				{
					outdfile=string(outdfile_base)+"_"+d->get_string("sta")+".w";
					d->put("dfile",outdfile);
					//save db.
					if(!save_metadata_only)
						dbsave(*d,dbh.db,outtable,mdl,am,
								output_channels,true);
					else
					{	
						cerr<<"ERROR in save_to_db(): can't save 3c ensemble to wfdisc table when"<<endl
							<<"      save_metadata_only is set to true!"<<endl;
						exit(-1);
					}
					if(save_decon_table) 
					{
						cerr<<"ERROR in save_to_db(): can't save 3c ensemble to decon table when"<<endl
							<<"      saving to wfdisc table!"<<endl;
						exit(-1);
					}
					nsaved++;
				}
				else
				{
					cerr<<"ERROR in save_to_db(): wrong outtable, could be either wfprocess or wfdisc"<<endl;
					exit(-1);
				}
				
			} catch (SeisppError& serr) {
				string sta=d->get_string("sta");
				cerr << "Error saving station "<<sta<<endl;
				serr.log_error();
			}
		}
	}
	
	return nsaved;
}
bool check_continue_mode(bool set_continue_mode_by_default,string laststation)
{
	try
	{
		bool turn_on_continue_mode(false);
		string ques;
		if(set_continue_mode_by_default) 
		{
			turn_on_continue_mode=true;
			return(turn_on_continue_mode);
		}
		else
		{
			cout<<"!!! Output table is not empty. Last station you worked on = [ "
				<<laststation<<" ]."<<endl
				<<"> Continue working on next station? (y/r/n) "<<endl
				<<"	y: continue;"<<endl
				<<"	r: re-do from the beginning. Caution for duplicate rows! "<<endl
				<<"		Suggest choose this only under review mode;"<<endl
				<<"	n: quit the program."<<endl<<"> Your choice: ";
			cin>>ques;
			if(ques == "y" || ques=="Y") turn_on_continue_mode=true;
			else if(ques == "r" || ques == "R") 
			{
				turn_on_continue_mode=false;
				cout<<"!!! CAUTION: if you are not under review mode, "<<endl
					<<"output wfdisc table will have duplicate rows!"<<endl;
				return(turn_on_continue_mode);
			}
			else
			{
				cout<<"Quited. "<<endl<<"Please clean up the output db "
					<<"and the data directory."<<endl;
				exit(-1);
			}					
		}
		return(turn_on_continue_mode);
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

void append_plot_window_params(Metadata& md)
{
	// ******** following are plotting parameters. change ONLY IF necessary. !!!
	try
	{
		//cerr<<"test"<<endl;
		if(!md.is_attribute_set((char *)"SUVariableArea_grey_value")) 
			md.put("SUVariableArea_grey_value",1);
		if(!md.is_attribute_set((char *)"VariableArea"))
			md.put("VariableArea",true);
		if(!md.is_attribute_set((char *)"WiggleTrace"))
			md.put("WiggleTrace",true);
		if(!md.is_attribute_set((char *)"blabel")) md.put("blabel",(char *)"Data");
		if(!md.is_attribute_set((char *)"blabel2")) md.put("blabel2",(char *)"Data");
		if(!md.is_attribute_set((char *)"clip_data")) md.put("clip_data",true);
		if(!md.is_attribute_set((char *)"clip_percent")) md.put("clip_percent",99.5);
		if(!md.is_attribute_set((char *)"clip_wiggle_traces")) 
			md.put("clip_wiggle_traces",false);
		if(!md.is_attribute_set((char *)"d1num")) md.put("d1num",0.0);
		if(!md.is_attribute_set((char *)"d2num")) md.put("d2num",0.0);
		if(!md.is_attribute_set((char *)"default_curve_color")) 
			md.put("default_curve_color",(char *)"black");
		if(!md.is_attribute_set((char *)"editing_mode")) 
			md.put("editing_mode",(char *)"single_trace");
		if(!md.is_attribute_set((char *)"f1num")) md.put("f1num",0.0);
		if(!md.is_attribute_set((char *)"f2num")) md.put("f2num",0.0);
		if(!md.is_attribute_set((char *)"first_trace_offset")) 
			md.put("first_trace_offset",0.0);
		if(!md.is_attribute_set((char *)"grid1")) md.put("grid1",1);
		if(!md.is_attribute_set((char *)"grid2")) md.put("grid2",1);
		if(!md.is_attribute_set((char *)"gridcolor")) md.put("gridcolor",(char *)"blue");
		if(!md.is_attribute_set((char *)"hbox")) md.put("hbox",5000);
		if(!md.is_attribute_set((char *)"interpolate")) md.put("interpolate",true);
		if(!md.is_attribute_set((char *)"label1")) md.put("label1",(char *)"time");
		if(!md.is_attribute_set((char *)"label2")) md.put("label2",(char *)"index");
		if(!md.is_attribute_set((char *)"labelcolor")) md.put("labelcolor",(char *)"blue");
		if(!md.is_attribute_set((char *)"labelfont")) md.put("labelfont",(char *)"Rom14");
		if(!md.is_attribute_set((char *)"labelsize")) md.put("labelsize",18.0);
		if(!md.is_attribute_set((char *)"n1tic")) md.put("n1tic",5);
		if(!md.is_attribute_set((char *)"n2tic")) md.put("n2tic",1);
		if(!md.is_attribute_set((char *)"plot_file_name")) 
			md.put("plot_file_name",(char *)"SeismicPlot.ps");
		if(!md.is_attribute_set((char *)"style")) md.put("style",(char *)"normal");
		if(!md.is_attribute_set((char *)"time_axis_grid_type")) 
			md.put("time_axis_grid_type",(char *)"solid");
		if(!md.is_attribute_set((char *)"time_scaling")) md.put("time_scaling",(char *)"auto");
		if(!md.is_attribute_set((char *)"title")) md.put("title",(char *)"Receiver");
		if(!md.is_attribute_set((char *)"titlecolor")) md.put("titlecolor",(char *)"red");
		if(!md.is_attribute_set((char *)"titlefont")) md.put("titlefont",(char *)"Rom22");
		if(!md.is_attribute_set((char *)"titlesize")) md.put("titlesize",36.0);
		if(!md.is_attribute_set((char *)"trace_axis_attribute")) 
			md.put("trace_axis_attribute",(char *)"assoc.delta");
		if(!md.is_attribute_set((char *)"trace_axis_grid_type")) 
			md.put("trace_axis_grid_type",(char *)"none");
		if(!md.is_attribute_set((char *)"trace_axis_scaling")) 
			md.put("trace_axis_scaling",(char *)"auto");
		if(!md.is_attribute_set((char *)"trace_spacing")) md.put("trace_spacing",1.0);
		if(!md.is_attribute_set((char *)"trim_gap_edges")) md.put("trim_gap_edges",true);
		if(!md.is_attribute_set((char *)"use_variable_trace_spacing")) 
			md.put("use_variable_trace_spacing",false);
		if(!md.is_attribute_set((char *)"verbose")) md.put("verbose",true);
		if(!md.is_attribute_set((char *)"wbox")) md.put("wbox",950);
		if(!md.is_attribute_set((char *)"windowtitle")) md.put("windowtitle",(char *)"RFeditor");
		if(!md.is_attribute_set((char *)"x1beg")) md.put("x1beg",0.0);
		if(!md.is_attribute_set((char *)"x1end")) md.put("x1end",120.0);
		if(!md.is_attribute_set((char *)"x2beg")) md.put("x2beg",0.0);
		if(!md.is_attribute_set((char *)"x2end")) md.put("x2end",24.0);
		if(!md.is_attribute_set((char *)"xbox")) md.put("xbox",50);
		if(!md.is_attribute_set((char *)"xcur")) md.put("xcur",1.0);
		if(!md.is_attribute_set((char *)"ybox")) md.put("ybox",50);
		if(!md.is_attribute_set((char *)"beam_hbox")) md.put("beam_hbox",250);
		if(!md.is_attribute_set((char *)"beam_clip_data")) md.put("beam_clip_data",false);
		if(!md.is_attribute_set((char *)"beam_xcur")) md.put("beam_xcur",1.0);
		if(!md.is_attribute_set((char *)"beam_trace_spacing")) 
			md.put("beam_trace_spacing",1.0);
		if(!md.is_attribute_set((char *)"beam_trace_axis_scaling")) 
			md.put("beam_trace_axis_scaling",(char *)"auto");
	}catch(...) {throw;};
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
    const string logfilename("RFeditor.log");
    switch(argc)
    {
    	case 1:
    		usage();
    		break;
    	case 2:
    		string sarg(argv[1]);
    		if(sarg=="-h" || sarg=="-H")
    			help();
    		else if(sarg=="-history")
        	{	history_current();
        		exit(0);}
        	else if(sarg=="-history2")
        	{	history_old();
        		history_current();
        		exit(0);}
    		else 
    			usage();
    		break;
    }
    
    string dbin_name(argv[1]);
    string dbout_name(argv[2]);
    string outdir("RFDataEdited");
    string pfname("RFeditor");
    string FA_filename("-");
    bool save_edit_summary_to_file(false);
    string editsummaryfname("TraceEditSummary.txt");
    //string stacktype("r");  // set default stacktype as RobustSNR. Xiaotao Yang
    string ques;
    bool apply_subset(false);
    bool review_mode(false);
    bool set_continue_mode_by_default(false);
    bool turn_on_continue_mode(true);
    bool datatype3c(false);
    bool GUIoff(false);
    //program will write FirstArrival information into 
    bool get_FA(false);
    bool pre_edit_FA(false),post_edit_FA(false); 
    	//unless review-mode and GUIoff are on, post-edit-FA will be computed.
    string subset_condition("");
    string filterspec;
	/* quotes needed around subsets because sometimes
	station names start with numbers that confuse datascope */
	const string quote("\"");
    int i;
    for(i=3;i<argc;++i)
    {
        string sarg(argv[i]);
        // read in for stack type
        if(sarg=="-rm" || sarg=="--review-mode")
        {
        	review_mode=true;
        	/*remind user of the review_mode. Xiaotao Yang*/
			if(review_mode)
			{
				cout<<"Warning: review mode is turned on! All edits will be dropped!"<<endl
					<<"> Continue? (y/n) ";
				cin>>ques;
				if(ques != "y" && ques!="Y")
				{
					cout<<"Quited."<<endl;
					exit(-1);
				}
			}
		}
        else if(sarg=="--gui-off" || sarg=="-go")
        	GUIoff=true;
        else if(sarg=="--first-arrival " || sarg=="-fa")
        {
        	get_FA=true,pre_edit_FA=false,post_edit_FA=true;
        	++i;
            if(i>=argc) usage();
            FA_filename=string(argv[i]);
        }
        else if(sarg=="-continue")
        {
        	set_continue_mode_by_default=true; 
        		/*
        		//if this is true, the program will continue working on next station 
        		//when output table is not empty. otherwise, it will ask the user to 
        		//choose either continue on the next station or quit.
        		// Xiaotao Yang
        		*/
        }
        else if(sarg=="-pf")
        {
            ++i;
            if(i>=argc) usage();
            pfname=string(argv[i]);
        }
        else if(sarg=="-d")
        {
        	++i;
            if(i>=argc) usage();
            outdir=string(argv[i]);
        }
        else if(sarg=="-tredit")
        {
        	++i;
            if(i>=argc) usage();
            save_edit_summary_to_file=true;
            editsummaryfname=string(argv[i]);
        }
        else if(sarg=="-ss")
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
        else if(sarg=="-history")
        {	history_current();
        	exit(0);}
        else if(sarg=="-history2")
        {	history_old();
        	history_current();
        	exit(0);}
        else
            usage();
    }
    
    //Review Mode must be off in GUIoff mode.
    if(review_mode && GUIoff && !get_FA)
    {
    	cerr<<errorbase<<"Can't turn on Review-Mode while under GUIoff mode "
    		<<"unless [-fa] option is used."<<endl;
    	usage();
    }
    else if(review_mode && GUIoff && get_FA && SEISPP_verbose)
    {
    	cout<<"!!!Caution: review-mode, GUIoff-mode and first-arrival mode are all turned on."<<endl
    		<<"This will result in ONLY writing out first arrival information without doing editing!"<<endl;
    	pre_edit_FA=true, post_edit_FA=false;
    }
    /*
    if(post_edit_FA) 
    	cout<<"!!!Caution: post-edit FA will ONLY be saved when "
    		<<"editing on radial component! Ignored otherwise!"<<endl;
    */
    if(SEISPP_verbose && get_FA)
    	cout<<"Get-FirstArrival mode is on. FA information will be written into [ "<<FA_filename<<" ]."<<endl;
    /*Standard way to open Antelope pf file.*/
    Pf *pf;
    if(pfread(const_cast<char *>(pfname.c_str()),&pf))
    {
        cerr << "pfread failed on pf file named "<<pfname<<endl;
        exit(-1);
    }
    try {
			/****************************************************************
			*****************************************************************
			*******<<<<<<<<<<<< READ IN CONTROL PARAMETERS >>>>>>>>>>********
			*****************************************************************
			*****************************************************************
			*/
        Metadata control(pf);
        Metadata trace_edit_params;
        if(GUIoff)
        	trace_edit_params=Metadata(pf,string("auto_edit_parameters"));
        else
        {
        	trace_edit_params=Metadata(pf,string("gui_edit_parameters"));
        	append_plot_window_params(trace_edit_params);
        }
        //Parameters needed in detecting FA. Use defaults when not set in pf.
        double FA_detect_length(1.0),FA_sensitivity(10e-4),
        		FA_search_TW_start(-5.0), FA_search_TW_end(5.0);
        if(control.is_attribute_set((char *)"FA_detect_length")) 
        		FA_detect_length=control.get_double("FA_detect_length");
		if(control.is_attribute_set((char *)"FA_sensitivity"))
				FA_sensitivity=control.get_double("FA_sensitivity");
		//Could only be: GAUSSIAN, SPIKE, RICKER (CASE SENSITIVE)
		string data_shaping_wavelet_type("GAUSSIAN");
		if(control.is_attribute_set((char *)"data_shaping_wavelet_type"))
				data_shaping_wavelet_type=control.get_string("data_shaping_wavelet_type");
		if(control.is_attribute_set((char *)"FA_search_TW_start"))
				FA_search_TW_start=control.get_double("FA_search_TW_start");
		if(control.is_attribute_set((char *)"FA_search_TW_end"))
				FA_search_TW_end=control.get_double("FA_search_TW_end");
        
        //put the above FA detection parameters into trace_edit_param metadata object.
        //use *.is_attribute_set method because old pf read in FA parameters for GUI and GUIoff seperately.
        if(!trace_edit_params.is_attribute_set((char *)"FA_detect_length"))
        		trace_edit_params.put("FA_detect_length",FA_detect_length);
        if(!trace_edit_params.is_attribute_set((char *)"FA_sensitivity"))
        		trace_edit_params.put("FA_sensitivity",FA_sensitivity);
        if(!trace_edit_params.is_attribute_set((char *)"data_shaping_wavelet_type"))
        		trace_edit_params.put("data_shaping_wavelet_type",data_shaping_wavelet_type);
        if(!trace_edit_params.is_attribute_set((char *)"FA_search_TW_start"))
        		trace_edit_params.put("FA_search_TW_start",FA_search_TW_start);
        if(!trace_edit_params.is_attribute_set((char *)"FA_search_TW_end"))
        		trace_edit_params.put("FA_search_TW_end",FA_search_TW_end);
        
        ///*Read in parameters from the pf*/
        MetadataList mdlens;
        if(control.is_attribute_set(MDL_ENSEMBLE))
        {
        	mdlens=pfget_mdlist(pf,control.get_string(MDL_ENSEMBLE));
        }
        else
        	mdlens=generate_mdlist("ensemble"); //=pfget_mdlist(pf,"ensemble_mdl");
        
        MetadataList mdl;
        MetadataList mdlout, mdlout_wfd, mdlout_wfp;
        bool use_arrival_data=control.get_bool("use_arrival_data");
        if(SEISPP_verbose && !use_arrival_data)
        	cout<<"Warning: use_arrival_data is set to false. "
        		<<"Use waveform start time in a_to_r conversion."<<endl;
        //read in preference for use of netmag table
        bool use_netmag_table=control.get_bool("use_netmag_table");
        trace_edit_params.put("use_netmag_table",use_netmag_table);
		bool use_wfdisc_in=control.get_bool("use_wfdisc_in");
        if(use_netmag_table && !use_arrival_data && use_wfdisc_in)
        {
        	cerr<<"WARNING: bad combination of parameters. When using wfdisc as input"<<endl
        	    <<" and using netmag table, then use_arrival_data MUST be true!"<<endl
        	    <<" The program forces use_arrival_data to true by default! Be sure that"<<endl
        	    <<" arrival, assoc, event, origin, netmag tables are all available."<<endl;
        	use_arrival_data=true;
        }
        /*
        //meaningless to read in this attribute.
        bool ThreeComponentMode=control.get_bool("ThreeComponentMode");
        if(ThreeComponentMode)
        {
        	cerr<<"Warning: RFeditor plotting requires 'ThreeComponentMode' be false!"<<endl
        		<<"Set to false by default."<<endl;
        	//this is not a fatal error. just automatically set to false.
        }
        */
        trace_edit_params.put("ThreeComponentMode",false); 
        //this is used when converting from a time to r time.
        // this value is the FA time set to the trace for display. The
        //default FA time will be 0.
        double FA_reference_time=control.get_double("FA_reference_time");

        bool use_decon_in_editing(false);
        bool save_3C_data(false);
        bool save_decon_table(false);
        //ignored if reading from wfdisc (use_wfdisc_in is true)
        bool save_wfdisc_table(false);
        //ignored if reading from wfprocess (use_wfdisc_in is false)
        bool save_wfprocess_table(false);
        if(use_wfdisc_in)
        {
        	//mdl=pfget_mdlist(pf,"trace_mdl_wfdisc");
        	save_wfprocess_table=control.get_bool("save_wfprocess_table");
        	control.put("use_decon_in_editing",false);
        	if(SEISPP_verbose)
        		cout<<"Warning: force use_decon_in_editing "
        			<<"to be false when using wfdisc input."<<endl;
        	if(control.is_attribute_set(MDL_WFDISCIN))
			{
				mdl=pfget_mdlist(pf,control.get_string(MDL_WFDISCIN));
			}
			else
        		mdl=generate_mdlist("wfdiscin",use_arrival_data, use_netmag_table);
        	
        	if(control.is_attribute_set(MDL_WFDISCOUT))
			{
				mdlout=pfget_mdlist(pf,control.get_string(MDL_WFDISCOUT));
			}
			else
        		mdlout=generate_mdlist("wfdiscout");
        	if(save_wfprocess_table)
        	{
				if(control.is_attribute_set(MDL_WFPROCESSOUT))
				{
					mdlout_wfp=pfget_mdlist(pf,control.get_string(MDL_WFPROCESSOUT));
				}
				else
					mdlout_wfp=generate_mdlist("wfprocessout");
			}
        }
        else
        {
        	save_wfdisc_table=control.get_bool("save_wfdisc_table");
        	save_3C_data=control.get_bool("save_3C_data");
        	use_decon_in_editing=control.get_bool("use_decon_in_editing");
        	if(control.is_attribute_set(MDL_WFPROCESSIN))
			{
				mdl=pfget_mdlist(pf,control.get_string(MDL_WFPROCESSIN));
			}
			else
        		mdl=generate_mdlist("wfprocessin",use_arrival_data, 
        			use_netmag_table,use_decon_in_editing);
        	if(use_decon_in_editing)
        	{
        		//mdl=pfget_mdlist(pf,"trace_mdl_wfprocess_decon");
        		save_decon_table=control.get_bool("save_decon_table");
        	}
        	if(control.is_attribute_set(MDL_WFPROCESSOUT))
			{
				mdlout=pfget_mdlist(pf,control.get_string(MDL_WFPROCESSOUT));
			}
			else
        		mdlout=generate_mdlist("wfprocessout");
        	//mdlout=pfget_mdlist(pf,"output_mdl_wfprocess");
        	if(save_wfdisc_table)
        	{
        		if(control.is_attribute_set(MDL_WFDISCOUT))
				{
					mdlout_wfd=pfget_mdlist(pf,control.get_string(MDL_WFDISCOUT));
				}
				else
        			mdlout_wfd=generate_mdlist("wfdiscout");
        	}
        }
        
        //put these two parameters into the metadata for trace-editing.
        trace_edit_params.put("use_decon_in_editing",use_decon_in_editing);
        if(use_decon_in_editing) append_decon_attribute_keys(trace_edit_params);
        trace_edit_params.put("FA_reference_time",FA_reference_time);

        string rchan=control.get_string("radial_channel_key");
        string tchan=control.get_string("transverse_channel_key");
        string zchan;
        bool no_vertical_data(false);
        try{
        zchan=control.get_string("vertical_channel_key");
        }catch(SeisppError& serr)
        {
        	serr.what();
			if(!control.is_attribute_set((char *)"no_vertical_data"))
			{cerr<<"*** ERROR: can't get vertical_channel_key! "
				 <<"Set no_vertical_data to true if no vertical data provided!"<<endl;
				exit(-1);
			}
			else
				no_vertical_data=control.get_bool("no_vertical_data");
			if(!no_vertical_data)
			{	cerr<<"*** ERROR: wrong parameter comination. Must privide vertical_channel_key"
					<<" if vertical data is provided!"<<endl;
				exit(-1);
			}
        }   
        
        //read in which channel to be edited.
        bool edit_on_radial(true), edit_on_transverse(false), edit_on_vertical(false);
        string edit_on_channel, edit_chan_code;
        int edit_component(1); //component in 3c data in wfprocess table.
        try{
        edit_on_channel=control.get_string("edit_on_channel");
        }catch(SeisppError& serr)
        {
        	cerr << "*** Warning: edit_on_channel is not defined in pf. Use radial by default!"<<endl;
        	serr.what();
        	edit_on_channel="radial";
        }
        if(edit_on_channel=="radial") 
        	{edit_on_radial=true;edit_chan_code=rchan;edit_component=1;}
        else if(edit_on_channel=="transverse") 
        	{edit_on_transverse=true;edit_chan_code=tchan;edit_component=0;}
        else if(edit_on_channel=="vertical") 
        {
        	if(no_vertical_data)
			{	
				if(use_decon_in_editing || use_wfdisc_in)
				{	
					cerr<<"*** ERROR: Please make sure you have vertical data and provide "
						<<"vertical_channel_key if you want to edit vertical data!"<<endl;
					exit(-1);
				}
			}
        	edit_on_vertical=true;
        	edit_chan_code=zchan;
        	edit_component=2;
        }
        else
        {
        	cerr << "*** ERROR: wrong edit_on_channel name! Could only be radial, "
        		 << "transverse, or vertical. Please check pf!" <<endl;
        	exit(-1);
        }
        
        int minrfcutoff=control.get_int("minimum_number_receiver_functions");
        //set_continue_mode_by_default=control.get_bool("set_continue_mode_by_default"); 
        		/*
        		//if this is true, the program will continue working on next station 
        		//when output table is not empty. otherwise, it will ask the user to 
        		//choose either continue on the next station or quit.
        		// Xiaotao Yang
        		*/
        bool apply_prefilter=control.get_bool("apply_prefilter");
        string wavelet_type=control.get_string("wavelet_type");
        bool save_vertical_channel(false);
        if(!save_3C_data && !no_vertical_data) 
        	save_vertical_channel=control.get_bool("save_vertical_channel");
        bool save_metadata_only=control.get_bool("save_metadata_only");
        bool save_filtered_data=control.get_bool("save_filtered_data");
        
        if(save_metadata_only && save_filtered_data)
        {
        	cerr<<"ERROR: Conflicting parameters!"<<endl
        		<<"Message: [save_metadata_only] & [save_filtered_data] cannot be both true!"<<endl;
        	exit(-1);
        }
        //string outdir=control.get_string("output_dir");
        string outdfile_base=control.get_string("output_dfile_base");
        
        /* Open a log file in append mode */
        ofstream logfile;
        
        logfile.open(logfilename.c_str(),ios::app);
        if(logfile.fail())
        {
            cerr << "Cannot open log file [ "<<logfilename<<" ] in append mode"
                <<endl << "Check permissions and try again"<<endl;
            exit(-1);
        }
		if(review_mode)
		{
			logfile << "RFeditor: review mode is turned on! All edits will be dropped!"<<endl;
		}
        cout << "Starting RFeditor on database [ "<<dbin_name<<" ]."<<endl;
        logfile << "RFeditor run on [ " << dbin_name << " ] at time "
            << strtime(now()) <<endl;
        
        if(!review_mode)
        {
        	cout << "Outputing edited db to database [ "<<dbout_name<<" ]."<<endl;
			logfile << "Output to [ " << dbout_name <<" ]."<<endl;
			//debug
    		cout<<"Will save data to dir [ "<<outdir<<" ]"<<endl;
    		logfile<<"Will save data to dir [ "<<outdir<<" ]"<<endl;
		}
		
		
				/****************************************************************
				*****************************************************************
				*********<<<<<<<<<<<< CHECK OUTPUT DATABASE >>>>>>>>>>***********
				*****************************************************************
				*****************************************************************
				*/

		DatascopeHandle dbout(dbout_name,false);
		DatascopeHandle dbhwfdisc(dbout), dbhwfprocess(dbout);
		if(!review_mode)
        {
			/* First check that the output wfdisc is empty */
			if(use_wfdisc_in)
			{
				dbout.lookup("wfdisc");
				if(save_wfprocess_table) dbhwfprocess.lookup("wfprocess");
			}
			else
			{
				dbout.lookup("wfprocess");
				if(save_wfdisc_table) dbhwfdisc.lookup("wfdisc");
			}
		
			logfile << "Writing results to "<<dbout_name<<endl
				<<"Number of existing rows in the output database table is "
				<<dbout.number_tuples()<<endl;
			
			/*if the output db table is not empty, turn on continue mode or quit program
			// and manually clean the output db.
			// Xiaotao Yang 1/23/2015
			*/
			if(dbout.number_tuples()>0)
			{
				if(!use_wfdisc_in) dbout.natural_join("sclink");
				//find last station in wfdisc table
				list<string> sortkey0;
				sortkey0.push_back("sta");
				dbout.sort(sortkey0);
				//dbout.rewind();
				//dbout=dbout+dbout.number_tuples();
				dbout.db.record=dbout.number_tuples()-1;
				string laststation;
				laststation=dbout.get_string("sta");
				//call function to check status of continue_mode.
				turn_on_continue_mode=check_continue_mode(set_continue_mode_by_default,laststation);
					
				if(turn_on_continue_mode)
				{
					if(SEISPP_verbose) 
						cout<<"!!!Caution: Output table is not empty. Working under continue mode."<<endl;
					logfile<<"Output table is not empty. Working under continue mode."<<endl;
					apply_subset=true;
					if(subset_condition.length()> 0)
						subset_condition=subset_condition+string("&&")+string("sta > ")
							+quote+laststation+quote;
					else
						subset_condition=string("sta > ")+quote
							+laststation+quote;
				}
				else
					logfile<<"Output table is not empty. User chose to "
							<<"re-do from the beginning."<<endl;
			}
		}
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
				
				if(datatype3c)
				{
					string sstring=string("decon.chan==")+quote+string(edit_chan_code)+quote;
					dbin.subset(string(sstring));
					/*
					if(subset_condition.length()> 0)
						subset_condition=subset_condition+string("&&")+string("decon.chan==")
							+quote+string(rchan)+quote;
					else
						subset_condition=string("decon.chan==")+quote+string(rchan)+quote;
					*/
				}
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
        logfile << "Number of rows in full wfdisc/wfprocess originally openned (may have duplicates) = "
            <<dbin.number_tuples()<<endl;
        /* Prep the input wfdisc table*/
        /* WARNING:  not adequate.  This needs to be changed as we 
           need evid.  May be able to fake this by searching for matching
           start times  */
        if(apply_subset)
        {
            cout << "Applying subset condition = [ "<<subset_condition<<" ]."<<endl;
            logfile << "Applied subset condition = [ "<<subset_condition<<" ]."<<endl;
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
				*******<<<<<<<<<<<< PRE-FILTER SETUP >>>>>>>>>>********
				*****************************************************************
				*****************************************************************
				*/
		int nwsamp;
		double datadt, wavelet_width_parameter;
		WaveletNormalizationMethod nm=PEAK;
		TimeSeries wavelet;
		
        if(apply_prefilter) 
        {
        	logfile<<"Pre-filter type: "<<wavelet_type<<endl;
        	if(wavelet_type=="filter")
			{
				filterspec=control.get_string("filter");
				//debug
				if(SEISPP_verbose)
				{
					cout<<"Data will be pre-filered before stacking."<<endl;
					cout<<"Pre-filter specs = "<<filterspec<<endl;
				}
				logfile<<"Filter specs = "<<filterspec<<endl;
			}
			else if(wavelet_type=="gaussian")
			{
				nwsamp=control.get_int("wavelet_length");
				datadt=control.get_double("data_sample_interval");
				wavelet_width_parameter=control.get_double("wavelet_width_parameter");
				wavelet=gaussian_wavelet(nwsamp,datadt,
						wavelet_width_parameter,nm);
				//logfile<<"Pre-filter type: "<<wavelet_type<<endl;
			}
			// ricker wavelet is not suitable for RFeditor
			else if(wavelet_type=="ricker")
			{
				nwsamp=control.get_int("wavelet_length");
				datadt=control.get_double("data_sample_interval");
				wavelet_width_parameter=control.get_double("wavelet_width_parameter");
				wavelet=ricker_wavelet(nwsamp,datadt,
						wavelet_width_parameter,nm);
			}
			
			else
			{
				cout << "-"<<wavelet_type<<"-"<<endl;
				cout << "wavelet_type parameter="<<wavelet_type
					<< " is not supported."<<endl
					<< "Must be:  filter, gaussian"<<endl;
				usage();
			}
        	
        }
        		/****************************************************************
				*****************************************************************
				******<<<<<<<<<<<< BUILD GLOBAL EDITING OBJECTS >>>>>>>>>>*******
				*****************************************************************
				*****************************************************************
				*/
        /* This launches the editing windows */
        RFeditorEngine *rfe;
        //if(!GUIoff) 
        	rfe= new RFeditorEngine(trace_edit_params, GUIoff);
        /*TraceEditOperator object for trace editing.*/
        TraceEditOperator teo(trace_edit_params);
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
		FILE * fh_fa;
		if(get_FA)
		{
			fh_fa=fopen(FA_filename.c_str(),"w");
			if(use_arrival_data)
				fprintf(fh_fa,"STA    EVID    START_TIME    FA_LAG    FA_AMPR    SEAZ\n");
			else
				fprintf(fh_fa,"STA    EVID    START_TIME    FA_LAG    FA_AMPR\n");
		}
        for(i=0,dbin.rewind();i<nsta;++i,++dbin)
        {   
            cout <<">>++++++++++++++++++++++++++++++"<<endl
            	<<"Calling data reader for ensemble number ["<<i+1<<" / "<<nsta<<"]"<<endl;
            logfile<<">>++++++++++++++++++++++++++++++"<<endl
            	<<"Working on ensemble number ["<<i+1<<" / "<<nsta<<"]"<<endl;
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
				logfile << "Read "<<dall.member.size()<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;
				cout << "Read "<<dall.member.size()<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;

				if(ntrace<minrfcutoff)
				{
					cout << "Station "<<sta<<" dropped.   Count below miminum of "
						<< minrfcutoff<<endl;
					logfile << "Station "<<sta
						<<" dropped. Count below miminum of "
						<< minrfcutoff<<endl;
					continue;
				}
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
				//TimeWindow twin0=teo.find_common_timewindow(dall);
                    //debug common window
                  //  cout<<"debug: twin.start="<<twin0.start<<", end="<<twin0.end<<endl;
                   // cout<<"debug: twin length="<<twin0.end-twin0.start<<endl;
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
					{	if(save_vertical_channel || edit_on_vertical) 
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
				
				if(edit_on_radial) tse_edit=radial;
				else if(edit_on_transverse) tse_edit=transverse;
				else if(edit_on_vertical) tse_edit=vertical;
            }
            // load 3 component data from the view.
            //
            else
            {
            	//try{
            	ThreeComponentEnsemble dall0_3c(dbin, mdl,mdlens,am);
            	//}catch(SeisppError& serr)
            	//{
            	//	cerr<<"ERROR in builing ThreeComponentEnsemble:"<<endl;
            	//	serr.log_error();
            	//	exit(-1);
            	//}
            	dall_3c=dall0_3c;
            	dall0_3c.member.clear();
            	//if(!save_filtered_data && apply_prefilter) dall_3c_bkp=dall0_3c;
            	sta=dall_3c.get_string("sta");
            	ntrace=dall_3c.member.size();
            	//nradial=ntrace;
            	tracetype=const_cast<char *>(tracetype_3c.c_str());
            	logfile << "Read "<<ntrace<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;
				cout << "Read "<<ntrace<<" "<<tracetype<<" RF traces for station = "
					<< sta <<endl;

				if(ntrace<minrfcutoff)
				{
					cout << "Station "<<sta<<" dropped.   Count below miminum of "
						<< minrfcutoff<<endl;
					logfile << "Station "<<sta
						<<" dropped. Count below miminum of "
						<< minrfcutoff<<endl;
					continue;
				}
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
				
				//extract TimeSeries ensemble by chan codes.
				try
				{
					if(save_3C_data || review_mode)
					{
						auto_ptr<TimeSeriesEnsemble> data=ExtractComponent(dall_3c,edit_component);
						tse_edit=*data;
						data.reset();
						/*
						if(pre_edit_FA && !edit_on_radial) 
						{
							auto_ptr<TimeSeriesEnsemble> rdata=ExtractComponent(dall_3c,1);
							radial=*rdata;
							rdata.reset();
						}
						*/
					}
					else
					{
						auto_ptr<TimeSeriesEnsemble> tdata=ExtractComponent(dall_3c,0);
						transverse=*tdata;
						transverse.put("chan",tchan);
						auto_ptr<TimeSeriesEnsemble> rdata=ExtractComponent(dall_3c,1);
						radial=*rdata;
						radial.put("chan",rchan);
						tdata.reset();
						rdata.reset();
						if(save_vertical_channel) 
						{
							auto_ptr<TimeSeriesEnsemble> zdata=ExtractComponent(dall_3c,2);
							vertical=*zdata;
							vertical.put("chan",zchan);
							zdata.reset();
						}
						
						if(edit_on_radial) tse_edit=radial;
						else if(edit_on_transverse) tse_edit=transverse;
						else if(edit_on_vertical) tse_edit=vertical;
					}
				}catch(SeisppError& serr)
				{
					cerr << "Problems extracting TimeSeriesEnsemble by component.  Message "
						<<serr.what()<<endl
						<<"Skipping ensemble for station = "<<sta
						<<endl;
					continue;
				}				
            }
			
			tse_edit.put("chan",edit_chan_code);
			
            int j=set_duplicate_traces_to_false(tse_edit,false);
            if(j>0 && SEISPP_verbose)
            	cout<<"Duplicate traces in "<<edit_on_channel<<" (set to false) = "<<j<<endl;

            //kill timeseries with t0>0
            for(long i=0;i<tse_edit.member.size();i++)
            {
            	if(tse_edit.member[i].t0>FA_reference_time+MYZERO)
            		{
            			cout<<"***Set trace with t0 > FA_reference_time to FALSE!"<<endl
            				<<"    Start time:"
            				<<strtime(tse_edit.member[i].get_double(string("time")))<<endl;
            			tse_edit.member[i].live=false;
            			//transverse.member[i].live=false;
            			//if(pre_edit_FA && !edit_on_radial) radial.member[i].live=false;
            		}
            }
            set<long> evids_killed=teo.find_false_traces(tse_edit);
            //save original data before applying filters. Xiaotao Yang 1/22/2015
            if(SEISPP_verbose) cout<<"-- Excluding false traces ..."<<endl;
            /*
            if(pre_edit_FA && !edit_on_radial)
            {	radial0=teo.exclude_false_traces(radial);
            	radial=radial0;
            }*/
            //transverse0=teo.exclude_false_traces(transverse);
            //radial=radial0;
            //transverse=transverse0;
            tse_edit0=teo.exclude_false_traces(tse_edit);
            tse_edit=tse_edit0;
			cout<<"After excluding false traces: "<<edit_on_channel<<" = "<<tse_edit.member.size()<<endl;
			//	<<", transverse = "<<transverse.member.size()<<endl;
			// apply prefilter.Xiaotao Yang 01/12/2015
			cout<<"** Pre-filtering :: ";
			if(apply_prefilter) 
			{
				if(wavelet_type=="filter")
				{	
					TimeInvariantFilter filter(filterspec);
					logfile<<"Applying TimeInvariant Filter to "<<edit_on_channel<<" ensemble ..."<<endl;
					if(SEISPP_verbose) 
						cout<<"Applying TimeInvariant Filter to "<<edit_on_channel<<" ensemble ..."<<endl;
					//SEISPP::FilterEnsemble(radial,filter);
					SEISPP::FilterEnsemble(tse_edit,filter);
					//if(pre_edit_FA && !edit_on_radial) SEISPP::FilterEnsemble(radial0,filter);
				}
				else if(wavelet_type=="gaussian" || wavelet_type=="ricker")
				{
					//TimeWindow twin(radial.member[0].t0,radial.member[0].endtime());
                    TimeWindow twin=teo.find_common_timewindow(tse_edit);
                    //debug common window
                    //cout<<"debug: twin.start="<<twin.start<<", end="<<twin.end<<endl;
                    //teo.convolve_ensemble(wavelet,radial,true,&twin);
                    if(SEISPP_verbose) 
						cout<<"Convolving "<<edit_on_channel<<" ensemble with wavelet: "<<wavelet_type<<" ..."<<endl;
                    teo.convolve_ensemble(wavelet,tse_edit,true,&twin);
                    //if(pre_edit_FA && !edit_on_radial) teo.convolve_ensemble(wavelet,radial0,true,&twin);
				}
			}  // end of applying filter

			/*
			|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
			|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
			<<<<<<<<<<<<<<<<<<<<<<<< Starting Editor >>>>>>>>>>>>>>>>>>>>>>>>
			|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
			|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
			*/
			if(get_FA && pre_edit_FA)
			//compute FA and write them out into a text file.
			{
				if(SEISPP_verbose) cout<<"Detecting first arrivals before applying editings ..."<<endl;
				TimeWindow FA_search_window=TimeWindow(FA_search_TW_start+FA_reference_time,
							FA_search_TW_end+FA_reference_time);
				vector<TimeSeries>::iterator iptr;
				TimeSeriesEnsemble tse_tmp(tse_edit);
				//if(!edit_on_radial) tse_tmp=radial0;
				for(iptr=tse_tmp.member.begin(); iptr!=tse_tmp.member.end();iptr++)
				{
					TimeSeries tmpts=teo.trim_data(*iptr,FA_search_window);
					string FA_type=find_FirstArrival(tmpts,FA_sensitivity,
								FA_detect_length,data_shaping_wavelet_type);
					double FA_time=tmpts.get_double(FA_time_key);
					double FA_lag=FA_time - FA_reference_time;
					double FA_amplitude=tmpts.get_double(FA_amplitude_key);
					int evid_tmp=iptr->get_int(evidkey);
					if(use_arrival_data)
						fprintf(fh_fa,"%5s   %6d    %15.3f   %6.3f   %8.4f   %6.1f\n",
							sta.c_str(),evid_tmp,iptr->get_double("time"),
							FA_lag,FA_amplitude,iptr->get_double(seaz_key));
					else
						fprintf(fh_fa,"%5s   %6d    %15.3f   %6.3f   %8.4f\n",
							sta.c_str(),evid_tmp,iptr->get_double("time"),
							FA_lag,FA_amplitude);
					
					tmpts.s.clear();
				}
				
				if(SEISPP_verbose) 
					cout<<"Saved [ "<<tse_tmp.member.size()
						<<" ] FA information for "<<edit_on_channel<<" data of station [ "<<sta<<" ]."<<endl;
				tse_tmp.member.clear();
				//exit(0);
			}
			
			if(!review_mode || !GUIoff)
			{
				if(SEISPP_verbose) cout<< "Loading data into editor ..."<<endl;
					//<< "When read, edit radial first then transverse."
					//<<endl;
		
				set<long> kills; //,tkills;
				if(!GUIoff)
					try{
						kills=rfe->edit(tse_edit); 
					}catch(...)
					{
						cerr<<"** Error in running GUI editor!"<<endl;
						exit(-1);
					}
				else
					try{
						kills=rfe->edit(tse_edit,trace_edit_params); 
					}catch(...)
					{
						cerr<<"** Error in running GUIoff editor!"<<endl;
						exit(-1);
					}
				rfe->save_statistics(logfilename);

				// stacktype for stack type. Xiaotao Yang
		
				/*
				=================================================================
				=================================================================
				<<<<<<<<<<<<<<<<<<<<<<<<<< Save or Not ? >>>>>>>>>>>>>>>>>>>>>>>>
				=================================================================
				=================================================================
				*/
				//find total number of killed traces.
				set<long> evids_killed2;
				evids_killed2=kills;
				//cout<<evids_killed2.size()<<endl;
				if(evids_killed2.size()>0)
					evids_killed.insert(evids_killed2.begin(),evids_killed2.end());
				cout<<"Found [ "<<evids_killed.size()<<" ] killed traces."<<endl;
				if(review_mode) 
				{
					cout<<"Review mode is on. Go to the next without saving the edits!"<<endl;
					rfe->reset_statistics();
				}
				else
				{
					//compute and save FA information
					if(get_FA && post_edit_FA )
					//compute FA and write them out into a text file.
					{
						if(evids_killed.size()>0)
						{
							if(SEISPP_verbose) cout<<"Getting FA: Applying kills to "
							<<edit_on_channel<<endl;
							teo.apply_kills(tse_edit,evids_killed);
						}
						TimeSeriesEnsemble tse_tmp=teo.exclude_false_traces(tse_edit);
						if(SEISPP_verbose) 
							cout<<"Getting FA: Detecting first arrivals after applying editings ..."<<endl;
						TimeWindow FA_search_window=TimeWindow(FA_search_TW_start+FA_reference_time,
									FA_search_TW_end+FA_reference_time);
						vector<TimeSeries>::iterator iptr;
						for(iptr=tse_tmp.member.begin(); iptr!=tse_tmp.member.end();iptr++)
						{
							TimeSeries tmpts=teo.trim_data(*iptr,FA_search_window);
							string FA_type=find_FirstArrival(tmpts,FA_sensitivity,
										FA_detect_length,data_shaping_wavelet_type);
							double FA_time=tmpts.get_double(FA_time_key);
							double FA_lag=FA_time - FA_reference_time;
							double FA_amplitude=tmpts.get_double(FA_amplitude_key);
							int evid_tmp=iptr->get_int(evidkey);
							if(use_arrival_data)
								fprintf(fh_fa,"%5s   %6d    %15.3f   %6.3f   %8.4f   %6.1f\n",
									sta.c_str(),evid_tmp,iptr->get_double("time"),
									FA_lag,FA_amplitude,iptr->get_double(seaz_key));
							else
								fprintf(fh_fa,"%5s   %6d    %15.3f   %6.3f   %8.4f\n",
									sta.c_str(),evid_tmp,iptr->get_double("time"),
									FA_lag,FA_amplitude);
							tmpts.s.clear();
						}
				
						if(SEISPP_verbose) 
							cout<<"Saved [ "<<tse_tmp.member.size()
								<<" ] FA information for "<<edit_on_channel
								<<" data of station [ "<<sta<<" ]."<<endl;
						tse_tmp.member.clear();
						//exit(0);
					}

					int nsaved;
					string outtable;
					////save edit statistics information first.
					if(save_edit_summary_to_file)
					{
						rfe->save_statistics_summary(editsummaryfname,csversion);
					}
					//save statistics to db table: tredit.
					rfe->save_statistics_summary(dbout,2,csversion);
					//reset is required to avoiding duplicate/accumulated statistical information.
					rfe->reset_statistics();
					//starting saving waveform data.
				
					if(save_3C_data)
					{
						if(SEISPP_verbose) 
							cout <<"Applying kills to 3C data ... "<<endl;
						teo.apply_kills(dall_3c, evids_killed);
						vector<ThreeComponentSeismogram>::iterator im2;
						if(apply_prefilter)
						{
							if(save_filtered_data)
							{
								if(wavelet_type=="filter")
								{
									TimeInvariantFilter filter(filterspec);
									SEISPP::FilterEnsemble(dall_3c,filter);
								}
								else if(wavelet_type=="gaussian" || wavelet_type=="ricker")
								{
									//TimeWindow twin(dall_3c.member[0].t0,dall_3c.member[0].endtime());
									TimeWindow twin=teo.find_common_timewindow(dall_3c);
									teo.convolve_ensemble(wavelet,dall_3c,true,&twin);
								}
							}
						}
						for(im2=dall_3c.member.begin();im2!=dall_3c.member.end();++im2)
						{	
							//change back to absolute time.
							if(use_arrival_data)
							{
								double atime=im2->get_double("atime");
								im2->rtoa(atime-FA_reference_time);
							}
							else
							{
								double t0=im2->get_double("time");
								im2->rtoa(t0);
							}
						}
						//save wfdisc table seperately if use wfprocess as input
                        if(save_wfdisc_table)
                        {
							if(SEISPP_verbose) cout <<"Saving to db (wfdisc). Please wait ..."<<endl;
							outtable="wfdisc";
							save_to_db(dall_3c,mdlout_wfd,
									am,dbout,outdir,outdfile_base,
									save_metadata_only,false,rchan,tchan,zchan,outtable);
						}
						//saving edits to db.
						if(SEISPP_verbose) cout <<"Saving to db (wfprocess). Please wait ..."<<endl;
								//Xiaotao Yang 1/16/2015
						outtable="wfprocess";
						nsaved=save_to_db(dall_3c,mdlout,
									am,dbout,outdir,outdfile_base,
									save_metadata_only,save_decon_table);
					}
					else
					{
						if(apply_prefilter)
						{
							if(!save_filtered_data)
							{
								//radial=radial0;
								//transverse=transverse0;
								// apply kills. moved from RFeditorEngine.cc to this place. Xiaotao Yang
								if(evids_killed.size()>0)
								{
									//debug
									if(SEISPP_verbose) cout<<"Applying kills to radial."<<endl;
									//cout<<"rkill size: "<<rkills.size()<<endl;
									teo.apply_kills(radial,evids_killed);
									
									if(SEISPP_verbose) cout<<"Applying kills to transverse."<<endl;
									teo.apply_kills(transverse,evids_killed);
									//kill vertical if turned on "save vertical channel"
									if(save_vertical_channel)
									{
										if(SEISPP_verbose) cout<<"Applying kills to vertical ..."<<endl;
										teo.apply_kills(vertical,evids_killed);
									}
								}
							}
							else  //save filtered data
							{
								if(wavelet_type=="filter")
								{	
									TimeInvariantFilter filter(filterspec);
									SEISPP::FilterEnsemble(radial,filter);
									SEISPP::FilterEnsemble(vertical,filter);
									SEISPP::FilterEnsemble(transverse,filter);
								}
								else if(wavelet_type=="gaussian" || wavelet_type=="ricker")
								{
									//TimeWindow twin(radial.member[0].t0,radial.member[0].endtime());
                                    TimeWindow twin=teo.find_common_timewindow(radial);
                                    teo.convolve_ensemble(wavelet,radial,true,&twin);
									teo.convolve_ensemble(wavelet,vertical,true,&twin);
									teo.convolve_ensemble(wavelet,transverse,true,&twin);
								}
								if(evids_killed.size()>0)
								{
									//debug
									//radial=radial0;
									if(SEISPP_verbose) cout<<"Applying kills to radial."<<endl;
									//cout<<"rkill size: "<<rkills.size()<<endl;
									teo.apply_kills(radial,evids_killed);
									
									if(SEISPP_verbose) cout<<"Applying kills to transverse."<<endl;
									teo.apply_kills(transverse,evids_killed);
									//kill vertical if turned on "save vertical channel"
									if(save_vertical_channel)
									{
										if(SEISPP_verbose) cout<<"Applying kills to vertical ..."<<endl;
										teo.apply_kills(vertical,evids_killed);
									}
								}
							}
						}
						else
						{	
							if(evids_killed.size()>0)
								{
									//debug
									if(SEISPP_verbose) cout<<"Applying kills to radial."<<endl;
									//cout<<"rkill size: "<<rkills.size()<<endl;
									teo.apply_kills(radial,evids_killed);
									
									if(SEISPP_verbose) cout<<"Applying kills to transverse."<<endl;
									teo.apply_kills(transverse,evids_killed);
									//kill vertical if turned on "save vertical channel"
									if(save_vertical_channel)
									{
										if(SEISPP_verbose) cout<<"Applying kills to vertical ..."<<endl;
										teo.apply_kills(vertical,evids_killed);
									}
								}
						}
						
						
						// change time reference to absolute.
						double t0;
						for(im=radial.member.begin();im!=radial.member.end();++im)
						{
							//im->rtoa(t0);
							if(use_arrival_data)
							{
								double atime=im->get_double("atime");
								im->rtoa(atime-FA_reference_time);
							}
							else
							{
								t0=im->get_double("time");
								im->rtoa(t0);
							}
							//Fragile way to handle this, but skip stack traces
							//if(!(im->live))
							//	logfile << "Deleting " <<sta<<":"<< im->get_string("chan")
							//		<< " for time "<<strtime(t0)<<endl;
						
						}
					
						for(im=transverse.member.begin();im!=transverse.member.end();++im)
						{
							//double t0=im->get_double("time");
							//im->rtoa(t0);
							if(use_arrival_data)
							{
								double atime=im->get_double("atime");
								im->rtoa(atime-FA_reference_time);
							}
							else
							{
								t0=im->get_double("time");
								im->rtoa(t0);
							}
						}
						if(save_vertical_channel)
							for(im=vertical.member.begin();im!=vertical.member.end();++im)
							{
								//double t0=im->get_double("time");
								//im->rtoa(t0);
								if(use_arrival_data)
								{
									double atime=im->get_double("atime");
									im->rtoa(atime-FA_reference_time);
								}
								else
								{
									t0=im->get_double("time");
									im->rtoa(t0);
								}
							}
						//saving edits to db.
						
						if(!use_wfdisc_in) 
						{	
							
							if(save_wfdisc_table)
							{
								if(SEISPP_verbose) cout <<"Saving to db (wfdisc). Please wait ..."<<endl;  
								nsaved=save_to_db(radial,transverse,vertical,mdlout_wfd,
										am,dbout,outdir,outdfile_base,
										save_metadata_only,save_vertical_channel,
										save_decon_table,rchan, tchan, zchan,"wfdisc");
							}
							outtable="wfprocess";
						}
						else
						{
							if(save_wfprocess_table)
							{
								if(SEISPP_verbose) cout <<"Saving to db (wfprocess). Please wait ..."<<endl;  
								nsaved=save_to_db(radial,transverse,vertical,mdlout_wfp,
										am,dbout,outdir,outdfile_base,
										save_metadata_only,save_vertical_channel,
										save_decon_table,rchan, tchan, zchan,"wfprocess");
							}
							outtable="wfdisc";
						}
						
						if(SEISPP_verbose) cout <<"Saving to db ("<<outtable<<"). Please wait ..."<<endl;
						nsaved=save_to_db(radial,transverse,vertical,mdlout,
										am,dbout,outdir,outdfile_base,
										save_metadata_only,save_vertical_channel,
										save_decon_table,rchan, tchan, zchan,outtable);
					}
					
					logfile << "Saved "<<nsaved<<" RFs for station "<<sta<<endl;
					if(SEISPP_verbose) cout << "Saved "<<nsaved<<" RFs for station "<<sta<<endl;
				}
				evids_killed2.clear();
				kills.clear(); 
            }
			//clear set containers.
			evids_killed.clear();   
			dall_3c.member.clear();    
        }
        radial.member.clear();
        transverse.member.clear();
        vertical.member.clear();
        tse_edit0.member.clear();
        tse_edit.member.clear();
        if(get_FA) fclose(fh_fa);
        if(SEISPP_verbose) cout<<"RFeditor finished."<<endl;
        logfile << "RFeditor finished on [ " << dbin_name << " ] at time "
            << strtime(now()) <<endl;
        logfile.close();
        //delete rfe;
    }catch(SeisppError& serr)
    {
        serr.log_error();
    }
    catch(exception& stdexcept)
    {
        cerr << "Exception thrown:  "<<stdexcept.what()<<endl;
    }
}
//END OF RFEDITOR.