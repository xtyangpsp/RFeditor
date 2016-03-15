#include <string>
#include <fstream>
#include <vector>
#include <list>
#include "seispp.h"
#include "dbpp.h"
#include "TraceEditOperator.h"
using namespace std;
using namespace SEISPP;
/*
This program is used to convert older tredit table to current version.
Currently, we only convert version 1 of tredit table, which is the original
version, with the following entries (by order from left to right):
**
sta ntracein nkilled acptrate aknfa aklat aksfa akgpc akca aklfc klsw klxcor kldsi klrfqi kdnitn kdnspike kdepsilon kdpkamp kdavamp kdsnr kmanual nrestored
**

This program reads stdin.

Xiaotao Yang @IU
3/14/2016

*/
void usage()
{
	cerr<<"USAGE: convert_tredit dbout [-version versionID -v|V] < input_tredit_table"<<endl;
	cerr<<"**For dbout, avoid using same name as the input db "<<endl
		<<"  containing the tredit table to be converted."<<endl
		<<"**The old tredit table is treated/readin as a plain text file."<<endl;
	exit(-1);
}
bool SEISPP::SEISPP_verbose(false);
int main(int argc, char **argv)
{
	int i;
	ios::sync_with_stdio();
	if(argc<2) usage();
	string dbout(argv[1]);
	int versionid(1);
	for(i=2;i<argc;++i)
	{
		string argstr=string(argv[i]);
		if((argstr=="-v") || (argstr=="-V"))
			SEISPP_verbose=true;
		else if(!strcmp(argv[i],"-version"))
		{
			++i;
			if(i>=argc) usage();
			versionid=atoi(argv[i]);
			if(versionid!=1)
			{
				cerr<<"ERROR: current version of this program only "<<endl
					<<"converts from versionID=1 tredit table."<<endl;
				usage();
			}
		}
		else
		{
			cerr << "Unknown argument = "<<argstr<<endl;
			usage();
		}
	}

	try 
	{
		string temp_line,line;
		int j(1),nsta(0);
		int aknfa=0, aklat=0, aksfa=0, akgpc=0, akca=0, aklfc=0;
		int klsw=0, klxcor=0, kldsi=0, klrfqi=0,kdnitn=0, kdnspike=0;
		int kdepsilon=0,kdpkamp=0,kdavamp=0, kdsnr=0, manual_nrestored=0;
		int total_nkilled=0, ntracein=0,ntraceout(0),manual_nkilled(0);
		double acptrate(0.0);
		string sta="-",rfe_version="converted";
		
		DatascopeHandle dbtredit(dbout,false);
		dbtredit.lookup("tredit");
		if(dbtredit.number_tuples()>0)
		{
			cerr<<"ERROR: output tredit table [ "<<dbout<<".tredit ] is NOT empty! Cleanup first!"<<endl;
			exit(-1);
		}
		switch(versionid)
		{
			case 1:
				/*ORDER OF ATTRIBUTES IN v1 tredit table
				sta ntracein nkilled acptrate aknfa aklat aksfa akgpc 
				akca aklfc klsw klxcor kldsi klrfqi kdnitn kdnspike 
				kdepsilon kdpkamp kdavamp kdsnr kmanual nrestored
				*/
				j=1;
				while(getline(cin,temp_line))
				{
					stringstream line(temp_line);
					line>>sta>>ntracein>>total_nkilled>>acptrate>>aknfa>>aklat>>aksfa>>akgpc>>akca;
					line>>aklfc>>klsw>>klxcor>>kldsi>>klrfqi>>kdnitn>>kdnspike>>kdepsilon;
					line>>kdpkamp>>kdavamp>>kdsnr>>manual_nkilled>>manual_nrestored;
					ntraceout=ntracein - total_nkilled;

					/* attributes in tredit table.
					sta ntracein ntraceout acceptancerate nkilled procedure version
					//procedure: name of the editing method.
					*/
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",aklat);
					dbtredit.put("procedure",AUTOKILL_LargeAmpTraces);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdnitn);
					dbtredit.put("procedure",AUTOKILL_DeconNiteration);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdnspike);
					dbtredit.put("procedure",AUTOKILL_DeconNspike);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdepsilon);
					dbtredit.put("procedure",AUTOKILL_DeconEpsilon);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdpkamp);
					dbtredit.put("procedure",AUTOKILL_DeconPeakamp);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdavamp);
					dbtredit.put("procedure",AUTOKILL_DeconAveramp);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kdsnr);
					dbtredit.put("procedure",AUTOKILL_DeconRawsnr);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",kldsi);
					dbtredit.put("procedure",AUTOKILL_DSICutoff);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",aknfa);
					dbtredit.put("procedure",AUTOKILL_NegativeFA);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",akgpc);
					dbtredit.put("procedure",AUTOKILL_GrowingPCoda);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",akca);
					dbtredit.put("procedure",AUTOKILL_ClusteredArrivals);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",aksfa);
					dbtredit.put("procedure",AUTOKILL_SmallFA);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",aklfc);
					dbtredit.put("procedure",AUTOKILL_LowFrequencyContaminated);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",klsw);
					dbtredit.put("procedure",AUTOKILL_StackWeightCutoff);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",klxcor);
					dbtredit.put("procedure",AUTOKILL_RefXcorCutoff);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",klrfqi);
					dbtredit.put("procedure",AUTOKILL_RFQICutoff);
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",manual_nkilled);
					dbtredit.put("procedure","ManualEdit");
					dbtredit.put("version",rfe_version);
			
					dbtredit.append();
					dbtredit.put("sta",sta);
					dbtredit.put("ntracein",ntracein);
					dbtredit.put("ntraceout",ntraceout);
					dbtredit.put("acceptancerate",acptrate);
					dbtredit.put("nkilled",manual_nrestored);
					dbtredit.put("procedure",ManualRestore);
					dbtredit.put("version",rfe_version);		
		
				j++;
				}
				nsta=j--;
				break;
			case 2:
				cerr<<"ERROR: current version of this program only "<<endl
					<<"converts from versionID=1 tredit table."<<endl;
				usage();
			default:
				cerr<<"ERROR: current version of this program only "<<endl
					<<"converts from versionID=1 tredit table."<<endl;
				usage();
		}
		cout<<"Converted [ "<<nsta<<" ] stations."<<endl
			<<"Saved to [ "<<dbout<<".tredit ]!"<<endl;
			
	}
	catch (SeisppError& serr)
	{
		serr.log_error();
	}
}
