#include <string>
#include <fstream>
#include <vector>
#include "perf.h"
#include "TraceEditOperator.h"
using namespace std;
using namespace SEISPP;
   //1e-10.
/*
//RFeditor Editing procedures/methods used in RFeditor.
//
// Xiaotao Yang Jan 2015 - Mar 2016
// Indiana University
*/
/////////////////////////
//// Define TraceEditStatistics class methods.
TraceEditStatistics::TraceEditStatistics()
{
	this->set_defaults();
}
void TraceEditStatistics::set_defaults()
{
	station=STATISTICS_STRINGDEFAULT;
	method=STATISTICS_STRINGDEFAULT;
	FA_twin=STATISTICS_TWDEFAULT;
	PCoda_twin=STATISTICS_TWDEFAULT;
	CodaCA_twin=STATISTICS_TWDEFAULT;
	XCor_twin=STATISTICS_TWDEFAULT;
	Robust_twin=STATISTICS_TWDEFAULT;
	Stack_twin=STATISTICS_TWDEFAULT;
	AmpNormalizeMethod=STATISTICS_STRINGDEFAULT;
	ref_trace_tag=STATISTICS_STRINGDEFAULT;
	CodaCA_tolerance_twin_length=STATISTICS_DOUBLEDEFAULT;
	PCoda_grow_tolerance=STATISTICS_DOUBLEDEFAULT;
	FA_range[0]=STATISTICS_DOUBLEDEFAULT;  //normalized FA
	FA_range[1]=FA_range[0];
	LowFrequency_range[0]=STATISTICS_DOUBLEDEFAULT;
	LowFrequency_range[1]=LowFrequency_range[0];
	TraceAmp_range[0]=STATISTICS_DOUBLEDEFAULT;
	TraceAmp_range[1]=TraceAmp_range[0];
	RefXcor_min=STATISTICS_DOUBLEDEFAULT;
	StackW_min=STATISTICS_DOUBLEDEFAULT;
	DSI_min=STATISTICS_DOUBLEDEFAULT;
	for(int i=0;i<3;++i) QI_weights[i]=STATISTICS_DOUBLEDEFAULT; //QI: Quality Index.
	QI_min=STATISTICS_DOUBLEDEFAULT;
	DeconNiteration_range[0]=STATISTICS_INTDEFAULT; //2 column for min and max.
	DeconNiteration_range[1]=DeconNiteration_range[0];
	DeconNspike_range[0]=STATISTICS_INTDEFAULT;
	DeconNspike_range[1]=DeconNspike_range[0];
	DeconEpsilon_range[0]=STATISTICS_DOUBLEDEFAULT;
	DeconEpsilon_range[1]=DeconEpsilon_range[0];
	DeconPeakamp_range[0]=STATISTICS_DOUBLEDEFAULT;
	DeconPeakamp_range[1]=DeconPeakamp_range[0];
	DeconAveramp_range[0]=STATISTICS_DOUBLEDEFAULT;
	DeconAveramp_range[1]=DeconAveramp_range[0];
	DeconRawsnr_range[0]=STATISTICS_DOUBLEDEFAULT;
	DeconRawsnr_range[1]=DeconRawsnr_range[0];
	nkilled=STATISTICS_INTDEFAULT;
}
void TraceEditStatistics::print_header()
{
	cerr<<"======= TraceEditStatistics Header ======="<<endl
		<<"<  Station Method N_Kills N_Parameters  >"<<endl
		<<"par1 value1"<<endl
		<<"par2 value2"<<endl
		<<".... ......"<<endl
		<<"parn valuen"<<endl
		<<"=========================================="<<endl<<endl;
}
void TraceEditStatistics::print_header(string fname)
{
	fh = fopen(fname.c_str(),"a");
	fprintf(fh,"======= TraceEditStatistics Header =======\n");
	fprintf(fh,"<  Station Method N_Kills N_Parameters  >\n");
	fprintf(fh,"par1 value1\n");
	fprintf(fh,"par2 value2\n");
	fprintf(fh,".... ......\n");
	fprintf(fh,"parn valuen\n");
	fprintf(fh,"==========================================\n\n");
	fclose(fh);
}
int TraceEditStatistics::count_parameters()
{
	int numpars=0;
	
	if(FA_twin.start!=STATISTICS_TWDEFAULT.start) ++numpars;
	if(FA_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(PCoda_twin.start!=STATISTICS_TWDEFAULT.start)++numpars;
	if(PCoda_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(CodaCA_twin.start!=STATISTICS_TWDEFAULT.start)++numpars;
	if(CodaCA_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(XCor_twin.start!=STATISTICS_TWDEFAULT.start)++numpars;
	if(XCor_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(Robust_twin.start!=STATISTICS_TWDEFAULT.start)++numpars;
	if(Robust_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(Stack_twin.start!=STATISTICS_TWDEFAULT.start)++numpars;
	if(Stack_twin.end!=STATISTICS_TWDEFAULT.end) ++numpars;
	if(AmpNormalizeMethod!=STATISTICS_STRINGDEFAULT) ++numpars;
	if(ref_trace_tag!=STATISTICS_STRINGDEFAULT) ++numpars;
	if(CodaCA_tolerance_twin_length!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(PCoda_grow_tolerance!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(FA_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;  //normalized FA
	if(FA_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(LowFrequency_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;  
	if(LowFrequency_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(TraceAmp_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(TraceAmp_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(RefXcor_min!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(StackW_min!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DSI_min!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	for(int i=0;i<3;++i) {if(QI_weights[i]!=STATISTICS_DOUBLEDEFAULT) ++numpars;}
	if(QI_min!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconNiteration_range[0]!=STATISTICS_INTDEFAULT) ++numpars; //2 column for min and max.
	if(DeconNiteration_range[1]!=STATISTICS_INTDEFAULT) ++numpars;
	if(DeconNspike_range[0]!=STATISTICS_INTDEFAULT) ++numpars;
	if(DeconNspike_range[1]!=STATISTICS_INTDEFAULT) ++numpars;
	if(DeconEpsilon_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconEpsilon_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconPeakamp_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconPeakamp_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconAveramp_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconAveramp_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconRawsnr_range[0]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	if(DeconRawsnr_range[1]!=STATISTICS_DOUBLEDEFAULT) ++numpars;
	
	return(numpars);
}
void TraceEditStatistics::print_parameters()
{
	if(FA_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("FA_twin_start  %8.2f\n",FA_twin.start);
	if(FA_twin.end!=STATISTICS_TWDEFAULT.end)
		printf("FA_twin_end  %8.2f\n",FA_twin.end);
	if(PCoda_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("PCoda_twin_start  %8.2f\n",PCoda_twin.start);
	if(PCoda_twin.end!=STATISTICS_TWDEFAULT.end) 
		printf("PCoda_twin_end  %8.2f\n",PCoda_twin.end);
	if(CodaCA_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("CodaCA_twin_start  %8.2f\n",CodaCA_twin.start);
	if(CodaCA_twin.end!=STATISTICS_TWDEFAULT.end) 
		printf("CodaCA_twin_end  %8.2f\n",CodaCA_twin.end);
	if(XCor_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("XCor_twin_start  %8.2f\n",XCor_twin.start);
	if(XCor_twin.end!=STATISTICS_TWDEFAULT.end) 
		printf("XCor_twin_end  %8.2f\n",XCor_twin.end);
	if(Robust_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("Robust_twin_start  %8.2f\n",Robust_twin.start);
	if(Robust_twin.end!=STATISTICS_TWDEFAULT.end) 
		printf("Robust_twin_end  %8.2f\n",Robust_twin.end);
	if(Stack_twin.start!=STATISTICS_TWDEFAULT.start)
		printf("Stack_twin_start  %8.2f\n",Stack_twin.start);
	if(Stack_twin.end!=STATISTICS_TWDEFAULT.end)
		printf("Stack_twin_end  %8.2f\n",Stack_twin.end);

	if(AmpNormalizeMethod!=STATISTICS_STRINGDEFAULT)
		printf("Amp_Normalize_Method  %s\n",AmpNormalizeMethod.c_str());
	if(ref_trace_tag!=STATISTICS_STRINGDEFAULT)
		printf("ReferenceTrace    %s\n",ref_trace_tag.c_str());
	if(CodaCA_tolerance_twin_length!=STATISTICS_DOUBLEDEFAULT)
		printf("CodaCA_tolerance_twin_length  %8.2f\n",CodaCA_tolerance_twin_length);
	if(PCoda_grow_tolerance!=STATISTICS_DOUBLEDEFAULT)
		printf("PCoda_grow_tolerance  %8.2f\n",PCoda_grow_tolerance);
	if(FA_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("FA_min  %8.2f\n",FA_range[0]);  //normalized FA
	if(FA_range[1]!=STATISTICS_DOUBLEDEFAULT) 
		printf("FA_max  %8.2f\n",FA_range[1]);
	if(LowFrequency_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("LowFrequency_min  %8.4f\n",LowFrequency_range[0]);
	if(LowFrequency_range[1]!=STATISTICS_DOUBLEDEFAULT) 
		printf("LowFrequency_max  %8.4f\n",LowFrequency_range[1]);
	if(TraceAmp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("TraceAmp_min  %8.2f\n",TraceAmp_range[0]);
	if(TraceAmp_range[1]!=STATISTICS_DOUBLEDEFAULT)
		printf("TraceAmp_max  %8.2f\n",TraceAmp_range[1]);
	if(RefXcor_min!=STATISTICS_DOUBLEDEFAULT)
		printf("RefXcor_min  %8.2f\n",RefXcor_min);
	if(StackW_min!=STATISTICS_DOUBLEDEFAULT)
		printf("StackW_min  %8.2f\n",StackW_min);
	if(DSI_min!=STATISTICS_DOUBLEDEFAULT)
		printf("DSI_min  %8.2f\n",DSI_min);
	if(QI_weights[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("SW_weight(a1)  %8.2f\n",QI_weights[0]);
	if(QI_weights[1]!=STATISTICS_DOUBLEDEFAULT)
		printf("XCC_weight(a2)  %8.2f\n",QI_weights[1]);
	if(QI_weights[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("DSI_weight(a3)  %8.2f\n",QI_weights[2]);
	if(QI_min!=STATISTICS_DOUBLEDEFAULT)
		printf("RFQI_min  %8.2f\n",QI_min);
	if(DeconNiteration_range[0]!=STATISTICS_INTDEFAULT)
		printf("DeconNiteration_min  %d\n",DeconNiteration_range[0]); 
	if(DeconNiteration_range[1]!=STATISTICS_INTDEFAULT)
		printf("DeconNiteration_max  %d\n",DeconNiteration_range[1]);
	if(DeconNspike_range[0]!=STATISTICS_INTDEFAULT)
		printf("DeconNspike_min  %d\n",DeconNspike_range[0]);
	if(DeconNspike_range[1]!=STATISTICS_INTDEFAULT)
		printf("DeconNspike_max  %d\n",DeconNspike_range[1]);
	if(DeconEpsilon_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconEpsilon_min  %8.2f\n",DeconEpsilon_range[0]);
	if(DeconEpsilon_range[1]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconEpsilon_max  %8.2f\n",DeconEpsilon_range[1]);
	if(DeconPeakamp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconPeakamp_min  %8.2f\n",DeconPeakamp_range[0]);
	if(DeconPeakamp_range[1]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconPeakamp_max  %8.2f\n",DeconPeakamp_range[1]);
	if(DeconAveramp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconAveramp_min  %8.2f\n",DeconAveramp_range[0]);
	if(DeconAveramp_range[1]!=STATISTICS_DOUBLEDEFAULT)	
		printf("DeconAveramp_max  %8.2f\n",DeconAveramp_range[1]);
	if(DeconRawsnr_range[0]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconRawsnr_min  %8.2f\n",DeconRawsnr_range[0]);
	if(DeconRawsnr_range[1]!=STATISTICS_DOUBLEDEFAULT)
		printf("DeconRawsnr_max  %8.2f\n",DeconRawsnr_range[1]);
}
void TraceEditStatistics::print_parameters(FILE * fh)
{
	if(FA_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"FA_twin_start  %8.2f\n",FA_twin.start);
	if(FA_twin.end!=STATISTICS_TWDEFAULT.end)
		fprintf(fh,"FA_twin_end  %8.2f\n",FA_twin.end);
	if(PCoda_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"PCoda_twin_start  %8.2f\n",PCoda_twin.start);
	if(PCoda_twin.end!=STATISTICS_TWDEFAULT.end) 
		fprintf(fh,"PCoda_twin_end  %8.2f\n",PCoda_twin.end);
	if(CodaCA_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"CodaCA_twin_start  %8.2f\n",CodaCA_twin.start);
	if(CodaCA_twin.end!=STATISTICS_TWDEFAULT.end) 
		fprintf(fh,"CodaCA_twin_end  %8.2f\n",CodaCA_twin.end);
	if(XCor_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"XCor_twin_start  %8.2f\n",XCor_twin.start);
	if(XCor_twin.end!=STATISTICS_TWDEFAULT.end) 
		fprintf(fh,"XCor_twin_end  %8.2f\n",XCor_twin.end);
	if(Robust_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"Robust_twin_start  %8.2f\n",Robust_twin.start);
	if(Robust_twin.end!=STATISTICS_TWDEFAULT.end) 
		fprintf(fh,"Robust_twin_end  %8.2f\n",Robust_twin.end);
	if(Stack_twin.start!=STATISTICS_TWDEFAULT.start)
		fprintf(fh,"Stack_twin_start  %8.2f\n",Stack_twin.start);
	if(Stack_twin.end!=STATISTICS_TWDEFAULT.end)
		fprintf(fh,"Stack_twin_end  %8.2f\n",Stack_twin.end);

	if(AmpNormalizeMethod!=STATISTICS_STRINGDEFAULT)
		fprintf(fh,"Amp_Normalize_Method  %s\n",AmpNormalizeMethod.c_str());
	if(ref_trace_tag!=STATISTICS_STRINGDEFAULT)
		fprintf(fh,"ReferenceTrace    %s\n",ref_trace_tag.c_str());
	if(CodaCA_tolerance_twin_length!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"CodaCA_tolerance_twin_length  %8.2f\n",CodaCA_tolerance_twin_length);
	if(PCoda_grow_tolerance!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"PCoda_grow_tolerance  %8.2f\n",PCoda_grow_tolerance);
	if(FA_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"FA_min  %8.2f\n",FA_range[0]);  //normalized FA
	if(FA_range[1]!=STATISTICS_DOUBLEDEFAULT) 
		fprintf(fh,"FA_max  %8.2f\n",FA_range[1]);
	if(LowFrequency_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"LowFrequency_min  %8.4f\n",LowFrequency_range[0]);
	if(LowFrequency_range[1]!=STATISTICS_DOUBLEDEFAULT) 
		fprintf(fh,"LowFrequency_max  %8.4f\n",LowFrequency_range[1]);
	if(TraceAmp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"TraceAmp_min  %8.2f\n",TraceAmp_range[0]);
	if(TraceAmp_range[1]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"TraceAmp_max  %8.2f\n",TraceAmp_range[1]);
	if(RefXcor_min!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"RefXcor_min  %8.2f\n",RefXcor_min);
	if(StackW_min!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"StackW_min  %8.2f\n",StackW_min);
	if(DSI_min!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DSI_min  %8.2f\n",DSI_min);
	if(QI_weights[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"SW_weight(a1)  %8.2f\n",QI_weights[0]);
	if(QI_weights[1]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"XCC_weight(a2)  %8.2f\n",QI_weights[1]);
	if(QI_weights[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DSI_weight(a3)  %8.2f\n",QI_weights[2]);
	if(QI_min!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"RFQI_min  %8.2f\n",QI_min);
		
	if(DeconNiteration_range[0]!=STATISTICS_INTDEFAULT)
		fprintf(fh,"DeconNiteration_min  %d\n",DeconNiteration_range[0]); 
	if(DeconNiteration_range[1]!=STATISTICS_INTDEFAULT)
		fprintf(fh,"DeconNiteration_max  %d\n",DeconNiteration_range[1]);
	if(DeconNspike_range[0]!=STATISTICS_INTDEFAULT)
		fprintf(fh,"DeconNspike_min  %d\n",DeconNspike_range[0]);
	if(DeconNspike_range[1]!=STATISTICS_INTDEFAULT)
		fprintf(fh,"DeconNspike_max  %d\n",DeconNspike_range[1]);
	if(DeconEpsilon_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconEpsilon_min  %8.2f\n",DeconEpsilon_range[0]);
	if(DeconEpsilon_range[1]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconEpsilon_max  %8.2f\n",DeconEpsilon_range[1]);
	if(DeconPeakamp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconPeakamp_min  %8.2f\n",DeconPeakamp_range[0]);
	if(DeconPeakamp_range[1]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconPeakamp_max  %8.2f\n",DeconPeakamp_range[1]);
	if(DeconAveramp_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconAveramp_min  %8.2f\n",DeconAveramp_range[0]);
	if(DeconAveramp_range[1]!=STATISTICS_DOUBLEDEFAULT)	
		fprintf(fh,"DeconAveramp_max  %8.2f\n",DeconAveramp_range[1]);
	if(DeconRawsnr_range[0]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconRawsnr_min  %8.2f\n",DeconRawsnr_range[0]);
	if(DeconRawsnr_range[1]!=STATISTICS_DOUBLEDEFAULT)
		fprintf(fh,"DeconRawsnr_max  %8.2f\n",DeconRawsnr_range[1]);
}
void TraceEditStatistics::report()
{
	int npars=count_parameters();
	cerr<<"<  "
		<<this->station<<"  "
		<<this->method<<"  "
		<<this->nkilled<<"  "
		<<npars<<"  >"<<endl;
	if(npars>0) this->print_parameters();
	//cerr<<endl;
}
void TraceEditStatistics::report(string fname)
{
	int npars=count_parameters();
	fh = fopen(fname.c_str(),"a");
	fprintf(fh,"<  %s  %s  %d  %d  >\n",
			station.c_str(),method.c_str(),nkilled,npars);
	if(npars>0) this->print_parameters(fh);
	//fprintf(fh,"\n");
	fclose(fh);
}

////// End of TraceEditStatistics class methods.
//////////////////////////////////////////////////////////
void TraceEditOperator::report_statistics()
{
	vector<TraceEditStatistics>::iterator iptr;
	for(iptr=statistics.begin(); iptr!=statistics.end(); ++iptr)
		iptr->report();
}
void TraceEditOperator::report_statistics(string fname)
{
	vector<TraceEditStatistics>::iterator iptr;
	for(iptr=statistics.begin(); iptr!=statistics.end(); ++iptr)
		iptr->report(fname);
}
Metadata TraceEditOperator::get_statistics_summary()
{
/* Editing Procedures/Methods
const string AUTOKILL_NegativeFA("NegativeFA");//Negative FA
const string AUTOKILL_LargeAmpTraces("LargeAmpTraces");  //Large Amplitude Traces
const string AUTOKILL_SmallFA("SmallFA");//Small FA
const string AUTOKILL_GrowingPCoda("GrowingPCoda"); //Growing PCoda
const string AUTOKILL_ClusteredArrivals("ClusteredArrivals");//Lack Zero Point
const string AUTOKILL_DeconNiteration("DeconNiteration"); 
const string AUTOKILL_DeconNspike("DeconNspike");
const string AUTOKILL_DeconEpsilon("DeconEpsilon");
const string AUTOKILL_DeconPeakamp("DeconPeakamp");
const string AUTOKILL_DeconAveramp("DeconAveramp");
const string AUTOKILL_DeconRawsnr("DeconRawsnr");
const string AUTOKILL_StackWeightCutoff("StackWeightCutoff");
const string AUTOKILL_RefXcorCutoff("RefXcorCutoff");
const string AUTOKILL_RFQICutoff("QualityIndexCutoff");
const string ManualRestore("ManualRestore");
*/
	Metadata result;
	int aknfa=0, aklat=0, aksfa=0, akgpc=0, akca=0, aklfc=0;
	int klsw=0, klxcor=0, kldsi=0, klrfqi=0,kdnitn=0, kdnspike=0;
	int kdepsilon=0,kdpkamp=0,kdavamp=0, kdsnr=0, nrestored=0;
	int total_nkilled=0;
	string sta="-";
	if(statistics.size()>0)
	{
		vector<TraceEditStatistics>::iterator iptr;
		for(iptr=statistics.begin(); iptr!=statistics.end(); ++iptr)
		{
			if(iptr->method==AUTOKILL_NegativeFA)
				aknfa+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_LargeAmpTraces)
				aklat+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_SmallFA)
				aksfa+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_GrowingPCoda)
				akgpc+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_ClusteredArrivals)
				akca+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_LowFrequencyContaminated)
				aklfc+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_StackWeightCutoff)
				klsw+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DSICutoff)
				kldsi+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_RefXcorCutoff)
				klxcor+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_RFQICutoff)
				klrfqi+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconNiteration)
				kdnitn+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconNspike)
				kdnspike+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconEpsilon)
				kdepsilon+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconPeakamp)
				kdpkamp+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconAveramp)
				kdavamp+=iptr->nkilled;
			else if(iptr->method==AUTOKILL_DeconRawsnr)
				kdsnr+=iptr->nkilled;
			else if(iptr->method==ManualRestore)
				nrestored+=abs(iptr->nkilled);
			else
				cerr<<"CAUTION: unrecognized editing method [ "
					<<iptr->method<<" ] in statistics!"<<endl;
		}
		sta=statistics[0].station;
	}
	total_nkilled=aknfa+aklat+aksfa+akgpc+akca+aklfc+
			klsw+klxcor+kldsi+klrfqi+kdnitn+kdnspike+kdepsilon+
			kdpkamp+kdavamp+ kdsnr-nrestored;
	result.put("sta",sta);
	result.put(TOTAL_NKILLED_AUTO,total_nkilled);
	result.put(AUTOKILL_NegativeFA,aknfa);
	result.put(AUTOKILL_LargeAmpTraces,aklat);
	result.put(AUTOKILL_SmallFA,aksfa);
	result.put(AUTOKILL_GrowingPCoda,akgpc);
	result.put(AUTOKILL_ClusteredArrivals,akca);
	result.put(AUTOKILL_LowFrequencyContaminated,aklfc);
	result.put(AUTOKILL_StackWeightCutoff,klsw);
	result.put(AUTOKILL_DSICutoff,kldsi);
	result.put(AUTOKILL_RefXcorCutoff,klxcor);
	result.put(AUTOKILL_RFQICutoff,klrfqi);
	result.put(AUTOKILL_DeconNiteration,kdnitn);
	result.put(AUTOKILL_DeconNspike,kdnspike);
	result.put(AUTOKILL_DeconEpsilon,kdepsilon);
	result.put(AUTOKILL_DeconPeakamp,kdpkamp);
	result.put(AUTOKILL_DeconAveramp,kdavamp);
	result.put(AUTOKILL_DeconRawsnr,kdsnr);
	result.put(ManualRestore,nrestored);
	
	//DEBUG
	//cout<<"in statistics, nkilled_sw ="<<klsw<<", total="<<total_nkilled<<endl;
	
	return(result);
}
//do sparse convolution. same as in SEISPP but here we process only timeseries data.
TimeSeries TraceEditOperator::sparse_convolve(TimeSeries& wavelet, TimeSeries& d)
{
	
	if( (wavelet.tref==absolute) || (d.tref==absolute) )
		throw SeisppError(string("Error (convolve procedure): ")
			+ "both functions to be convolved must have "
			+ "relative time base");
	
	TimeSeries result(d);
	int nw=wavelet.ns;
	double *wptr;
	wptr=&(wavelet.s[0]);
	// Add a generous padding for out3c
	int nsout=d.ns+2*nw;

	result.s=vector<double>(nsout,0.0);
	//std::fill(result.s.begin(), result.s.end(), 0.0);
	//result.s.zero();
	result.t0=d.t0-(result.dt)*static_cast<double>(wavelet.ns);
	result.ns=nsout;
	// oi is the position of the moving index position in out3c 
	int oi=result.sample_number(d.t0);
	//si is the index to the point where the wavelet is to be inserted. offset by 0 of wavelet//
	int si=oi-wavelet.sample_number(0.0);
	if(si<0) throw SeisppError("Error computed result timeseries index is less than 0 ");
	//Intentionally do not check for stray indices as padding above
	//   should guarantee no pointers fly outside the bounds of the data.
	int i;
	
	for(i=0;i<d.ns;++i,++si){
		double *sptr=&(result.s[si]);
		double *dptr=&(d.s[i]);
		if((*dptr)!=0.0){
			daxpy(nw,(*dptr),wptr,1,sptr,1);
		}
	}
	
	return result;
}
void TraceEditOperator::convolve_ensemble(TimeSeries& wavelet,
			TimeSeriesEnsemble& tse, bool trimdata,TimeWindow *twin)
{
	vector<TimeSeries>::iterator im;
	TimeWindow trimwindow;
	if(trimdata)
	{
		if(twin==NULL)
		{
			cerr<<"Warning: Trim time window not defined. Use common time window by default."<<endl;
			trimwindow=find_common_timewindow(tse);
		}
		else trimwindow=*twin;
	}
	try{
		for(im=tse.member.begin();im!=tse.member.end();++im)
		{
			TimeSeries tempts=this->sparse_convolve(wavelet, *im);
			if(!trimdata)*im=tempts;
			else *im=trim_data(tempts,trimwindow);
			tempts.s.clear();
		}
	}catch(...){throw;};
}
void TraceEditOperator::convolve_ensemble(TimeSeries& wavelet,
			ThreeComponentEnsemble& tce, bool trimdata,TimeWindow *twin)
{
	vector<ThreeComponentSeismogram>::iterator im;
	TimeWindow trimwindow;
	if(trimdata)
	{
		if(twin==NULL)
		{
			cerr<<"Warning: Trim time window not defined. Use common time window by default."<<endl;
			trimwindow=find_common_timewindow(tce);
		}
		else trimwindow=*twin;
	}
	try{
		for(im=tce.member.begin();im!=tce.member.end();++im)
		{
			ThreeComponentSeismogram temp3c=SEISPP::sparse_convolve(wavelet, *im);
			if(!trimdata)*im=temp3c;
			else
			{
				*im=trim_data(temp3c,trimwindow);
			}
		}
	}catch(...){throw;};
}
//reverse order
vector<int> TraceEditOperator::reverse_order(TimeSeriesEnsemble& tse)
{
	try
	{
		vector<int> result;
		TimeSeriesEnsemble tse_tmp(tse);
		vector<TimeSeries>::iterator iptr;
		int i;
		for(i=tse.member.size()-1,iptr=tse_tmp.member.begin();
				iptr!=tse_tmp.member.end();++iptr,--i)
		{
			tse.member[i]=*iptr;
		}
		
		result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
            /*
            cout<<"order= "<<i<<", evid= "<<im
            <<", xcorcoe = "<<iptr->get_double(xcorcoekey)<<endl;
            */
        }
    	return result;	
    }catch(...){throw;};
}
// extract and print out metadata for given trace (with evid).
bool TraceEditOperator::show_metadata(TimeSeriesEnsemble& tse, long evid, bool use_decon)
{
	bool found_trace(false);
	try
	{
		TimeSeries ts_tmp;
		vector<TimeSeries>::iterator iptr;
		int i,trn;
		for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++i,++iptr)
			if(iptr->get_int(evidkey)==evid) {ts_tmp=*iptr; trn=i;found_trace=true;break;}
		if(!found_trace)
		{cerr<<"ERROR in show_metadata(): trace not found."<<endl; return found_trace;}
		int niteration(-1),nspike(-1);
		double stw(-9999.0), rtxc(-9999.0),epsilon(-9999.0),
				peakamp(-9999.0),averamp(-9999.0),rawsnr(-9999.0);
		if(ts_tmp.is_attribute_set(SEISPP::stack_weight_keyword))
			stw=ts_tmp.get_double(SEISPP::stack_weight_keyword);
		else
			cerr<<"Warning: stack weight keyword not set!"<<endl;
		if(ts_tmp.is_attribute_set(xcorcoekey))
			rtxc=ts_tmp.get_double(xcorcoekey);
		else
			cerr<<"Warning: ref trace xcorcoe keyword not set!"<<endl;
		if(use_decon)
		{
			try{
				niteration=ts_tmp.get_int(decon_niteration_key);
				nspike=ts_tmp.get_int(decon_nspike_key);
				epsilon=ts_tmp.get_double(decon_epsilon_key);
				peakamp=ts_tmp.get_double(decon_peakamp_key);
				averamp=ts_tmp.get_double(decon_averamp_key);
				rawsnr=ts_tmp.get_double(decon_rawsnr_key);
			}catch(...){cerr<<"Warning: error in extracting decon parameters!"<<endl;};
		}
		cerr<<endl
			<<"******* Trace Metadata *******"<<endl
			<<"Station       : "<<ts_tmp.get_string("sta")<<endl
			<<"Start time    : "<<strtime(ts_tmp.get_double("time"))<<endl
			<<"Trace number  : "<<trn+1<<endl
			<<"evid          : "<<ts_tmp.get_int(evidkey)<<endl;
		
		if(ts_tmp.is_attribute_set(magnitude_key))
		{cerr<<"magnitude     : "<<ts_tmp.get_double(magnitude_key)<<endl;}
		if(ts_tmp.is_attribute_set(magtype_key))
		{cerr<<"magtype       : "<<ts_tmp.get_string(magtype_key)<<endl;}
		
		//debug
		if(ts_tmp.is_attribute_set(seaz_key))
		{cerr<<"SEAZ:         : "<<ts_tmp.get_double(seaz_key)<<endl;}
		cerr<<"Stack weight  : "<<stw<<endl
			<<"RT XcorCoe    : "<<rtxc<<endl;
		if(ts_tmp.is_attribute_set(killed_trace_key))
			if(ts_tmp.get_bool(killed_trace_key))
			{	
				if(ts_tmp.is_attribute_set(killmethodkey))
					cerr<<"Killed by     : "<<ts_tmp.get_string(killmethodkey)<<endl;
				else
					cerr<<"Killed by     : Manual Editing"<<endl;
			}
		if(ts_tmp.is_attribute_set(RF_quality_index_key))
			cerr<<"Quality Index : "<<ts_tmp.get_double(RF_quality_index_key)<<endl;
		if(use_decon)
		{	
			if(ts_tmp.is_attribute_set(decon_success_index_key))
				cerr<<"Success Index : "<<ts_tmp.get_double(decon_success_index_key)<<endl;
			else
			{
            	cerr<<"Success Index : "<<compute_decon_success_index(ts_tmp)<<endl;
			}
			cerr<<"< Decon Parameters > "<<endl
			<<"  niteration  : "<<niteration<<endl
			<<"  nspike      : "<<nspike<<endl
			<<"  epsilon     : "<<epsilon<<endl
			<<"  peakamp     : "<<peakamp<<endl
			<<"  averamp     : "<<averamp<<endl
			<<"  rawsnr      : "<<rawsnr<<endl;
		}
	}catch(SeisppError& serr)
	{
		serr.what();
	}
	return found_trace;
}
// extract and save metadata for given trace (with evid) to FILE handle fh.
//mdversion is set to 1.
// void TraceEditOperator::save_metadata(TimeSeries& ts, FILE * fh, bool use_decon)
// {
// 	try
// 	{
// 		int i,nhdrs(0),mdversion(1);
// 		int niteration(-99),nspike(-99);
// 		double stw(-9999.0), rtxc(-9999.0),epsilon(-9999.0),
// 				peakamp(-9999.0),averamp(-9999.0),rawsnr(-9999.0),
// 				rfqi(-9999.0),dsi(-9999.0);
// 		if(ts.is_attribute_set(SEISPP::stack_weight_keyword))
// 			stw=ts.get_double(SEISPP::stack_weight_keyword);
// 		else
// 			cerr<<"Warning: stack weight keyword not set! Save default."<<endl;
// 		if(ts.is_attribute_set(xcorcoekey))
// 			rtxc=ts.get_double(xcorcoekey);
// 		else
// 			cerr<<"Warning: ref trace xcorcoe keyword not set! Save default."<<endl;
// 		if(use_decon)
// 		{
// 			if(ts.is_attribute_set(decon_success_index_key))
// 				dsi=ts.get_double(decon_success_index_key);
// 			else
// 			{
//             	dsi=compute_decon_success_index(ts);
// 			}
// 			try{
// 				niteration=ts.get_int(decon_niteration_key);
// 				nspike=ts.get_int(decon_nspike_key);
// 				epsilon=ts.get_double(decon_epsilon_key);
// 				peakamp=ts.get_double(decon_peakamp_key);
// 				averamp=ts.get_double(decon_averamp_key);
// 				rawsnr=ts.get_double(decon_rawsnr_key);
// 			}catch(...){cerr<<"Warning: error in extracting decon parameters!"<<endl;};
// 		}
// 		/*
// 		string killedmethod("-");
// 		if(ts.is_attribute_set(killed_trace_key))
// 			if(ts.get_bool(killed_trace_key))
// 			{	
// 				if(ts.is_attribute_set(killmethodkey))
// 					killedmethod=ts.get_string(killmethodkey);
// 			}
// 		*/
// 		if(ts.is_attribute_set(RF_quality_index_key))
// 			rfqi=ts.get_double(RF_quality_index_key);
// 		string sta=ts.get_string("sta");
// 		string start_time=strtime(ts.get_double("time"));
// 		int evid=ts.get_int(evidkey);
// 		double t0=ts.t0;
// 		double dt=ts.dt;
// 		int nsamp=ts.get_int("nsamp");
// 		
// 		//save to fh.
// 		fprintf(fh,"station            : %s\n",sta.c_str());
// 		fprintf(fh,"start_time         : %s\n",start_time.c_str());
// 		fprintf(fh,"evid               : %10d\n",evid);
// 		fprintf(fh,"samples            : %10d\n",nsamp);
// 		fprintf(fh,"dt                 : %10.6f\n",dt);
// 		fprintf(fh,"t0                 : %10.6f\n",t0);
// 		fprintf(fh,"stack_weight       : %7.4f\n",stw);
// 		fprintf(fh,"RT_xcorcoe         : %7.4f\n",rtxc);
// 		fprintf(fh,"RFQualityIndex     : %7.4f\n",rfqi);
// 		fprintf(fh,"DeconSuccessIndex  : %7.4f\n",dsi);
// 		fprintf(fh,"niteration         : %10d\n",niteration);
// 		fprintf(fh,"nspike             : %10d\n",nspike);
// 		fprintf(fh,"epsilon            : %10.5f\n",epsilon);
// 		fprintf(fh,"peakamp            : %10.5f\n",peakamp);
// 		fprintf(fh,"averamp            : %10.5f\n",averamp);
// 		fprintf(fh,"rawsnr             : %10.5f\n",rawsnr);
// 	}catch(SeisppError& serr)
// 	{
// 		serr.what();
// 	}
// }
// extract and save metadata for given trace (with evid) to FILE handle fh.
// mdversion: metadata version, currently supports 1, 2.
void TraceEditOperator::save_metadata(TimeSeries& ts, FILE * fh, bool use_decon, int mdversion)
{
	try
	{
		int i,nhdrs(0);
		int niteration(-99),nspike(-99);
		double stw(-9999.0), rtxc(-9999.0),epsilon(-9999.0),
				peakamp(-9999.0),averamp(-9999.0),rawsnr(-9999.0),
				rfqi(-9999.0),dsi(-9999.0),magnitude(-9999.0);
		string magtype("-");
		if(ts.is_attribute_set(SEISPP::stack_weight_keyword))
			stw=ts.get_double(SEISPP::stack_weight_keyword);
		else
			cerr<<"Warning: stack weight keyword not set! Save default."<<endl;
		if(ts.is_attribute_set(xcorcoekey))
			rtxc=ts.get_double(xcorcoekey);
		else
			cerr<<"Warning: ref trace xcorcoe keyword not set! Save default."<<endl;
		if(use_decon)
		{
			if(ts.is_attribute_set(decon_success_index_key))
				dsi=ts.get_double(decon_success_index_key);
			else
			{
            	dsi=compute_decon_success_index(ts);
			}
			try{
				niteration=ts.get_int(decon_niteration_key);
				nspike=ts.get_int(decon_nspike_key);
				epsilon=ts.get_double(decon_epsilon_key);
				peakamp=ts.get_double(decon_peakamp_key);
				averamp=ts.get_double(decon_averamp_key);
				rawsnr=ts.get_double(decon_rawsnr_key);
			}catch(...){cerr<<"Warning: error in extracting decon parameters!"<<endl;};
		}
		/*
		string killedmethod("-");
		if(ts.is_attribute_set(killed_trace_key))
			if(ts.get_bool(killed_trace_key))
			{	
				if(ts.is_attribute_set(killmethodkey))
					killedmethod=ts.get_string(killmethodkey);
			}
		*/
		if(ts.is_attribute_set(RF_quality_index_key))
			rfqi=ts.get_double(RF_quality_index_key);
		string sta=ts.get_string("sta");
		string start_time=strtime(ts.get_double("time"));
		int evid=ts.get_int(evidkey);
		double t0=ts.t0;
		double dt=ts.dt;
		int nsamp=ts.get_int("nsamp");
		
		if(ts.is_attribute_set(magnitude_key)) magnitude=ts.get_double(magnitude_key);
		if(ts.is_attribute_set(magtype_key)) magtype=ts.get_string(magtype_key);
		
		//save to fh.
		switch(mdversion)
		{
			case 1:
				fprintf(fh,"%%Metadata version %d\n",mdversion);
				fprintf(fh,"station            : %s\n",sta.c_str());
				fprintf(fh,"start_time         : %s\n",start_time.c_str());
				fprintf(fh,"evid               : %10d\n",evid);
				fprintf(fh,"samples            : %10d\n",nsamp);
				fprintf(fh,"dt                 : %10.6f\n",dt);
				fprintf(fh,"t0                 : %10.6f\n",t0);
				fprintf(fh,"stack_weight       : %7.4f\n",stw);
				fprintf(fh,"RT_xcorcoe         : %7.4f\n",rtxc);
				fprintf(fh,"RFQualityIndex     : %7.4f\n",rfqi);
				fprintf(fh,"DeconSuccessIndex  : %7.4f\n",dsi);
				fprintf(fh,"niteration         : %10d\n",niteration);
				fprintf(fh,"nspike             : %10d\n",nspike);
				fprintf(fh,"epsilon            : %10.5f\n",epsilon);
				fprintf(fh,"peakamp            : %10.5f\n",peakamp);
				fprintf(fh,"averamp            : %10.5f\n",averamp);
				fprintf(fh,"rawsnr             : %10.5f\n",rawsnr);
				break;
			case 2:
				fprintf(fh,"%%Metadata version %d\n",mdversion);
				fprintf(fh,"station            : %s\n",sta.c_str());
				fprintf(fh,"start_time         : %s\n",start_time.c_str());
				fprintf(fh,"evid               : %10d\n",evid);
				fprintf(fh,"magnitude          : %6.2f\n",magnitude);
				fprintf(fh,"magtype            : %s\n",magtype.c_str());
				fprintf(fh,"samples            : %10d\n",nsamp);
				fprintf(fh,"dt                 : %10.6f\n",dt);
				fprintf(fh,"t0                 : %10.6f\n",t0);
				fprintf(fh,"stack_weight       : %7.4f\n",stw);
				fprintf(fh,"RT_xcorcoe         : %7.4f\n",rtxc);
				fprintf(fh,"RFQualityIndex     : %7.4f\n",rfqi);
				fprintf(fh,"DeconSuccessIndex  : %7.4f\n",dsi);
				fprintf(fh,"niteration         : %10d\n",niteration);
				fprintf(fh,"nspike             : %10d\n",nspike);
				fprintf(fh,"epsilon            : %10.5f\n",epsilon);
				fprintf(fh,"peakamp            : %10.5f\n",peakamp);
				fprintf(fh,"averamp            : %10.5f\n",averamp);
				fprintf(fh,"rawsnr             : %10.5f\n",rawsnr);
				break;
			default:
				cerr<<"ERROR in save_metadata(). Unrecoganized medata version speficier! Use 1 or 2."<<endl;
		}
	}catch(SeisppError& serr)
	{
		serr.what();
	}
}
// extract and print out metadata for ALL traces (with given attribute).
bool TraceEditOperator::show_metadata(TimeSeriesEnsemble& tse, string mdtag,MDtype mdt)
{
	bool flag(false);
	try
	{
		TimeSeries ts_tmp;
		const string errorbase("show_metadata() for all traces: ");
		vector<TimeSeries>::iterator iptr;
		int i,trn;
		int evid;
		for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++i,++iptr)
		{
			evid=iptr->get_int(evidkey);
			trn=i+1;
			if(!iptr->is_attribute_set(mdtag))
			{
				cerr<<errorbase<<endl
					<<"attribute not set or wrong attribute name."<<endl;
				flag=false;
				break;
			}
			else
			{
				if(i==0)
					printf("number  evid  %s\n",mdtag.c_str());
				string stmp;
				switch(mdt)
				{
					case MDint:
						printf("%8d  %8d  %8d\n",trn,evid,iptr->get_int(mdtag));
						break;
					case MDreal:
						printf("%8d  %8d  %10.4f\n",trn,evid,iptr->get_double(mdtag));
						break;
					case MDstring:
						stmp=iptr->get_string(mdtag);
						printf("%8d  %8d  %s\n",trn,evid,stmp.c_str());
						break;
					case MDboolean:
						if(iptr->get_bool(mdtag))
							printf("%8d  %8d  live\n",trn,evid);
						else
							printf("%8d  %8d  dead\n",trn,evid);
						break;
					default:
						cerr<<errorbase<<"unknown medatadata type."<<endl;
						flag=false;
						return flag;
				}
			}
		}
	}catch(SeisppError& serr)
	{
		serr.what();
	}
	return flag;
}
//look for first arrival amplitude of a TimeSeries object. returns 
//the point where derivative of amplitude changes sings (peak or trough).
//FA_amp: pointer to FA amplitude; this routine returns the time location of the First Arrival.
double find_FirstArrival(TimeSeries& ts, double FA_sensitivity)
{
	try{
		//int ns_start,ns_end;
		double amplitude(0.0);
		double FA_amplitude(0.0), FA_time(0.0);
		for(int i=0;i<ts.ns;++i) 
		{
			double amp_tmp=ts.s[i];
			if((fabs(amplitude)<=FA_sensitivity || i==0)&&   
				fabs(amp_tmp)>FA_sensitivity)  // from 0 to non-zero or starts from non-zero
			{
				//not all RF data have pure spike first arrivals, this loop
				// take into account this issue and looks for the first peak 
				//value (doesn't have to be pure spike).
				// 
				FA_amplitude=amp_tmp;
				cout<<"i="<<i<<", time="<<ts.t0+i*ts.dt<<", s[i] = "<<FA_amplitude<<endl;
				for(int j=i;j<(ts.ns-1);++j)
				{
					cout<<"+++>  j="<<j<<", time="<<ts.t0+j*ts.dt<<",s[j] = "<<ts.s[j]<<endl;
					if((j==(ts.ns-1))||(ts.s[j]>=0.0 && FA_amplitude>=0.0 &&
							ts.s[j]>=FA_amplitude &&
							ts.s[j+1]<FA_amplitude)||
							(ts.s[j]<=0.0 && FA_amplitude<=0.0 &&
							ts.s[j]<=FA_amplitude &&
							ts.s[j+1]>FA_amplitude))
					{
						FA_amplitude=ts.s[j];
						FA_time=ts.t0+j*ts.dt;
						break;
					}
					
					else if((ts.s[j]>=0.0 && FA_amplitude>=0.0 &&
							ts.s[j]>=FA_amplitude &&
							ts.s[j+1]>FA_amplitude)||
							(ts.s[j]<=0.0 && FA_amplitude<=0.0 &&
							ts.s[j]<=FA_amplitude &&
							ts.s[j+1]<FA_amplitude))
					{
						FA_amplitude=ts.s[j+1];
						FA_time=ts.t0+(j+1)*ts.dt;
					}
				}
				break;
			}
			amplitude=ts.s[i];
		}
		//DEBUG
		//cout<<"FA = "<<FA_amplitude<<endl;
		ts.put(FA_time_key,FA_time);
		ts.put(FA_amplitude_key,FA_amplitude);
		return(FA_time);
	}
	catch(...){throw;};
}
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
			double detect_length, string data_shaping_wavelet_type)
{
	try{
		const string errorbase("ERROR in find_FirstArrival: ");
		string FA_type="";
		if(ts.endtime()-ts.t0<detect_length)
		{
			cerr<<errorbase<<"detect_length can't be greater than data length."<<endl;
			cerr<<"detect_length: "<<detect_length<<endl
				<<"data length: "<<ts.endtime()-ts.t0<<endl;
			return FA_type;
		}
		//int ns_start,ns_end;
		double amplitude(0.0);
		double FA_amplitude(0.0), FA_time(0.0);
		for(int i=0;i<ts.ns;++i) 
		{
			double amp_tmp=ts.s[i];
			if((fabs(amplitude)<=FA_sensitivity || i==0)&&   
				fabs(amp_tmp)>FA_sensitivity)  // from 0 to non-zero or starts from non-zero
			{
				FA_amplitude=amp_tmp;
				FA_type="BEGIN";
				FA_time=ts.t0+i*ts.dt;
				
				int ns_end=i+SEISPP::nint(detect_length/ts.dt);
				if(ns_end>ts.ns) ns_end=ts.ns;
				//not all RF data have pure spike first arrivals, this loop
				// take into account this issue and looks for the first peak 
				//value (doesn't have to be pure spike).
				// 
				//cout<<"i="<<i<<", time="<<ts.t0+i*ts.dt<<", s[i] = "<<FA_amplitude<<endl;
				if(data_shaping_wavelet_type=="RICKER")
				{
					for(int j=i;j<(ns_end);++j)
					{
						//cout<<"+++>  j="<<j<<", time="<<ts.t0+j*ts.dt<<",s[j] = "<<ts.s[j]<<endl;
					
						if(fabs(ts.s[j])>fabs(FA_amplitude))
						{
							FA_amplitude=ts.s[j];
							FA_time=ts.t0+j*ts.dt;
						}
					}
					if(FA_time==ts.s[ns_end-1]) FA_type="END";
					else if(FA_time==ts.s[i]) FA_type="BEGIN";
					else if(FA_amplitude>MYZERO) FA_type="PEAK";
					else if(FA_amplitude<-1*MYZERO) FA_type="TROUGH";
				}
				else if(data_shaping_wavelet_type=="GAUSSIAN")
				{
					for(int j=i;j<(ns_end);++j)
					{
						//cout<<"+++>  j="<<j<<", time="<<ts.t0+j*ts.dt<<",s[j] = "<<ts.s[j]<<endl;
						if(j==(ns_end-1))
						{
							FA_type="END";
							FA_amplitude=ts.s[j];
							FA_time=ts.t0+j*ts.dt;
							break;
						}
						else if(ts.s[j]>=0.0 && FA_amplitude>=0.0 &&
								ts.s[j]>=FA_amplitude &&
								ts.s[j+1]<FA_amplitude)
						{
							FA_type="PEAK";
							FA_amplitude=ts.s[j];
							FA_time=ts.t0+j*ts.dt;
							break;
						}
						else if(ts.s[j]<=0.0 && FA_amplitude<=0.0 &&
								ts.s[j]<=FA_amplitude &&
								ts.s[j+1]>FA_amplitude)
						{
							FA_type="TROUGH";
							FA_amplitude=ts.s[j];
							FA_time=ts.t0+j*ts.dt;
							break;
						}
						else if((ts.s[j]>=0.0 && FA_amplitude>=0.0 &&
								ts.s[j]>=FA_amplitude &&
								ts.s[j+1]>FA_amplitude)||
								(ts.s[j]<=0.0 && FA_amplitude<=0.0 &&
								ts.s[j]<=FA_amplitude &&
								ts.s[j+1]<FA_amplitude))
						{
							FA_amplitude=ts.s[j+1];
							FA_time=ts.t0+(j+1)*ts.dt;
						}
					}
				}
				else if(data_shaping_wavelet_type=="SPIKE")
				{
					FA_type="SPIKE";
					break;
				}
				break;
			}
			amplitude=ts.s[i];
		}
		ts.put(FA_time_key,FA_time);
		ts.put(FA_amplitude_key,FA_amplitude);
		//DEBUG
		//cout<<"FA = "<<FA_amplitude<<endl;
		//*FA_amp=FA_amplitude;
		return(FA_type);
	}
	catch(...){throw;};
}
//find global max absolute FA values from all traces.
//
double find_max_abs_FirstArrival(TimeSeriesEnsemble& tse, double FA_sensitivity,
		double detect_length, string data_shaping_wavelet_type)
{
	try
	{	
		double vmax(0.0);
		int i,count,vsize;
		double FA_amplitude,FA_time;
		vector<TimeSeries>::iterator iptr;
		for(count=0,iptr=tse.member.begin();iptr!=tse.member.end();++count,++iptr)
        {
            if(iptr->live)
            {
				find_FirstArrival(*iptr,FA_sensitivity,detect_length,
						data_shaping_wavelet_type);
				//FA_time=tmpts.get_double(FA_time_key);
				FA_amplitude=iptr->get_double(FA_amplitude_key);
				if(fabs(FA_amplitude)>fabs(vmax)) 
					vmax=fabs(FA_amplitude);
			}
        }
    	return(vmax);
    }catch(...){throw;};
}

//find global max absolute amplitude values from all traces.
//
double find_max_abs_amplitude(TimeSeriesEnsemble& tse)
{
	try
	{	
		double vmax(0.0);
		int i,count,vsize;
		double FA_amplitude,FA_time;
		vector<TimeSeries>::iterator iptr;
		for(count=0,iptr=tse.member.begin();iptr!=tse.member.end();++count,++iptr)
        {
            if(iptr->live)
            {
            	for(i=0; i<iptr->ns; ++i)
            	{
					if(fabs(iptr->s[i])>fabs(vmax)) 
						vmax=fabs(iptr->s[i]);
				}
			}
        }
    	return(vmax);
    }catch(...){throw;};
}

//find false traces and returns the evid list of those traces.
set<long> TraceEditOperator::find_false_traces(TimeSeriesEnsemble& tse)
{
	try
	{
		long evid=-1;
		set<long> evids_killed;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			if(!iptr->live )
			{
				evid=iptr->get_long(evidkey);
				evids_killed.insert(evid);
			}
		}
		return(evids_killed);
	}catch(...){throw;};
}
TimeWindow TraceEditOperator::find_common_timewindow(TimeSeriesEnsemble& tse)
{
	try
		{
		TimeWindow tw;
		tw=TimeWindow(tse.member[0].t0,tse.member[0].endtime());
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			if(iptr->t0>tw.start) tw.start=iptr->t0;
			if(iptr->endtime()<tw.end) tw.end=iptr->endtime();
		}
		return tw;
	}catch(...){throw;};
}
TimeWindow TraceEditOperator::find_common_timewindow(ThreeComponentEnsemble& tce)
{
	try
		{
		TimeWindow tw;
		tw=TimeWindow(tce.member[0].t0,tce.member[0].endtime());
		vector<ThreeComponentSeismogram>::iterator iptr;
		for(iptr=tce.member.begin();iptr!=tce.member.end();++iptr)
		{
			if(iptr->t0>tw.start) tw.start=iptr->t0;
			if(iptr->endtime()<tw.end) tw.end=iptr->endtime();
		}
		return tw;
	}catch(...){throw;};
}
/*
compute fft forward transform using gsl function: gsl_fft_real_transform()
The core codes of this routine are modified from: 
https://www.gnu.org/software/gsl/manual/gsl-ref_16.html
*/
//*****Commented out for now ********
/*
vector<double> compute_fft_real_transform(vector<double> data)
{
	vector<double> result;
	
	int i,n=data.size();
	
	double d[n];
	result.resize(n);
	//may not be the best way. simply copy the vector to the double array.
	for(i=0;i<n;++i) d[i]=data[i];
	
	gsl_fft_real_wavetable * real;
  	//gsl_fft_halfcomplex_wavetable * hc;
  	gsl_fft_real_workspace * work;
  	
  	work = gsl_fft_real_workspace_alloc (n);
  	real = gsl_fft_real_wavetable_alloc (n);
	
	//DEBUG
	
	//for (i = 0; i < n; i++)
    //{
    //  printf ("%d: %e\n", i, d[i]);
    //}
    //printf ("\n");
  	gsl_fft_real_transform (d, 1, n,real, work);
  	gsl_fft_real_wavetable_free (real);
  	//DEBUG
  	
  	//cout<<"After FFT: "<<endl;
	//for (i = 0; i < n; i++)
    //{
    //  printf ("%d: %e\n", i, abs(d[i]));
    //}
    //printf ("\n");
    
  	
	//for(i=0;i<n;++i) result[i]=d[i];
	
	//cout<<"After FFT: "<<endl;
	//for (i = 0; i < n; i++)
    //{
    //  printf ("%d: %e\n", i, fabs(result[i]));
    //}
    //printf ("\n");
    
	//return result;
//}
*/

//simply compute dot products of the of two TimeSeries traces.
//it assumes that the data have same length. this is a zerolag version.
// originally from: http://paulbourke.net/miscellaneous/correlate/
double compute_xcor_zerolag(vector<double> x, vector<double> y)
{
	const int maxdelay=0; //force delay to zero for now.
	int i,j;
	double mx,my,sx,sy,sxy,denom,r;
	int delay=0,n;
   	n=x.size();
	/* Calculate the mean of the two series x[], y[] */
	mx = 0;
	my = 0;   
	for (i=0;i<n;i++) {
		mx += x[i];
		my += y[i];
	}
	mx /= n;
	my /= n;

	/* Calculate the denominator */
	sx = 0;
	sy = 0;
	for (i=0;i<n;i++) {
		sx += (x[i] - mx) * (x[i] - mx);
		sy += (y[i] - my) * (y[i] - my);
	}
	denom = sqrt(fabs(sx*sy)+MYZERO);

	/* Calculate the correlation series */
	//for (delay=-maxdelay;delay<maxdelay;delay++) 
	//{
		sxy = 0;
		for (i=0;i<n;i++) {
			//j = i + delay;
			/*
			if (j < 0 || j >= n)
				continue;
			else
			*/
			sxy += (x[i] - mx) * (y[i] - my);
			/* Or should it be (?)
			if (j < 0 || j >= n)
			sxy += (x[i] - mx) * (-my);
			else
			sxy += (x[i] - mx) * (y[j] - my);
			*/
		}
		r = sxy / denom;

	  /* r is the correlation coefficient at "delay" */

	//}
	return r;
}
bool TraceEditOperator::compute_trace_xcorcoe
			(TimeSeriesEnsemble& tse, int ref_evid, TimeWindow xcor_twin)
{
	try
	{	
		const string base_message("compute_trace_xcorcoe: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		bool found_ref_trace(false);
		TimeSeries ref_trace;
		bool success(false);
		vector<TimeSeries>::iterator iptr;

		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
        	if(iptr->live)
        	{
        		revid=iptr->get_int(evidkey);
        		if(revid==ref_evid)
        		{
        			ref_trace=*iptr;
        			found_ref_trace=true;
        			break;
        		}
        	}
        }
		
		if(!found_ref_trace)
		{
			cerr<<base_message<<"can't find reference trace with given evid."<<endl;
			//for debugging. we exit the program.
			success=false;
			return success; //return empty set.
		}
		//check if xcor_twin is bigger than the actual ref_trace length.
		//it is the caller's responsibility to check this but better to do it here.
		if(xcor_twin.start<ref_trace.t0 || xcor_twin.end>ref_trace.endtime())
		{
			cerr<<base_message<<"error with xcor time window bigger than reference trace."<<endl;
			success=false;
			return success;
		}
		int nstart, nend,evid;
		nstart=xcor_twin.start/ref_trace.dt;
		nend=xcor_twin.end/ref_trace.dt;
		int refsize=nend-nstart+1;
		vector<double> ref_vector, xcor_vector;
		ref_vector.reserve(refsize);
		
		//cerr<<"ref trace: "<<endl;
		for(i=nstart;i<=nend;++i)
		{
			//debug
			//cerr<<ref_trace.s[i]<<endl;
			ref_vector.push_back(ref_trace.s[i]);
		}
		//
		//cerr<<"data trace: "<<endl;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				xcor_vector.reserve(refsize);
				for(i=nstart;i<=nend;++i)
				{
					//cerr<<iptr->s[i]<<endl;
					xcor_vector.push_back(iptr->s[i]);
				}
				vxcor=compute_xcor_zerolag(xcor_vector, ref_vector);
				iptr->put(xcorcoekey,vxcor);
				//cout<<vxcor<<endl;
				xcor_vector.clear();
			}
			
			//debug
			//exit(-1);
        }
        ref_vector.clear();
        success=true;
        return(success);
    }catch(...){throw;};
}
bool TraceEditOperator::compute_trace_xcorcoe
			(TimeSeriesEnsemble& tse, TimeSeries& ref_trace, TimeWindow xcor_twin)
{
	bool success(false);
	try
	{	
		const string base_message("compute_trace_xcorcoe: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		
		vector<TimeSeries>::iterator iptr;
		//check if xcor_twin is bigger than the actual ref_trace length.
		//it is the caller's responsibility to check this but better to do it here.
		if(xcor_twin.start<ref_trace.t0 || xcor_twin.end>ref_trace.endtime())
		{
			cerr<<base_message<<"error with xcor time window bigger than reference trace."<<endl;
			success=false;
			return success;
		}
		int nstart, nend,evid;
		nstart=SEISPP::nint(xcor_twin.start/ref_trace.dt);
		nend=SEISPP::nint(xcor_twin.end/ref_trace.dt);
		int refsize=nend-nstart+1;
		vector<double> ref_vector;
		TimeSeries ts_tmp=trim_data(ref_trace,xcor_twin);
		ref_vector=ts_tmp.s;
		
		//cerr<<"ref trace: "<<endl;
		/*
		for(i=nstart;i<=nend;++i)
		{
			//debug
			//cerr<<ref_trace.s[i]<<endl;
			ref_vector.push_back(ref_trace.s[i]);
		}
		*/
		//
		//cout<<"refsize="<<refsize<<endl;
		//cerr<<"data trace: "<<endl;
		/*
		for(i=0;i<refsize;++i)
		{
			//debug
			//cerr<<ref_trace.s[i]<<endl;
			xcor_vector.push_back(0.0);
		}*/
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {

			//cerr<<"xcor_vector reserved."<<endl;
            vector<double> xcor_vector;
			//xcor_vector.reserve(refsize);
			TimeSeries ts2=trim_data(*iptr,xcor_twin);
            if(iptr->live)
            {
				int j=0;
				xcor_vector=ts2.s;
				//xcor_vector.begin();
				/*
				for(i=nstart;i<(nstart+refsize);++i,++j)
				{
					//cerr<<iptr->s[i]<<endl;
					cout<<"   j="<<j<<endl;
					//xcor_vector.push_back(iptr->s[i]);
					xcor_vector[j]=iptr->s[i];
				}
				*/
				//cerr<<"xcor_vector filled. size="<<xcor_vector.size()<<endl;
				vxcor=compute_xcor_zerolag(xcor_vector, ref_vector);
				//cerr<<"xcorcoe computed. "<<endl;
				iptr->put(xcorcoekey,vxcor);
				//cerr<<"metadata set for xcorcoe. "<<endl;
				//cout<<vxcor<<endl;
			}
			ts2.s.clear();
			xcor_vector.clear();
			//cerr<<"xcor_vector cleared. "<<endl;
			//debug
			//exit(-1);
        }
        ts_tmp.s.clear();
        
        ref_vector.clear();
        success=true;
        
    }catch(SeisppError& serr)
    {cerr<<"Error in compute_trace_xcorcoe: "<<endl<<serr.what()<<endl;};
    return(success);
}
// this is used to sort the data by correlation with the reference trace with ref_evid.
// returns the rank/order of all of the traces.
vector<int> TraceEditOperator::sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, int ref_evid)
{
	try
	{	
		const string base_message("sort_by_less_xcorcoef: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		bool flag(true);
		TimeSeries ref_trace;
		vector<int> result;
		vector<TimeSeries>::iterator iptr;
		TimeWindow xcor_twin=find_common_timewindow(tse);
		flag=compute_trace_xcorcoe(tse,ref_evid,xcor_twin);
		if(!flag) return result;
        //sort
        sort(tse.member.begin(),tse.member.end(),less_xcorcoe<TimeSeries>());
        
        result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
            /*
            cout<<"order= "<<i<<", evid= "<<im
            <<", xcorcoe = "<<iptr->get_double(xcorcoekey)<<endl;
            */
        }
        tse.put(sort_method_key,(char *)"xcorcoe_ref_evid");
    	return result;
    }catch(...){throw;};
}
// this is used to sort the data by correlation with the reference trace with ref_evid.
// returns the rank/order of all of the traces. IN DEBUGGING.
vector<int> TraceEditOperator::sort_by_less_xcorcoef
			(TimeSeriesEnsemble& tse, int ref_evid, TimeWindow xcor_twin)
{
	try
	{	
		const string base_message("sort_by_less_xcorcoef: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		bool flag(true);
		TimeSeries ref_trace;
		vector<int> result;
		vector<TimeSeries>::iterator iptr;

		flag=compute_trace_xcorcoe(tse,ref_evid,xcor_twin);
		if(!flag) return result;
        //sort
        sort(tse.member.begin(),tse.member.end(),less_xcorcoe<TimeSeries>());
        
        result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
            /*
            cout<<"order= "<<i<<", evid= "<<im
            <<", xcorcoe = "<<iptr->get_double(xcorcoekey)<<endl;
            */
        }
        tse.put(sort_method_key,(char *)"xcorcoe_ref_evid");
    	return result;
    }catch(...){throw;};
}
vector<int> TraceEditOperator::sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, 
					TimeSeries& ref_trace)
{
	try
	{	
		const string base_message("sort_by_less_xcorcoef: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		vector<int> result;
		vector<TimeSeries>::iterator iptr;
		bool flag(true);
		TimeWindow xcor_twin=find_common_timewindow(tse);
		flag=compute_trace_xcorcoe(tse,ref_trace,xcor_twin);
		if(!flag) return result;
        //sort
        sort(tse.member.begin(),tse.member.end(),less_xcorcoe<TimeSeries>());
        
        result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
            /*
            cout<<"order= "<<i<<", evid= "<<im
            <<", xcorcoe = "<<iptr->get_double(xcorcoekey)<<endl;
            */
        }
        tse.put(sort_method_key,(char *)"xcorcoe_ref_trace");
    	return result;
    }catch(...){throw;};
}
vector<int> TraceEditOperator::sort_by_less_xcorcoef(TimeSeriesEnsemble& tse, 
					TimeSeries& ref_trace, TimeWindow xcor_twin)
{
	try
	{	
		const string base_message("sort_by_less_xcorcoef: ");
		double vxcor(0.0);
		int i,vsize,revid=-1;
		vector<int> result;
		vector<TimeSeries>::iterator iptr;
		bool flag(true);
		
		flag=compute_trace_xcorcoe(tse,ref_trace,xcor_twin);
		if(!flag) return result;
        //sort
        sort(tse.member.begin(),tse.member.end(),less_xcorcoe<TimeSeries>());
        
        result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
            /*
            cout<<"order= "<<i<<", evid= "<<im
            <<", xcorcoe = "<<iptr->get_double(xcorcoekey)<<endl;
            */
        }
        tse.put(sort_method_key,(char *)"xcorcoe_ref_trace");
    	return result;
    }catch(...){throw;};
}
//Sort by event magnitude extracted from trace metadata
vector<int> TraceEditOperator::sort_by_ascend_magnitude(TimeSeriesEnsemble& tse)
{
	try
	{	
		const string base_message("sort_by_magnitude: ");
		int i;
		vector<int> result;
		vector<TimeSeries>::iterator iptr;
		//put sortkey.
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			iptr->put(ensemblesortkey,magnitude_key);
		}
        //sort
        sort(tse.member.begin(),tse.member.end(),sort_less_double<TimeSeries>());
        
        result.reserve(tse.member.size());
        for(i=0,iptr=tse.member.begin();iptr!=tse.member.end();++iptr,++i)
        {
            int im;
            im=iptr->get_int(evidkey);
            result.push_back(im);
        }
        tse.put(sort_method_key,(char *)"event_magnitude");
    	return result;
    }catch(...){throw;};
}
//stw: stack timewindow; rtw: rubust stacking time window;
//d_stack: reference of the stack trace (stores the stack trace).
//returns the evid order after sorting by less stacking weight.
vector<int> TraceEditOperator::stack_and_sort(
			TimeSeriesEnsemble& d,TimeWindow twin, 
			TimeWindow rtw, StackType stacktype)
{
    try{
        vector<int> result;
        /* Silently do nothing if the ensemble is empty */
        if(d.member.size()<=0) return result;
        /* This works for RF data from EARS that all have a common
           length.  DO NOT transport this code without making this
           more general */
        //TimeWindow twin(d.member[0].t0,d.member[0].endtime());
        /* This is a convenient way to post original order to the 
           ensemble.  Used before returning to produce result vector */
        vector<TimeSeries>::iterator dptr;
        int i;
        //for(i=0,dptr=d.member.begin();dptr!=d.member.end();++dptr,++i)
        //    dptr->put("member_number",i);
        /* Always use the robust method with the stack and robust
           window the same */
    	this->get_stack(d,twin,rtw,stacktype);
        //Stack s(d,twin,rtw,stacktype); //st for stacktype. Xiaotao Yang 01/08/2015
        /* Frozen name a problem, but label this stack so clear it
           is such */
        //s.stack.put(string("sta"),stackstaname);
        /* Sort always by stack weight */
        sort(d.member.begin(),d.member.end(),less_stackwt<TimeSeries>());
        result.reserve(d.member.size());
        for(i=0,dptr=d.member.begin();dptr!=d.member.end();++dptr,++i)
        {
            int im;
            im=dptr->get_int(evidkey);
            //Debug
            /*
            cout<<"stack order: "<<i
            	<<", evid = "<<dptr->get_int(evidkey)
            	<<", stackweight = "
            	<<dptr->get_double(SEISPP::stack_weight_keyword)<<endl;
            */
            /*
            cerr<<"member "<<i
            <<", moveout= "<<dptr->get_double(SEISPP::moveout_keyword)
			<<", amplitude factor= "<<dptr->get_double(SEISPP::amplitude_static_keyword)<<endl;
			*/
            result.push_back(im);
        }
        //DEBUG
        //result=sort_by_less_xcorcoef(d, 578);

        //DEBUG
        /* push two copies of the stack to the front of the vector
           container.  Mark one dead to serve as a spacer.  This will
           make stack appear at the bottom of the plot with a gap.  This 
           is a bit dangerous and could scramble some things later so
           caution if the algorithm is later modified. */
        /*
        d.member.insert(d.member.begin(),s.stack);
        d.member.insert(d.member.begin(),s.stack);
        d.member[1].live=false;  
        d.member[1].put("sta",seperatorsta); 
        */
        d.put(sort_method_key,(char *)"robust_stack_weight");
        return result;
    } catch(...){throw;};
}
//a1: for stackweight; a2: for xcorcoe.
//a3 is weight of the decon success index.
double TraceEditOperator::compute_RF_quality_index(TimeSeries& ts, double a1, double a2, double a3)
{
	const string errorbase("ERROR in compute_RF_quality_index for timeseries: ");
	const double dpower=0.2;
	double sw(-9999.9); //normalized stack weight;
	double xc(-9999.9); //normalized correlation coefficience with reference trace.
	double dsi(-9999.9); //decon_success_index
	double qi(-9999.9); //RF quality index.
	if(a1*a2*a3<-1.0*MYZERO)
	{
		cerr<<errorbase<<endl<<"attribute weight can't be negative."<<endl;
		return qi;
	}
	if((fabs(a1)+fabs(a2)+fabs(a3))<=MYZERO)
	{
		cerr<<errorbase<<endl<<"attribute weight can't all be zero."<<endl;
		return qi;
	}
	if((fabs(a1)+fabs(a2)+fabs(a3))>1.0)
	{
		cerr<<errorbase<<endl<<"sum of attribute weights can't be greater than 1.0."<<endl;
		return qi;
	}
	if(a1>MYZERO)
	{	if(!ts.is_attribute_set(SEISPP::stack_weight_keyword))
		{
			cerr<<errorbase<<SEISPP::stack_weight_keyword<<" not set."<<endl;
			return qi;
		}
		else
			sw=ts.get_double(SEISPP::stack_weight_keyword);
	}
	
	if(a2>MYZERO)
	{	if(!ts.is_attribute_set(xcorcoekey))
		{
			cerr<<errorbase<<xcorcoekey<<" not set."<<endl;
			return qi;
		}
		else
			xc=ts.get_double(xcorcoekey);
	}
		
	if(a3>MYZERO)
	{	if(!ts.is_attribute_set(decon_success_index_key))
		{
			cerr<<errorbase<<decon_success_index_key<<" not set."<<endl;
			return qi;
		}
		else
			dsi=ts.get_double(decon_success_index_key);	
	}
	//qi=1.0/(a1/(sw*sw) + a2/(a2*xc*xc) + a3/(dsi*dsi));	
	//qi=a1*pow(sw,dpower) + a2*pow(xc,dpower) + a3*pow(dsi,dpower);
	//sw=pow(sw,1/(1-a1));
	//xc=pow(xc,1/(1-a2));
	//dsi=pow(dsi,1/(1-a3));
	if(a3>MYZERO)
	{	if(sw*xc*dsi <= MYZERO || sw<= MYZERO || xc<=MYZERO || dsi<=MYZERO) 
			qi=0.0;
		else
			qi=a1*sw + a2*xc + a3*dsi;
	}
	else
	{	if(sw*xc <= MYZERO || sw<= MYZERO || xc<=MYZERO) 
			qi=0.0;
		else
			qi=a1*sw + a2*xc;
	}
	return(qi);
}
bool TraceEditOperator::compute_RF_quality_index(TimeSeriesEnsemble& tse, 
			double a1, double a2, double a3, bool normalize)
{
	try
	{
		bool flag(true);
		vector<TimeSeries>::iterator iptr;
		//compute success rate if they are not set yet.
		double vmax=-9999.9;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			double tmp=compute_RF_quality_index(*iptr, a1, a2,a3);
			if(tmp>vmax) vmax=tmp;
			//cerr<<tmp<<endl;
			if(tmp>-9999.9) iptr->put(RF_quality_index_key,tmp);
			else
			{
				flag=false;
				return(flag);
			}
		}
		if(normalize)
			//the quality indices are normalized by the max value.
			for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
			{
				double tmp=compute_RF_quality_index(*iptr, a1, a2,a3);
				tmp=tmp/(vmax+MYZERO);
				iptr->put(RF_quality_index_key,tmp);
			}
		return(flag);
	}catch(...){throw;};
}
//the quality indices are normalized by the max value.
vector<int> TraceEditOperator::sort_by_RF_quality_index(TimeSeriesEnsemble& tse,
					double a1, double a2, double a3, bool normalize)
{
	try
	{
		const string errorbase("TraceEditOperator::sort_by_RF_quality_index(): ");
		vector<int> result;
		bool flag=compute_RF_quality_index(tse,a1,a2,a3,normalize);
		//silently return empty vector.
		//it's the users responsibility to check the size of the returned vector.
		if(!flag)
		{
			cerr<<errorbase<<"Error in computing RF Quality Index"<<endl;
			return result;
		}
		vector<TimeSeries>::iterator iptr;
		//put sortkey.
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			iptr->put(ensemblesortkey,RF_quality_index_key);
		}
		
		//start sorting.
		sort(tse.member.begin(),tse.member.end(),sort_less_double<TimeSeries>());
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
		{
			int im;
			im=iptr->get_int(evidkey);
            result.push_back(im);
		}
		tse.put(sort_method_key,(char *)"RF_quality_index");
		return result;
	}catch(...){throw;};
}
double TraceEditOperator::compute_decon_success_index(TimeSeries& ts)
{
	//double weight_epsilon=0.5;
	//double weight_spikerate=0.5;
	int nspike=ts.get_int(decon_nspike_key);
	int nitn=ts.get_int(decon_niteration_key);
	double epsilon=ts.get_double(decon_epsilon_key);
	
	//double tmp=weight_epsilon*(1.0-(epsilon/100.0))+
		//	weight_spikerate*((double)nspike/(double)nitn);
	//trace_decon use 1e-15 as threshold for zero to  count the number
	//of spikes. Theoretically, nspike should be no greater than
	// niteration. After manually check traces with nspike>niteration,
	//we found that those traces are bad. Here we just set DSI for those
	//traces as zero or MYZERO.
	double tmp;
	if(nspike>nitn) tmp=MYZERO;
	else
		tmp=(1.0-(epsilon/100.0))*((double)nspike/(double)nitn);
	
	return tmp;
}
void TraceEditOperator::compute_decon_success_index(TimeSeriesEnsemble& tse)
{
	vector<TimeSeries>::iterator iptr;
	//compute success rate if they are not set yet.
	for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
	{
		if(!iptr->is_attribute_set(decon_success_index_key))
        {
			iptr->put(decon_success_index_key,compute_decon_success_index(*iptr));
		}
	}
}
//sort by decon parameters.
vector<int> TraceEditOperator::sort_by_decon_parameters(
			TimeSeriesEnsemble& tse,string decon_par)
{
	string decon_key;
	vector<int> result;
	int deconpartype;  //0: double; 1: int;
	if(decon_par=="niteration") {decon_key=decon_niteration_key; deconpartype=1;}
	else if(decon_par=="nspike") {decon_key=decon_nspike_key;deconpartype=1;}
	else if(decon_par=="epsilon") {decon_key=decon_epsilon_key;deconpartype=0;}
	else if(decon_par=="peakamp") {decon_key=decon_peakamp_key;deconpartype=0;}
	else if(decon_par=="averamp") {decon_key=decon_averamp_key;deconpartype=0;}
	else if(decon_par=="rawsnr") {decon_key=decon_rawsnr_key;deconpartype=0;}
	else if(decon_par=="success_index")
	{
		decon_key=decon_success_index_key;
		vector<TimeSeries>::iterator iptr;
        //compute success rate if they are not set yet.
        for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(!iptr->is_attribute_set(decon_success_index_key))
            {
            	
            	//cerr<<"nspike = "<<nspike<<", nitn = "<<nitn<<", epsilon = "<<epsilon<<endl;
            	//cerr<<"index not set. compute = "<<tmp<<endl;
            	iptr->put(decon_success_index_key,compute_decon_success_index(*iptr));
            }
            //else
            //	cerr<<"index set = "<<iptr->get_double(decon_success_index_key)<<endl;
        }
        deconpartype=0;
	}
	else
	{
		cerr<<"Warning: unrecognized decon parameter: "<<decon_par<<endl<<"> ";
		return result;
	}
	try
	{
		vector<TimeSeries>::iterator dptr;
        int i;
        //put decon sortkey to the metadata.
        // insufficient to put to each trace but this is current the way the sort template works.
        for(dptr=tse.member.begin();dptr!=tse.member.end();++dptr)
        {
            dptr->put(ensemblesortkey,decon_key);
        }
        //sort by decon key.
        switch(deconpartype)
        {
			case 0:
				sort(tse.member.begin(),tse.member.end(),sort_less_double<TimeSeries>());
				break;
			case 1:
				sort(tse.member.begin(),tse.member.end(),sort_less_int<TimeSeries>());
				break;
		}
		//extract evid order.
        result.reserve(tse.member.size());
        for(i=0,dptr=tse.member.begin();dptr!=tse.member.end();++dptr,++i)
        {
            int im;
            im=dptr->get_int(evidkey);
            /*
            string stmp=dptr->get_string(ensemblesortkey);
            double v=dptr->get_double(stmp);
            cerr<<v<<endl; */
            result.push_back(im);
			//cerr<<dptr->get_double(decon_key)<<endl;
        }
        tse.put(sort_method_key,decon_key);
    	return result;
	}catch(...){throw;};
}
//get stack trace
TimeSeries TraceEditOperator::get_stack(TimeSeriesEnsemble& d,TimeWindow twin, 
					TimeWindow rtw, StackType stacktype)
{
	TimeSeries result(d.member[0]);
	result.s.clear();
	try{
        /* Silently do nothing if the ensemble is empty */
        if(d.member.size()<=0) return result;
        //manually set stack weight to 1.0 if ensemble contains only one trace.
		//otherwise, will get error when extracting stack_weight.
		//cerr<<d.member.size()<<endl;
		//exit(-1);
		if(d.member.size()==1) 
		{
			d.member[0].put(SEISPP::stack_weight_keyword,1.0);
			cerr<<"Warning in get_stack(): stack weight is "
				<<"manually set to 1.0 for single-trace ensemble!"<<endl;
			cerr<<"Warning in get_stack(): ensemble has only 1 member, which is assigned to the stacked trace."<<endl;
			return result;
		}
        Stack s(d,twin,rtw,stacktype);
        /*
        TimeSeries *initial_beam=new TimeSeries(s.stack);
        double dt=d.member[0].dt;
        //using the full data window doesn't work for MCC object.
        //don't know the exact reason but this way works by removing
        //one sample from each end of the full timewindow.
        TimeWindow twtemp(twin);
        //if(twtemp.start-d.member[0].t0<= MYZERO) twtemp.start += dt;
        if(d.member[0].endtime()-twtemp.end<= MYZERO) twtemp.end -= dt;
        //cout<<"twtemp start: "<<twtemp.start<<endl;
        //TEST FOR MultichannelCorrelator
        MultichannelCorrelator MCC(d, RobustStack, twtemp,rtw, 5.0, stacktype,
        							initial_beam,-1,true,false,false,false);
        delete initial_beam;
        */
        /*
        MultichannelCorrelator(TimeSeriesEnsemble& d,
     	      CorrelationMethod meth,
	        TimeWindow beam_window,
	          TimeWindow robust_window=TimeWindow(),
	           double lag_cutoff=5.0,
	            StackType stack_type=BasicStack,
	              TimeSeries *initial_beam=NULL,
	               int reference_member=0,
	                bool normalize=false,
	                  bool parallel=false,
	                    bool correlate_only=false,
                      	      bool freeze=false);
        */
        
		//Stack s(d,twin,rtw,stacktype); //st for stacktype. Xiaotao Yang 01/08/2015
		//Here we normalize the stackweight metadata to make it dimensionless.
		//It is inefficient to get the max and then normalize the values by
		// two for loops through all of the traces. But I don't have a better way for now.
		
		
		vector<TimeSeries>::iterator iptr;
		double vmax(MYZERO),tmp;
		int i;
		if(stacktype==BasicStack || stacktype==MedianStack) 
		{
			cerr<<"Warning: for BasicStack and MedianStack, "
				<<"all traces have the same stack weight 1.0!"<<endl;
		}
		
		for(i=0,iptr=d.member.begin();iptr!=d.member.end();++iptr,++i)
		{	
			if(stacktype==BasicStack || stacktype==MedianStack) 
			{
				iptr->put(SEISPP::stack_weight_keyword,1.0);
			}
			tmp=iptr->get_double(SEISPP::stack_weight_keyword);
			//cout<<"test 2, trace member: "<<i<<endl;
			//tmp=iptr->get_double(SEISPP::moveout_keyword);
			//DEBUG
			//cerr<<"member "<<i<<", moveout= "<<tmp
			//<<", amplitude factor= "<<iptr->get_double(SEISPP::amplitude_static_keyword)<<endl;
			if(tmp>vmax) vmax=tmp;
			//DEBUG
			//cout<<"evid="<<iptr->get_int(evidkey)<<", vmax = "<<vmax<<", tmp="<<tmp<<endl;
		}
		//only if vmax has been modified by the above loop, we do the normalization.
		if(vmax>MYZERO)
			for(iptr=d.member.begin();iptr!=d.member.end();++iptr)
			{	
				tmp=iptr->get_double(SEISPP::stack_weight_keyword);
				iptr->put(SEISPP::stack_weight_keyword,tmp/vmax);
			}
		
        /* Frozen name a problem, but label this stack so clear it
           is such */
        //TEST FOR MCC
        
        //s.stack=MCC.ArrayBeam();
        //s.stack.put(string("sta"),stackstaname);
        result=s.stack;
        /*
        result=MCC.ArrayBeam();
        result.put(string("sta"),stackstaname);
        */
        //since we may trim the stack timewindow by one sample,
        // here, we push back at the end one sample with zero amplitude.
        //This allows the stack trace be used in other functions.
        /*
        if((twin.length()-twtemp.length())/dt>MYZERO)
        {
        	result.s.push_back(0.0);
        	result.ns=result.ns + 1;
        }
        */
        return result;
    } catch(SeisppError& serr)
    {cerr<<"**ERROR in get_stack(): "<<endl;
    	//debug
    	//cerr<<"stack tw: "<<twin.start<<", "<<twin.end<<endl
    	//	<<"robust tw: "<<rtw.start<<", "<<rtw.end<<endl;
    	//exit(-1);
    	throw;
    	//end of debug	
    };  
}
//remove duplicates by evidkey
//returns number of duplicates.
int TraceEditOperator::remove_duplicates(TimeSeriesEnsemble& d, bool verbose)
{
	try{
	TimeSeriesEnsemble result(d);
	result.member.clear();
	vector<TimeSeries>::iterator dptr;
    int i=0,evid,evid_tmp=-1;
    for(dptr=d.member.begin();dptr!=d.member.end();++dptr)
	{	
		evid=dptr->get_long(evidkey);
		if(evid==evid_tmp)
		{	
			if(verbose)
				cout<<"Duplicate evid found! Removing it from the ensemble."<<endl;
			d.member.erase(dptr);
			++i;
		}
		evid_tmp=evid;
	}
	return(i);
	}catch(...){throw;};
}
TimeSeriesEnsemble TraceEditOperator::exclude_false_traces(TimeSeriesEnsemble& d)
{
    try {
        //int nd=d.member.size();
        //copy metadata
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();dptr!=d.member.end();++i,++dptr)
        {
            if(dptr->live)
                result.member.push_back(*dptr);
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
//subset ensemble by extracting traces with match evids.
TimeSeriesEnsemble TraceEditOperator::extract_traces(TimeSeriesEnsemble& d, 
											set<long> evids_to_extract)
{
	try {
        //int nd=d.member.size();
        //copy metadata
        int evid;
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();dptr!=d.member.end();++i,++dptr)
        {
            evid=dptr->get_long(evidkey);
			if(evids_to_extract.find(evid)!=evids_to_extract.end())
			{
                result.member.push_back(*dptr);
            }
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
//subset ensemble by extracting traces with match evids.
////this overload version set the extracted trace metadata 'is_killed_trace'.
TimeSeriesEnsemble TraceEditOperator::
		extract_traces(TimeSeriesEnsemble& d, 
						set<long> evids_to_extract,
						bool is_killed_trace)
{
	try {
        //int nd=d.member.size();
        //copy metadata
        int evid;
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();dptr!=d.member.end();++i,++dptr)
        {
            evid=dptr->get_long(evidkey);
			if(evids_to_extract.find(evid)!=evids_to_extract.end())
			{
                dptr->put(killed_trace_key,is_killed_trace);
                result.member.push_back(*dptr);
            }
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
//subset ensemble by extracting traces with match evids.
////this overload version set the extracted trace metadata 'is_killed_trace'
// and set the live flag to 'live_status'.
TimeSeriesEnsemble TraceEditOperator::
		extract_traces(TimeSeriesEnsemble& d, 
						set<long> evids_to_extract,
						bool live_status,
						bool is_killed_trace)
{
	try {
        //int nd=d.member.size();
        //copy metadata
        int evid;
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();dptr!=d.member.end();++i,++dptr)
        {
            evid=dptr->get_long(evidkey);
			if(evids_to_extract.find(evid)!=evids_to_extract.end())
			{
                dptr->put(killed_trace_key,is_killed_trace);
                dptr->live=true;
                result.member.push_back(*dptr);
            }
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
TimeSeriesEnsemble TraceEditOperator::extract_traces(
					TimeSeriesEnsemble& d1,
					TimeSeriesEnsemble& d2, MetadataList& mdl)
{
	try {
        //int nd=d.member.size();
        //copy metadata
        int evid1, evid2;
        MDtype mdtype;
        string mdtag;
        TimeSeriesEnsemble result(d1);
        result.member.clear();
        vector<TimeSeries>::iterator dptr2,dptr1;
        int i;
        for(i=0,dptr2=d2.member.begin();dptr2!=d2.member.end();++dptr2)
        {
            evid2=dptr2->get_long(evidkey);
            for(dptr1=d1.member.begin();dptr1!=d1.member.end();++dptr1)
        	{
        		evid1=dptr1->get_long(evidkey);
        		if(evid1==evid2)
        		{
        			TimeSeries ts=*dptr1;
        			//DEBUG
        			MetadataList::iterator mdptr;
        			for(mdptr=mdl.begin();mdptr!=mdl.end();++mdptr)
        			{
        				mdtag=(*mdptr).tag;
        				mdtype=(*mdptr).mdt;
							switch(mdtype)
							{
								case MDreal:
									if(dptr2->is_attribute_set(mdtag))
        							{
										ts.put(mdtag,dptr2->get_double(mdtag));
										break;
									}
									else
									{
										cerr<<"TraceEditOperator: ERROR in copying metadata."<<endl
											<<"attribute "<<mdtag<<" with type real not set."<<endl;
										result.member.clear();
										return(result);
									}
								case MDint:
									if(dptr2->is_attribute_set(mdtag))
        							{
										ts.put(mdtag,dptr2->get_int(mdtag));
										break;
									}
									else
									{
										cerr<<"TraceEditOperator: ERROR in copying metadata."<<endl
											<<"attribute "<<mdtag<<" with type int not set."<<endl;
										result.member.clear();
										return(result);
									}
								case MDstring:
									if(dptr2->is_attribute_set(mdtag))
        							{
										ts.put(mdtag,dptr2->get_string(mdtag));
										break;
									}
									else
									{
										cerr<<"TraceEditOperator: ERROR in copying metadata."<<endl
											<<"attribute "<<mdtag<<" with type string not set."<<endl;
										result.member.clear();
										return(result);
									}
								case MDboolean:
									if(dptr2->is_attribute_set(mdtag))
        							{
										ts.put(mdtag,dptr2->get_bool(mdtag));
										break;
									}
									else
									{
										cerr<<"TraceEditOperator: ERROR in copying metadata."<<endl
											<<"attribute "<<mdtag<<" with type boolean not set."<<endl;
										result.member.clear();
										return(result);
									}
								default:
									throw MetadataError("TraceEditOperator::extract_traces: "
									 + string("requested unsupported metadata type."));
									break;
							}
        			}
        			result.member.push_back(ts);
        		}
        	}
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
TimeSeriesEnsemble TraceEditOperator::extract_false_traces(
		TimeSeriesEnsemble& d,bool is_killed_trace)
{
	try {
        //int nd=d.member.size();
        //copy metadata
        TimeSeriesEnsemble result(d);
        result.member.clear();
        vector<TimeSeries>::iterator dptr;
        int i;
        for(i=0,dptr=d.member.begin();i<d.member.size();++i,++dptr)
        {
            if(!dptr->live)
            {
            	dptr->put(killed_trace_key,is_killed_trace);
                result.member.push_back(*dptr);
            }
        }
        //DEBUG
        //cout<<"result.member.size()="<<result.member.size()<<endl;
        return(result);
    }catch(...){throw;};
}
////add members (d2) to the ensemble (d1).
void TraceEditOperator::merge_ensembles(TimeSeriesEnsemble& d1, 
					TimeSeriesEnsemble& d2)
{
	vector<TimeSeries>::iterator dptr;
	int i;
	try
	{
		for(i=1,dptr=d2.member.begin();dptr!=d2.member.end();++dptr,++i)
		{
			d1.member.insert(d1.member.end(),*dptr);
			cerr<<"added number of traces: "<<i<<endl;
		}
	}catch(...){throw;};
}
////add members (d2) to the ensemble (d1).
//for d2 status to 'trace_status'.
void TraceEditOperator::merge_ensembles(TimeSeriesEnsemble& d1, 
					TimeSeriesEnsemble& d2, bool trace_status)
{
	vector<TimeSeries>::iterator dptr;
	int i;
	try
	{
		for(i=1,dptr=d2.member.begin();dptr!=d2.member.end();++dptr,++i)
		{
			dptr->live=trace_status;
			d1.member.insert(d1.member.end(),*dptr);
			cerr<<"added number of traces: "<<i<<endl;
		}
	}catch(...){throw;};
}
/* Helpers for edit method.  */ //copied from original RFeditorEngine.cc
//written by Gary L. Pavlis. Add this to TraceEditOperator is reasonable.
/* Followup companion helper to apply_kills.  This procedure takes the
set of event ids returned by apply_edits and uses them to kill any
members of tse passed to this function with matching evid.  
In this code this is called to the complement of radial or transverse
depending on which was being edited. */
void TraceEditOperator::apply_kills(TimeSeriesEnsemble& tse, set<long> evids_to_kill)
{
    try {
        vector<TimeSeries>::iterator tptr;
        int i,numkill=0,evid_tmp=-1;
        for(tptr=tse.member.begin();tptr!=tse.member.end();++tptr)
        {
            if(tptr->live)
            {
				// Skip stack trace when present 
				//string sta=tptr->get_string("sta");
				//if(sta==stackstaname) continue;
				int evid=tptr->get_long(evidkey);
				if(evid==evid_tmp) cout<<"duplicate evid: "<<evid<<endl;
				if(evids_to_kill.find(evid)!=evids_to_kill.end())
				{
					//cout << "Transverse killing evid "<<evid<<endl;
					tptr->live=false;
					++numkill;
				}
				evid_tmp=evid;
            }
        }
        //DEBUG
        //cout<<"Number of traces in input data: "<<tse.member.size()<<endl;
        //cout<<"size of evids_to_kill in apply_kills_to_other: "<<evids_to_kill.size()<<endl;
        //cout<<"Number of kills applied in apply_kills_to_other: "<<numkill<<endl;
    }catch(...){throw;};
}
void TraceEditOperator::apply_kills(ThreeComponentEnsemble& tce, set<long> evids_to_kill)
{
    try {
        vector<ThreeComponentSeismogram>::iterator tptr;
        int i,numkill=0,evid_tmp=-1;
        for(tptr=tce.member.begin();tptr!=tce.member.end();++tptr)
        {
            if(tptr->live)
            {
				// Skip stack trace when present 
				string sta=tptr->get_string("sta");
				int evid=tptr->get_long(evidkey);
				if(evid==evid_tmp) cout<<"duplicate evid: "<<evid<<endl;
				if(evids_to_kill.find(evid)!=evids_to_kill.end())
				{
					//cout << "Transverse killing evid "<<evid<<endl;
					tptr->live=false;
					++numkill;
				}
				evid_tmp=evid;
            }
        }
        //DEBUG
        //cout<<"Number of traces in input data: "<<tse.member.size()<<endl;
        //cout<<"size of evids_to_kill in apply_kills_to_other: "<<evids_to_kill.size()<<endl;
        //cout<<"Number of kills applied in apply_kills_to_other: "<<numkill<<endl;
    }catch(...){throw;};
}
/* Returns a new ensemble reordered by ordering.   Does this 
   blindly without checking bounds for efficiency as it assumes
   this is intimately linked to the procedure immediately above.
   Do not transport blindly.  Also not very efficient because of
   double copy operation*/
//copied from RFeditorEngine.cc.
TimeSeriesEnsemble TraceEditOperator::reorder_ensemble(TimeSeriesEnsemble& t,
        vector<int> ordering)
{
    try{
        TimeSeriesEnsemble result(t);
        int i;
        /* This is not the most obvious way to do this, but
           I think this will be more efficient than indexing
           the other way */
        result.member.clear();
        int nm=t.member.size();
        result.member.reserve(nm);
        for(i=0;i<nm;++i)
        {
            int im=ordering[i];
            result.member.push_back(t.member[im]);
        }
        return(result);
    } catch(...){throw;};
}
//get trimed data within given TimeWindow parameter.
ThreeComponentSeismogram TraceEditOperator::trim_data(ThreeComponentSeismogram& d, TimeWindow twin)
{
	if( d.tref==absolute )
		throw SeisppError(string("Error (trim data): ")
			+ "data to be trimed must have "
			+ "relative time base");
	ThreeComponentSeismogram out3c(d);
	int nstart=d.sample_number(twin.start);
	int nend=d.sample_number(twin.end);
	int nsout=1+static_cast<int>(twin.length()/d.dt);
	out3c.u=dmatrix(3,nsout);
	out3c.u.zero();
	//get t0 for out3c.
	out3c.t0=d.t0+d.dt*nstart;
	out3c.ns=nsout;
	int i,k,si=0;
	for(i=nstart;i<=nend;++i,++si)
	{
		double *sptr=out3c.u.get_address(0,si);
		double *dptr=d.u.get_address(0,i);
		for(k=0;k<3;++k)
		{
			*sptr=*dptr;
			++sptr;
			++dptr;
		}
    }
	return out3c;
}
//get trimed data within given TimeWindow parameter.
TimeSeries TraceEditOperator::trim_data(TimeSeries& d, TimeWindow twin)
{
	if( d.tref==absolute )
		throw SeisppError(string("Error (trim data): ")
			+ "data to be trimed must have "
			+ "relative time base");
	TimeSeries outts(d);
	int nstart=d.sample_number(twin.start);
	int nend=d.sample_number(twin.end);
	int nsout=1+static_cast<int>(twin.length()/d.dt);
	outts.s.clear();
	outts.s=vector<double>(nsout,0.0);
	//std::fill(outts.s.begin(),outts.s.end(),0.0);
	//cerr<<nsout<<endl;
	//cerr<<"size "<<outts.s.size()<<endl; 
	//get t0 for outts.
	outts.t0=twin.start;
	outts.ns=nsout;
	int i,si=0;
	
	for(i=nstart;i<=nend;++i,++si)
	{
		//cerr<<"i="<<i<<":"<<d.s[i]<<endl;
		outts.s[si]=d.s[i];
		//cerr<<"i="<<i<<":"<<outts.s[si]<<","<<d.s[i]<<endl;
    }
    //exit(-1);
	return outts;
}
/////////////////////////////////////////////////////////////////////
//=================================================================//
//============*** Begin of Auto-Killing Procedures ***=============//
//===== Note: those procedures are supposed to be applied =========//
//=====       only on radial traces.                      =========//
//=================================================================//
/////////////////////////////////////////////////////////////////////
/*
// Kill traces with extremely large amplitude phases (doesn't have to be the first arrival).
// Amp_max is fabs of true amplitude value, not normalized.
*/
set<long> TraceEditOperator::kill_large_amplitude_traces(TimeSeriesEnsemble& tse, double Amp_max)
{
	try
	{
		double vmax(0.0),vmin(0.0);
		int numkills;
		set<long> evids_to_kill;
		long evid;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
        	vmax=0.0;
        	if(iptr->live)
        	{
        		for(int i=0;i<iptr->ns;++i)
        			if(fabs(iptr->s[i])>=(vmax ))
        				vmax=fabs(iptr->s[i]);
        		if(vmax>Amp_max )
        		{
        			iptr->live=false;
        			iptr->put(killmethodkey,string(AUTOKILL_LargeAmpTraces));
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//DEBUG
					cout<<"Kill evid = "<<evid<<" with max abs amplitude = "
						<<vmax<<endl;
					++numkills;
        		}
        	}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_LargeAmpTraces;
    	tes.TraceAmp_range[1]=Amp_max;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
        return(evids_to_kill);
	}catch(...){throw;};
}

/*
kill traces by first peak amplitudes following the given amplitude threshold (Amp_min).
this function returns the list of evids assocated with the killed traces.
Xiaotao Yang
*/
set<long> TraceEditOperator::kill_negative_FA_traces(TimeSeriesEnsemble& tse,
			double FA_detect_length)
{
	try 
	{
		TraceEditStatistics tes;
		double amplitude(0.0),FA_amplitude(0.0),FA_time(0.0);
		long evid=-1, evid_tmp;
		int numkills=0;
		set<long> evids_to_kill;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            //amplitude=0.0;
            if(iptr->live)
            {
				string FA_type=find_FirstArrival(*iptr,FA_sensitivity,
								FA_detect_length,data_shaping_wavelet_type);
				FA_time=iptr->get_double(FA_time_key);
				FA_amplitude=iptr->get_double(FA_amplitude_key);
				//FA_time=find_FirstArrival(*iptr,FA_sensitivity,&FA_amplitude);
				if(FA_amplitude<=(-1.0*MYZERO)) 
				{
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_NegativeFA));
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					++numkills;
				}
        	}
    	}
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_NegativeFA;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
        //cout<<"numkills:"<<numkills<<endl;
		return(evids_to_kill);
	}catch(...){throw;};
}
//Kill traces with negative first arrival
set<long> TraceEditOperator::kill_negative_FA_traces(
			TimeSeriesEnsemble& tse,TimeWindow FA_search_window,
			double FA_detect_length)
{
	try 
	{
		double amplitude(0.0),FA_amplitude(0.0),FA_time(0.0);
		long evid=-1, evid_tmp;
		int numkills=0;
		set<long> evids_to_kill;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            //amplitude=0.0;
            if(iptr->live)
            {
				TimeSeries tmpts=trim_data(*iptr,FA_search_window);
				/*
				FA_time=find_FirstArrival(
						tmpts,FA_sensitivity,&FA_amplitude);
				*/
				string FA_type=find_FirstArrival(tmpts,FA_sensitivity,
								FA_detect_length,data_shaping_wavelet_type);
				FA_time=tmpts.get_double(FA_time_key);
				FA_amplitude=tmpts.get_double(FA_amplitude_key);
				
				//debug
				/*
				cout<<"FA_TYPE = "<<FA_type<<endl;
				cout<<"evid= "<<iptr->get_long(evidkey)<<", FA_time= "<<FA_time
					<<", FA_amplitude= "<<FA_amplitude<<endl;
				*/
				/*
				double vmax=0.0;
				for(int i=0;i<tmpts.ns;++i)
				{
					if(tmpts.s[i]>vmax) vmax=tmpts.s[i];
				}*/
				if(FA_amplitude<=(-1.0*MYZERO)) 
				//if(vmax<MYZERO)
				{
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_NegativeFA));
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					++numkills;
				}
				tmpts.s.clear();
        	}
    	}
    	TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_NegativeFA;
    	tes.FA_twin=FA_search_window;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
        //cout<<"numkills:"<<numkills<<endl;
		return(evids_to_kill);
	}catch(...){throw;};
}
//New algorithm: kill Negative Frist Arrivals, which
//mean the arrivals within the first several seconds defined 
//by the NFA_tolerance_window. The old algorithm kills
//the traces with negative first peaks, which is sometimes not correct.
set<long> TraceEditOperator::kill_negative_FAs_traces(
			TimeSeriesEnsemble& tse,TimeWindow NFA_tolerance_window)
{
	try 
	{
		//double amplitude(0.0),FA_amplitude(0.0),FA_time(0.0);
		long evid=-1, evid_tmp;
		//int numkills=0;
		set<long> evids_to_kill;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            //amplitude=0.0;
            if(iptr->live)
            {
				TimeSeries tmpts=trim_data(*iptr,NFA_tolerance_window);
				
				double vmax=0.0;
				for(int i=0;i<tmpts.ns;++i)
				{
					if(tmpts.s[i]>vmax) vmax=tmpts.s[i];
				}
				//if(FA_amplitude<=(-1.0*MYZERO)) 
				if(vmax<MYZERO)
				{
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_NegativeFA));
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//++numkills;
				}
				tmpts.s.clear();
        	}
    	}
    	TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_NegativeFA;
    	tes.FA_twin=NFA_tolerance_window;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
        //cout<<"numkills:"<<numkills<<endl;
		return(evids_to_kill);
	}catch(...){throw;};
}

//search small FA amplitude within given timewindow. 
// Amp_min: minimum amplitude (normalized by the maximum amplitude of the current trace or the global FA).
// normalization_method: either GLOBAL_MAX or LOCAL_MAX
set<long> TraceEditOperator::kill_small_FA_traces
							(TimeSeriesEnsemble& tse, 
								double Amp_min,
								TimeWindow FA_search_window, 
								string normalization_method,
								double FA_detect_length)
{
	try 
	{
		const string error_base("ERROR in 'kill_small_FA_traces': ");
		double amplitude(0.0);
		double vmax(0.0),ratio(0.0), FA_amplitude(0.0),FA_time(0.0);
		long evid;
		int numkills(0),i;
		bool local_normalization;
		set<long> evids_to_kill;
		//find global maximum FA values from all traces.
		if(normalization_method==NORMALIZE_BY_GLOBAL_FA_MAX)
			vmax=find_max_abs_FirstArrival(tse, FA_sensitivity,
					FA_detect_length,data_shaping_wavelet_type);
		else if(normalization_method==NORMALIZE_BY_GLOBAL_MAX)
			vmax=find_max_abs_amplitude(tse);
		else if(normalization_method==NORMALIZE_BY_LOCAL_MAX)
			local_normalization=true;
		else
		{
			cerr<<error_base<<"Unrecognized normalization method!"<<endl
				<<"Use GLBOAL_MAX, GLOCAL_FA_MAX or LOCAL_MAX."<<endl;
			exit(-1);	
		}
		//DEBUG
		//cout<<"max global FA = "<<vmax<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	if(local_normalization)
            	{
            		vmax=0.0;
					for(int i=0;i<iptr->ns;++i)
        				if(fabs(iptr->s[i])>=fabs(vmax))
        					vmax=fabs(iptr->s[i]);
        		}
        		//DEBUG
				//cout<<"vmax = "<<vmax<<endl;
				TimeSeries tmpts=trim_data(*iptr,FA_search_window);
				//FA_time=find_FirstArrival(
				//		tmpts,FA_sensitivity,&FA_amplitude);
				string FA_type=find_FirstArrival(tmpts,FA_sensitivity,
								FA_detect_length,data_shaping_wavelet_type);
				if(FA_type.size()==0)
				{
					cerr<<error_base<<"skipped this procedure!"<<endl;
					evids_to_kill.clear();
					return(evids_to_kill);
				}
				FA_time=tmpts.get_double(FA_time_key);
				FA_amplitude=tmpts.get_double(FA_amplitude_key);
				
				tmpts.s.clear();
				//cout<<"FA_amplitude = "<<FA_amplitude<<", at time = "<<FA_time<<endl;
				//if(FA_time<FA_search_window.start || FA_time>FA_search_window.end) 
				//	FA_amplitude=0.0;
				ratio=FA_amplitude/(fabs(vmax)+MYZERO);
				//cout<<"Normalized FA amplitude = "<<ratio<<endl;
				if(fabs(ratio)<(Amp_min )) 
				{
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//cout<<"killing evid: "<<evid<<endl;
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_SmallFA));
					++numkills;
				}
			}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_SmallFA;
    	tes.FA_twin=FA_search_window;
    	tes.FA_range[0]=Amp_min;
    	tes.AmpNormalizeMethod=normalization_method;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
		return(evids_to_kill);
	}catch(...){throw;};
}

//kill traces with growing p-coda phases within given p-coda search window.
//Algorithm: max amplitude within the P-coda search window is greater than
// the first arrival amplitude with given FA search window.
set<long> TraceEditOperator::kill_growing_PCoda_traces(TimeSeriesEnsemble& tse,
					TimeWindow FA_search_window,TimeWindow PCoda_search_window,
					double PCoda_grow_tolerance,double FA_detect_length)
{
	try 
	{
		const string error_base("ERROR in 'kill_growing_PCoda_traces': ");
		//tolerance: only kill trace satisfing condition 
		//'amplitude_coda - amplitude_FA >= tolerance'.
		//This is the normalized value allowing the P-Coda to grow.
		double tolerance=PCoda_grow_tolerance; 
		double FA_amplitude(0.0),FA_time(0.0);;
		double vmax(0.0),vmax_global(0.0);
		long evid;
		int numkills(0),i;
		set<long> evids_to_kill;
		
		//DEBUG
		//cerr<<"Pcoda tolerance: "<<tolerance<<endl;
		//cout<<"max global FA = "<<vmax<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	vmax=0.0;
            	vmax_global=0.0;
            	//find peak amplitude of the trace.
            	for(int i=0;i<iptr->ns;++i)
					if(fabs(iptr->s[i])>=fabs(vmax_global))
						vmax_global=fabs(iptr->s[i]);
            	//TimeSeries tmpts=trim_data(*iptr,PCoda_search_window);
				//find peak amplitude of the trimed data window.
				int nstart=iptr->sample_number(PCoda_search_window.start);
				int nend=iptr->sample_number(PCoda_search_window.end);
				vector<double>::iterator dptr;
				for(dptr=(iptr->s.begin()+nstart);dptr<=(iptr->s.begin()+nend);++dptr)
					if(fabs(*dptr)>=fabs(vmax))
						vmax=fabs(*dptr);
				
				//for(int i=0;i<tmpts.ns;++i)
				//	if(fabs(tmpts.s[i])>=fabs(vmax))
				//		vmax=fabs(tmpts.s[i]);
				
        		//DEBUG
				//cout<<"vmax = "<<vmax<<endl;
				//tmpts.s.clear();
				
				TimeSeries tmpts=trim_data(*iptr,FA_search_window);
				string FA_type=find_FirstArrival(tmpts,FA_sensitivity,
								FA_detect_length,data_shaping_wavelet_type);
				FA_time=tmpts.get_double(FA_time_key);
				FA_amplitude=tmpts.get_double(FA_amplitude_key);
				tmpts.s.clear();
				
				if((fabs(vmax)-fabs(FA_amplitude))/(vmax_global+MYZERO)>(tolerance )) 
				{
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//cout<<"killing evid: "<<evid<<endl;
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_GrowingPCoda));
					++numkills;
				}
			}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_GrowingPCoda;
    	tes.FA_twin=FA_search_window;
    	tes.PCoda_twin=PCoda_search_window;
    	tes.PCoda_grow_tolerance=tolerance;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
		return(evids_to_kill);
	}catch(...){throw;};
}
//Algorithm: max amplitude within the P-coda search window is greater than
// the max amplitude prior to the P-coda window.

set<long> TraceEditOperator::kill_growing_PCoda_traces(TimeSeriesEnsemble& tse,
					TimeWindow PCoda_search_window,double PCoda_grow_tolerance)
{
	try 
	{
		const string error_base("ERROR in 'kill_growing_PCoda_traces': ");
		//tolerance: only kill trace satisfing condition 
		//'amplitude_coda - amplitude_FA >= tolerance'.
		//This is the normalized value allowing the P-Coda to grow.
		double tolerance=PCoda_grow_tolerance; 
		double vmax(0.0),vmax_coda(0.0),vmax_global(0.0);
		long evid;
		int numkills(0),i;
		set<long> evids_to_kill;

		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	vmax=0.0; vmax_coda=0.0;
            	vmax_global=0.0;
            	//find peak amplitude of the trace. used in normalization.
            	for(int i=0;i<iptr->ns;++i)
					if(fabs(iptr->s[i])>=fabs(vmax_global))
						vmax_global=fabs(iptr->s[i]);
            	//TimeSeries tmpts=trim_data(*iptr,PCoda_search_window);
				//find peak amplitude of the trimed data window.
				int nstart=iptr->sample_number(PCoda_search_window.start);
				int nend=iptr->sample_number(PCoda_search_window.end);
				vector<double>::iterator dptr;
				for(dptr=(iptr->s.begin()+nstart);dptr<=(iptr->s.begin()+nend);++dptr)
					if(fabs(*dptr)>=fabs(vmax_coda))
						vmax_coda=fabs(*dptr);
				for(dptr=iptr->s.begin();dptr<(iptr->s.begin()+nstart);++dptr)
					if(fabs(*dptr)>=fabs(vmax))
						vmax=fabs(*dptr);
				//for(int i=0;i<tmpts.ns;++i)
				//	if(fabs(tmpts.s[i])>=fabs(vmax_coda))
				//		vmax_coda=fabs(tmpts.s[i]);
				
        		//DEBUG
				//cout<<"vmax_coda = "<<vmax_coda<<endl;
				//tmpts.s.clear();
				
				if((fabs(vmax_coda)-fabs(vmax))/(vmax_global+MYZERO)>(tolerance )) 
				{
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//cout<<"killing evid: "<<evid<<endl;
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_GrowingPCoda));
					++numkills;
				}
			}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_GrowingPCoda;
    	//tes.FA_twin=FA_search_window;
    	tes.PCoda_twin=PCoda_search_window;
    	tes.PCoda_grow_tolerance=tolerance;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
		return(evids_to_kill);
	}catch(...){throw;};
}
//kill traces with P-Coda lacking zero points within the tolerance length of a timewindow.
//tlength: tolerance timewindow length. Zero points mean the data have both negative and
//positive values and have zeroes changing from positive to negative or vice versa.
set<long> TraceEditOperator::kill_ClusteredArrivals_traces(TimeSeriesEnsemble& tse,
			TimeWindow CodaCA_search_window,double tlength)
{
	try 
	{
		const string error_base("ERROR in 'kill_ClusteredArrivals_traces': ");
		double FA_amplitude(0.0),FA_time(0.0),tflag,vmax=0.0,fmin=1.0/(2.0*tlength);
		long evid;
		int numkills(0),i,nzp=0,tnsample;
		set<long> evids_to_kill;
		//tolerance number of samples.
		tnsample=SEISPP::nint(tlength/tse.member[0].dt)+1;
		//cerr<<"tnsample = "<<tnsample<<endl;
		//DEBUG
		//cout<<"max global FA = "<<vmax<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	vmax=0.0;
            	TimeSeries tmpts=trim_data(*iptr,CodaCA_search_window);
            	//get max amplitude within the searching window.
            	for(int i=0;i<tmpts.ns;++i)
					if(fabs(tmpts.s[i])>=fabs(vmax))
						vmax=fabs(tmpts.s[i]) ;
				for(int i=0;i<(tmpts.ns-tnsample+1);++i)
				{
					nzp=0;
					tflag=tmpts.s[i]/vmax;;
					for(int j=i+1; j<(i+tnsample);++j)
						if(((tmpts.s[j])*tflag)<=MYZERO )//|| 
							//fabs(tmpts.s[j]/vmax)<=0.001)
						{
							++nzp;
							break;
						}	
						else
							tflag=tmpts.s[j];
					if(nzp==0)
					{
						iptr->live=false;
						iptr->put(killmethodkey,string(AUTOKILL_ClusteredArrivals));
						evid=iptr->get_long(evidkey);
						evids_to_kill.insert(evid);
						++numkills;
						break;
					}
				}
        		//DEBUG
				//cout<<"vmax = "<<vmax<<endl;
				tmpts.s.clear();
			}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_ClusteredArrivals;
    	tes.CodaCA_twin=CodaCA_search_window;
    	tes.CodaCA_tolerance_twin_length=tlength;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
		return(evids_to_kill);
	}catch(...){throw;};
}
//kill low frequency Contaminated traces.
//looking for the max from the spectrum. kill traces that the frequency
//of the max amplitude falls into the low-frequency window (lf_min, lf_max).
//the lower bound is needed since some traces with ultra low frequency background
//may not be bad traces. e.g., 100s singnal may have no effect on the receiver functions.
//*****Commented out for now ********

set<long> TraceEditOperator::kill_LowFrequencyContaminated_traces(TimeSeriesEnsemble& tse,
			double lf_min,double lf_max)
{
	
	try 
	{
		const string error_base("ERROR in 'kill_LowFrequencyContaminated_traces': ");
		double vmax=0.0;
		long evid;
		int numkills(0),i;
		set<long> evids_to_kill;
		
		cerr<<error_base<<"this procedure is blocked in the current version of RFeditor. Skipped!"<<endl;
		
		evids_to_kill.clear();
		return(evids_to_kill);
		
		/*
		//tolerance number of samples.
		//tnsample=SEISPP::nint(tlength/tse.member[0].dt)+1;
		//cerr<<"tnsample = "<<tnsample<<endl;
		//DEBUG
		//cout<<"max global FA = "<<vmax<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	vmax=0.0;
            	
            	//test for fft spectrum analysis
            	int vmax_loc=0;
            	double vmax_f=0.0,df;
            	//TimeSeries tmpts=trim_data(*iptr,CodaCA_search_window);
            	df=0.5*(1.0/iptr->dt)/static_cast<double>(iptr->ns);
            	//cout<<"df="<<df<<endl;
            	vector<double> fft_tmp;
            	//cout<<"nsamp="<<iptr->ns<<endl;
            	fft_tmp.resize(iptr->ns);
            	//cout<<"Before FFT: size="<<fft_tmp.size()<<endl;
            	fft_tmp=compute_fft_real_transform(iptr->s);
            	//cout<<"After FFT: returned size="<<fft_tmp.size()<<endl;
            	
				for (int i = 0; i < fft_tmp.size(); i++)
				{
				  printf ("%d: %e\n", i, fabs(fft_tmp[i]));
				}
				printf ("\n");
            	//get histogram of the spectrum data.
            	//nbin is estimated based on the given tolerance minimum frequency.
            	
            	//int nbin=SEISPP::nint(fft_tmp.size()*df/fmin);
            	
            	//gsl_histogram * h = gsl_histogram_alloc (nbin);
    			//gsl_histogram_set_ranges_uniform (h, 0, fft_tmp.size()*df);
    			
    			
            	for(int k=0;k<fft_tmp.size();++k)
            	//	gsl_histogram_increment (h, fft_tmp[k]);
            	{	if(fabs(fft_tmp[k])>fabs(vmax)) {vmax=fabs(fft_tmp[k]);vmax_loc=k;}}
            	//cout<<"vmax_loc= "<<vmax_loc<<endl;
            	
            	//int fmax_loc=gsl_histogram_max_bin(h);
            	//double lr,ur;
            	//gsl_histogram_get_range(h,fmax_loc,&lr,&ur);
            	
            	vmax_f=vmax_loc*df;
            	//gsl_histogram_fprintf (stdout, h, "%g", "%g");
            	if(vmax_f<=lf_max && vmax_f>=lf_min)
            	{
            		//cout<<"vmax_f= "<<vmax_f<<", fmin required="<<fmin<<endl;
            		iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_LowFrequencyContaminated));
					evid=iptr->get_long(evidkey);
					//debug
					
					//if(evid==1164)
					//{
					//	for(int k=0;k<fft_tmp.size();++k)
						//	gsl_histogram_increment (h, fft_tmp[k]);
					//	{	cout<<k*df<<"    "<<fabs(fft_tmp[k])<<endl;}
					//}
					evids_to_kill.insert(evid);
					++numkills;
					
            	}
            	fft_tmp.clear();
            	//gsl_histogram_free (h);
            	//tmpts.s.clear();
            	//end of test for fft spectrum analysis
			}
        }
        TraceEditStatistics tes;
    	tes.station=tse.get_string("sta");
    	tes.method=AUTOKILL_LowFrequencyContaminated;
    	tes.LowFrequency_range[0]=lf_min;
    	tes.LowFrequency_range[1]=lf_max;
    	tes.nkilled=evids_to_kill.size();
    	statistics.push_back(tes);
		return(evids_to_kill);
		*/
	}catch(...){throw;};
	
}


//a series of procedures killing by decon parameter thresholds.
set<long> TraceEditOperator::kill_by_decon_ALL(TimeSeriesEnsemble& tse,
									int niteration_min,int niteration_max,
									int nspike_min,int nspike_max,
									double epsilon_min,double epsilon_max,
									double peakamp_min,double peakamp_max,
									double averamp_min,double averamp_max,
									double rawsnr_min,double rawsnr_max)
{
	set<long> evids_to_kill, kill_tmp;
	try
	{
		//
		evids_to_kill=kill_by_decon_niteration(tse,niteration_min,niteration_max);
		//
		kill_tmp=kill_by_decon_nspike(tse,nspike_min,nspike_max);
		if(kill_tmp.size()>0) evids_to_kill.insert(kill_tmp.begin(),kill_tmp.end());
		//
		kill_tmp=kill_by_decon_epsilon(tse,epsilon_min,epsilon_max);
		if(kill_tmp.size()>0) evids_to_kill.insert(kill_tmp.begin(),kill_tmp.end());
		//
		kill_tmp=kill_by_decon_peakamp(tse,peakamp_min,peakamp_max);
		if(kill_tmp.size()>0) evids_to_kill.insert(kill_tmp.begin(),kill_tmp.end());
		//
		kill_tmp=kill_by_decon_averamp(tse,averamp_min,averamp_max);
		if(kill_tmp.size()>0) evids_to_kill.insert(kill_tmp.begin(),kill_tmp.end());
		//
		kill_tmp=kill_by_decon_rawsnr(tse,rawsnr_min,rawsnr_max);
		if(kill_tmp.size()>0) evids_to_kill.insert(kill_tmp.begin(),kill_tmp.end());
	}catch(...){throw;};
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_by_decon_niteration(TimeSeriesEnsemble& tse,
									int niteration_min,int niteration_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_niteration"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	int tmp=iptr->get_int(decon_niteration_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<niteration_min || tmp>niteration_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconNiteration));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr){serr.what();};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconNiteration;
	tes.DeconNiteration_range[0]=niteration_min;
	tes.DeconNiteration_range[1]=niteration_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_by_decon_nspike(TimeSeriesEnsemble& tse,
									int nspike_min,int nspike_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_nspike"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	int tmp=iptr->get_int(decon_nspike_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<nspike_min || tmp>nspike_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconNspike));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr)
	{
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconNspike;
	tes.DeconNspike_range[0]=nspike_min;
	tes.DeconNspike_range[1]=nspike_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_by_decon_epsilon(TimeSeriesEnsemble& tse,
									double epsilon_min,double epsilon_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_epsilon"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	double tmp=iptr->get_double(decon_epsilon_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<epsilon_min || tmp>epsilon_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconEpsilon));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr)
	{
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconEpsilon;
	tes.DeconEpsilon_range[0]=epsilon_min;
	tes.DeconEpsilon_range[1]=epsilon_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}

set<long> TraceEditOperator::kill_by_decon_peakamp(TimeSeriesEnsemble& tse,
									double peakamp_min,double peakamp_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_peakamp"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	double tmp=iptr->get_double(decon_peakamp_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<peakamp_min || tmp>peakamp_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconPeakamp));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr)
	{
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconPeakamp;
	tes.DeconPeakamp_range[0]=peakamp_min;
	tes.DeconPeakamp_range[1]=peakamp_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_by_decon_averamp(TimeSeriesEnsemble& tse,
									double averamp_min,double averamp_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_averamp"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	double tmp=iptr->get_double(decon_averamp_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<averamp_min || tmp>averamp_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconAveramp));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr)
	{
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconAveramp;
	tes.DeconAveramp_range[0]=averamp_min;
	tes.DeconAveramp_range[1]=averamp_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_by_decon_rawsnr(TimeSeriesEnsemble& tse,
									double rawsnr_min,double rawsnr_max)
{
	set<long> evids_to_kill;
	try
	{
		//cerr<<"kill_by_decon_rawsnr"<<endl;
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
            	double tmp=iptr->get_double(decon_rawsnr_key);
            	//cerr<<"	"<<tmp<<endl;
            	if(tmp<rawsnr_min || tmp>rawsnr_max)
            	{
            		iptr->live=false;
            		iptr->put(killmethodkey,string(AUTOKILL_DeconRawsnr));
            		evids_to_kill.insert(iptr->get_long(evidkey));
            	}
            }
        }
        //cerr<<evids_to_kill.size()<<endl;
	}catch(SeisppError& serr)
	{
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DeconRawsnr;
	tes.DeconRawsnr_range[0]=rawsnr_min;
	tes.DeconRawsnr_range[1]=rawsnr_max;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return evids_to_kill;
}

//kill by setting minimum stackweight.
set<long> TraceEditOperator::kill_low_stackweight_traces(
				TimeSeriesEnsemble& tse,double sw_min)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_stackweight_traces.");
	set<long> evids_to_kill;
	double swtmp;
	try 
	{
		long evid;
		int numkills(0),i;		
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				swtmp=iptr->get_double(SEISPP::stack_weight_keyword);
				//cerr<<swtmp<<endl;
				if(swtmp < (sw_min)) 
				{
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//cout<<"killing evid: "<<evid<<endl;
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_StackWeightCutoff));
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_StackWeightCutoff;
	tes.StackW_min=sw_min;
	tes.nkilled=evids_to_kill.size();
	//DEBUG
	//cout<<tes.nkilled<<endl;
	statistics.push_back(tes);
	return(evids_to_kill);
}

//kill by setting minimum dsi.
set<long> TraceEditOperator::kill_low_dsi_traces(
				TimeSeriesEnsemble& tse,double dsi_min)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_dsi_traces.");
	set<long> evids_to_kill;
	double dsitmp;
	try 
	{
		long evid;
		int numkills(0),i;	
		this->compute_decon_success_index(tse);
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				dsitmp=iptr->get_double(decon_success_index_key);
				//cerr<<swtmp<<endl;
				if(dsitmp < (dsi_min)) 
				{
					evid=iptr->get_long(evidkey);
					evids_to_kill.insert(evid);
					//cout<<"killing evid: "<<evid<<endl;
					iptr->live=false;
					iptr->put(killmethodkey,string(AUTOKILL_DSICutoff));
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_DSICutoff;
	tes.DSI_min=dsi_min;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}

//kill by setting minimum correlation coefficience with reference trace.
set<long> TraceEditOperator::kill_low_ref_correlation_traces(
				TimeSeriesEnsemble& tse,double xcorcoe_min)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_ref_correlation_traces.");
	set<long> evids_to_kill;
	try 
	{
		long evid;
		int numkills(0),i;
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				if(iptr->is_attribute_set(xcorcoekey))
				{	if(iptr->get_double(xcorcoekey) < xcorcoe_min) 
					{
						evid=iptr->get_long(evidkey);
						evids_to_kill.insert(evid);
						//cout<<"killing evid: "<<evid<<endl;
						iptr->live=false;
						iptr->put(killmethodkey,string(AUTOKILL_RefXcorCutoff));
					}
				}
				else
				{	cerr<<error_base<<endl
						<<xcorcoekey<<" not set in evid = "<<iptr->get_long(evidkey)<<endl;
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_RefXcorCutoff;
	tes.RefXcor_min=xcorcoe_min;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_low_ref_correlation_traces(
					TimeSeriesEnsemble& tse,int ref_evid, 
					TimeWindow xcor_twin,double xcorcoe_min)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_ref_correlation_traces. ");
	set<long> evids_to_kill;
	try 
	{
		long evid;
		int numkills(0),i;
		//set xcorcoef before running killing procedure.
		if(!this->compute_trace_xcorcoe(tse,ref_evid,xcor_twin))
		{
			cerr<<error_base<<"Can't compute trace xcorcoe."<<endl;
			return evids_to_kill;
		};
		
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				if(iptr->is_attribute_set(xcorcoekey))
				{	if(iptr->get_double(xcorcoekey) < xcorcoe_min) 
					{
						evid=iptr->get_long(evidkey);
						evids_to_kill.insert(evid);
						//cout<<"killing evid: "<<evid<<endl;
						iptr->live=false;
						iptr->put(killmethodkey,string(AUTOKILL_RefXcorCutoff));
					}
				}	
				else
				{	cerr<<error_base<<endl
						<<xcorcoekey<<" not set in evid = "<<iptr->get_long(evidkey)<<endl;
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};
	stringstream ss;
	ss<<"eventid_"<<ref_evid;
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_RefXcorCutoff;
	tes.XCor_twin=xcor_twin;
	tes.ref_trace_tag=ss.str();
	tes.RefXcor_min=xcorcoe_min;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_low_ref_correlation_traces(
					TimeSeriesEnsemble& tse,TimeSeries ref_trace, 
					TimeWindow xcor_twin,double xcorcoe_min)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_ref_correlation_traces.");
	set<long> evids_to_kill;
	try 
	{
		long evid;
		int numkills(0),i;
		//set xcorcoef before running killing procedure.
		if(!this->compute_trace_xcorcoe(tse,ref_trace,xcor_twin))
		{
			cerr<<error_base<<"Can't compute trace xcorcoe."<<endl;
			return evids_to_kill;
		};		
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				if(iptr->is_attribute_set(xcorcoekey))
				{	if(iptr->get_double(xcorcoekey) < xcorcoe_min ) 
					{
						evid=iptr->get_long(evidkey);
						evids_to_kill.insert(evid);
						//cout<<"killing evid: "<<evid<<endl;
						iptr->live=false;
						iptr->put(killmethodkey,string(AUTOKILL_RefXcorCutoff));
					}
				}
				else
				{	cerr<<error_base<<endl
						<<xcorcoekey<<" not set in evid = "<<iptr->get_long(evidkey)<<endl;
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};
	
	string ref_trace_tag;
	if(ref_trace.is_attribute_set((char *)"sta"))
		ref_trace_tag="trace_"+ref_trace.get_string("sta");
	else
		ref_trace_tag="trace_unknown_station";
	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_RefXcorCutoff;
	tes.XCor_twin=xcor_twin;
	tes.ref_trace_tag=ref_trace_tag;
	tes.RefXcor_min=xcorcoe_min;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
set<long> TraceEditOperator::kill_low_RFQualityIndex_traces(
					TimeSeriesEnsemble& tse,
					double a1, double a2, double a3, 
					double qi_min,bool normalize)
{
	const string error_base("ERROR in TraceEditOperator::kill_low_RFQualityIndex_traces.");
	set<long> evids_to_kill;
	try 
	{
		long evid;
		int numkills(0),i;
		//compute RF Quality Index before running killing procedure.
		if(!this->compute_RF_quality_index(tse,a1,a2,a3,normalize))
		{
			cerr<<error_base<<"failed to compute RF Quality Index."<<endl;
			return evids_to_kill;
		};		
		//DEBUG
		vector<TimeSeries>::iterator iptr;
		for(iptr=tse.member.begin();iptr!=tse.member.end();++iptr)
        {
            if(iptr->live)
            {
				if(iptr->is_attribute_set(RF_quality_index_key))
				{	if(iptr->get_double(RF_quality_index_key) < qi_min) 
					{
						evid=iptr->get_long(evidkey);
						evids_to_kill.insert(evid);
						//cout<<"killing evid: "<<evid<<endl;
						iptr->live=false;
						iptr->put(killmethodkey,string(AUTOKILL_RFQICutoff));
					}
				}
				else
				{	cerr<<error_base<<endl
						<<RF_quality_index_key<<" not set in evid = "
						<<iptr->get_long(evidkey)<<endl;
				}
			}
        }
	}catch(SeisppError& serr)
	{
		cerr<<error_base<<endl;
		serr.what();
	};

	TraceEditStatistics tes;
	tes.station=tse.get_string("sta");
	tes.method=AUTOKILL_RFQICutoff;
	tes.QI_weights[0]=a1;
	tes.QI_weights[1]=a2;
	tes.QI_weights[2]=a3;
	tes.QI_min=qi_min;
	tes.nkilled=evids_to_kill.size();
	statistics.push_back(tes);
	return(evids_to_kill);
}
//set all traces back to live.
void TraceEditOperator::set_ensemble_live_status(TimeSeriesEnsemble& tse,
	 bool live_status)
{
	try
	{
		vector<TimeSeries>::iterator dptr;
		int i;
		for(i=0,dptr=tse.member.begin();dptr!=tse.member.end();++dptr,++i)
		{
			dptr->live=live_status;
			/*
			if(dptr->live)
				cerr<<"live"<<endl;
			else
				cerr<<"dead"<<endl;
			*/
		}
	}catch(SeisppError& serr)
	{
		cerr<<"Error in 'set_ensemble_live_status':"<<endl;
		serr.what();
	};
}

//undo kills to traces with given evid list to restore.
void TraceEditOperator::undo_kills_to_trace
	(TimeSeriesEnsemble& tse, set<long> evids_to_restore)
{
	try {
        vector<TimeSeries>::iterator tptr;
        for(tptr=tse.member.begin();tptr!=tse.member.end();++tptr)
        {
				// Skip stack trace when present 
				//string sta=tptr->get_string("sta");
				//if(sta==stackstaname) continue;
				int evid=tptr->get_long(evidkey);
				if(evids_to_restore.find(evid)!=evids_to_restore.end())
				{
					//cout << "Transverse killing evid "<<evid<<endl;
					tptr->live=true;
				}
        }
    }catch(...){throw;};
}
void TraceEditOperator::set_operator_defaults()
{
	decon_nspike_key="decon.nspike";
	decon_rawsnr_key="decon.rawsnr";
	decon_averamp_key="decon.averamp";
	decon_epsilon_key="decon.epsilon";
	decon_niteration_key="decon.niteration";
	decon_peakamp_key="decon.peakamp";
	FA_sensitivity=10e-5;
	data_shaping_wavelet_type="GAUSSIAN";
}
/*Default constructor*/
TraceEditOperator::TraceEditOperator()
{
	this->set_operator_defaults();
}

/* Construct from a Metadata object. */
TraceEditOperator::TraceEditOperator(Metadata& md)
{
	try
	{
		//read in default parameters from metadata. decon_key_definition is a table
		//, i.e., MetadataList
		this->set_operator_defaults();
		FA_sensitivity=md.get_double("FA_sensitivity");
		data_shaping_wavelet_type=md.get_string("data_shaping_wavelet_type");
		bool use_decon_in_editing=md.get_bool("use_decon_in_editing");
		if(use_decon_in_editing)
		{
			decon_nspike_key=md.get_string("decon_nspike_key");
			decon_rawsnr_key=md.get_string("decon_rawsnr_key");
			decon_averamp_key=md.get_string("decon_averamp_key");
			decon_epsilon_key=md.get_string("decon_epsilon_key");
			decon_niteration_key=md.get_string("decon_niteration_key");
			decon_peakamp_key=md.get_string("decon_peakamp_key");
		}
	}catch(...){throw;}
}
