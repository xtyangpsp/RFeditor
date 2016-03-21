#include <iostream>
#include <X11/Xthreads.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include "SeismicPlot.h"
/* This is needed here because this class is also defined in this file.  
   This was necessary to allow both classes to use the BuildMenu procedure. */
#include "TraceEditPlot.h"
#include "TraceEditOperator.h"
using namespace std;
using namespace SEISPP;
#define APP_CLASS "seismicplot"
// this constant cloned from dbxcor
#define MAINFORM_GRID_CNT 15
/* This is cloned from dbxcor.  It is used as a compact data structure to sent to 
   a procedure immediately below (BuildMenu) that constructs a widget based on this specification*/
typedef struct _menu_item
{
        char              *label;          /* the label for the item */
        WidgetClass       *class1;          /* pushbutton, label, separator... */
        char               mnemonic;       /* mnemonic; NULL if none */
        char              *accelerator;    /* accelerator; NULL if none */
        char              *accel_text;     /* to be converted to compound string */
        void             (*callback)(Widget,void *,void *);    /* routine to call; NULL if none */
        XtPointer          callback_data;  /* client_data for callback() */
        Widget           w;
        struct _menu_item *subitems;       /* pullright menu items, if not NULL */
} MenuItem;

/* Pulldown menus are built from cascade buttons, so this function
** also includes pullright menus. Create the menu, the cascade button
** that owns the menu, and then the submenu items.

This is a copy from dbxcor.  I think it originally came from some X cookbook. 
*/
Widget BuildMenu (Widget parent, int menu_type, char *menu_title, char menu_mnemonic,
                  Boolean tear_off, MenuItem *items)
{
        Widget   menu, cascade, widget;
        int      i;
        XmString str;
        Arg      args[8];
        int      n;

        if (menu_type == XmMENU_PULLDOWN)
                menu = XmCreatePulldownMenu (parent,(char *) "_pulldown", NULL, 0);
        else {
                n = 0;
                XtSetArg (args[n], XmNpopupEnabled, XmPOPUP_AUTOMATIC_RECURSIVE); n++;
                menu = XmCreatePopupMenu (parent, (char *) "_popup", args, n);
        }

        if (tear_off)
                XtVaSetValues (menu, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);

        if (menu_type == XmMENU_PULLDOWN) {
                str = XmStringCreateLocalized (menu_title);
                n = 0;
                XtSetArg (args[n], XmNsubMenuId, menu); n++;
                XtSetArg (args[n], XmNlabelString, str); n++;
                XtSetArg (args[n], XmNmnemonic, menu_mnemonic); n++;
                cascade = XmCreateCascadeButtonGadget (parent, menu_title, args, n);
                XtManageChild (cascade);
                XmStringFree (str);
        }

        /* Now add the menu items */
        for (i = 0; items[i].label != NULL; i++) {
                /* If subitems exist, create the pull-right menu by calling this
                ** function recursively. Since the function returns a cascade
                ** button, the widget returned is used..
                */

                if (items[i].subitems) {
                        widget = BuildMenu (menu, XmMENU_PULLDOWN, items[i].label,
                                                items[i].mnemonic, tear_off, items[i].subitems);
			items[i].w=widget;
                } else {
                        widget = XtVaCreateManagedWidget (items[i].label, *items[i].class1, menu, NULL);
                        items[i].w=widget;
                }

                /* Whether the item is a real item or a cascade button with a
                ** menu, it can still have a mnemonic.
                */
                if (items[i].mnemonic)
                        XtVaSetValues (widget, XmNmnemonic, items[i].mnemonic, NULL);

                /* any item can have an accelerator, except cascade menus. But,
                ** we don't worry about that; we know better in our declarations.
                */
                if (items[i].accelerator) {
                        str = XmStringCreateLocalized (items[i].accel_text);

                        XtVaSetValues (widget, XmNaccelerator, items[i].accelerator, XmNacceleratorText,
                                       str, NULL);
                        XmStringFree (str);
                }
                if (items[i].callback) {
                        String resource;

                        if (XmIsToggleButton(widget) || XmIsToggleButtonGadget(widget))
                                resource = XmNvalueChangedCallback;
                        else
                                resource = XmNactivateCallback;

                        XtAddCallback(widget, resource, items[i].callback,(XtPointer) items[i].callback_data);
                }
        }

        return (menu_type == XmMENU_PULLDOWN) ? cascade : menu ;
}
//save selected trace to file as timeseries
void save_timeseries_to_file_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *edit_plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	edit_plot_handle->select_trace_typename="SAVE_TIMESERIES_TO_FILE";
	edit_plot_handle->add_select_trace_callback();
}
void continue_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    TraceEditPlot *edit_plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	edit_plot_handle->report_kills();   //report kills when saving and quiting the display.
	if(edit_plot_handle->killed_review_mode) 
		edit_plot_handle->exit_review_killed_traces();
	edit_plot_handle->reset_local_variables();
    SeismicPlot *plot_handle=reinterpret_cast<SeismicPlot *>(client_data);
    for(int i=0; i<3; ++i) plot_handle->original_ensemble[i].member.clear();
    plot_handle->ExitDisplay();
}
//cleanly quit program. Xiaotao Yang 1/201516/
void quit_without_save(Widget w, XtPointer client_data, XtPointer call_data)
{
	SeismicPlot *plot_handle=reinterpret_cast<SeismicPlot *>(client_data);
    plot_handle->~SeismicPlot();
    cerr<<"Quiting program..."<<endl;
	exit(-1);
}
/* This is a private method used by both constructors.  It provides
   a convenient way to standardize this without requiring a copy
   operation which would be deadly here. */
void SeismicPlot::initialize_widgets(Metadata& md)
{
    try{
        ThreeComponentMode=md.get_bool("ThreeComponentMode");
        comp0=NULL;  comp1=NULL;  comp2=NULL;
        argc=1;
        argv[0]=strdup("SeismicPlotWidget");
        XtSetLanguageProc(NULL, NULL, NULL);
        XtToolkitThreadInitialize();
        toplevel = XtVaAppInitialize(&AppContext, (char *)"seismicplot",NULL,0,
                                &argc,argv, NULL,NULL);
        main_w=XmCreateForm(toplevel,(char *) "seismicplot",NULL,0);
        EventLoopIsActive=false;
        menu_bar=XmCreateMenuBar(main_w,(char *)"menuBar",NULL,0);
        XtVaSetValues (menu_bar, XmNtopAttachment,  XmATTACH_FORM, XmNleftAttachment, 
        				XmATTACH_FORM,XmNrightAttachment, XmATTACH_FORM, NULL);
        MenuItem file_menu[]={
            {(char *) "<Save Picked-Trace To File >",&xmPushButtonGadgetClass,'T', 
            //Modified by Xiaotao Yang 1/16/2015
            //(char *) "Exit Event Loop",&xmPushButtonGadgetClass,'x',
			(char *)"Ctrl<Key>T",(char *)"Ctrl+T",
			save_timeseries_to_file_callback,this,NULL,(MenuItem *)NULL},
			{(char *) "Save & Go Next",&xmPushButtonGadgetClass,'G', 
            //Modified by Xiaotao Yang 1/16/2015
            //(char *) "Exit Event Loop",&xmPushButtonGadgetClass,'x',
			(char *)"<Key>G",(char *)"G",
			continue_callback,this,NULL,(MenuItem *)NULL},
			{(char *) "Quit Without Save",&xmPushButtonGadgetClass,'Q', 
			//Modified by Xiaotao Yang 1/16/2015
			(char *)"Ctrl<Key>Q",(char *)"Ctrl+Q",
			quit_without_save,this,NULL,(MenuItem *)NULL},
			{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
		};
        menu_file=BuildMenu(menu_bar,XmMENU_PULLDOWN,
                (char *)"File",'F',false,file_menu);
        XtManageChild(menu_bar);
        /* We create a secondary form which will hold the 3 seismic components within it in 
           a paned window.  We'll create that pair now one after the other. */
        Arg args[4];
        int n=0;
        XtSetArg(args[n],XmNfractionBase,MAINFORM_GRID_CNT); n++;
        slrc = XmCreateForm(main_w, (char *) "form", args,n);
        XtVaSetValues (slrc,
                 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
                 XmNtopAttachment,  XmATTACH_WIDGET,
		 XmNtopWidget, menu_bar,
                 XmNbottomAttachment, XmATTACH_FORM,
                 NULL);
        XtManageChild(slrc);
        paned_win=XtVaCreateWidget((char *)"pane",
        	xmPanedWindowWidgetClass,slrc,
        	XmNorientation,			XmVERTICAL,
        	XmNseparatorOn,			True,
        	XmNtopAttachment,		 XmATTACH_POSITION,
        	XmNtopPosition,		 	0,
        	XmNleftAttachment,		 XmATTACH_POSITION,
        	XmNleftPosition,		 0,
        	XmNrightAttachment,		 XmATTACH_POSITION,
        	XmNrightPosition,		 MAINFORM_GRID_CNT,
        	XmNbottomAttachment,		 XmATTACH_POSITION,
        	XmNbottomPosition,		MAINFORM_GRID_CNT-1,
        	NULL);
        XtManageChild(paned_win);
        /* Now build empty seisw widgets */
        n=0;
        XtSetArg(args[n],(char *) ExmNdisplayOnly,1); n++;
        XtSetArg(args[n],(char *) ExmNzoomFactor,100); n++;
        int wbox=md.get_int("wbox");
        XtSetArg(args[n],XmNpaneMaximum,wbox+50); n++;
        XtSetArg(args[n],XmNpaneMinimum,100); n++;
        seisw[0]=ExmCreateSeisw(paned_win,(char *)"Seisw0",args,n);
        XtManageChild(seisw[0]);
        if(ThreeComponentMode)
        {
            seisw[1]=ExmCreateSeisw(paned_win,(char *)"Seisw1",args,n);
            XtManageChild(seisw[1]);
            seisw[2]=ExmCreateSeisw(paned_win,(char *)"Seisw2",args,n);
            XtManageChild(seisw[2]);
        }
        else
        {
            seisw[1]=NULL;
            seisw[2]=NULL;
        }
        XtManageChild(main_w);
        XtRealizeWidget(toplevel);
    } catch (...){throw;}
}
/* Default constructor gets its parameter information from the
standard antelope parameter file location under a frozen name 
(SeismicPlot.pf).  */
SeismicPlot::SeismicPlot()
{
    cerr << "Entered default constructor"<<endl;
    Pf *pf;
    const string pfglobal_name("SeismicPlot");
    if(pfread(const_cast<char *>(pfglobal_name.c_str()),&pf))
        throw SeisppError("SeismicPlot constructor:  pfread failed for "
                + pfglobal_name);
    try {
	Metadata md(pf);
	pffree(pf);
        this->initialize_widgets(md);
        EventLoopIsActive=false;
        block_till_exit_pushed=true;   // default 
    } catch (...) {throw;}
}
/* Construct from a Metadata object. */
SeismicPlot::SeismicPlot(Metadata& md) : Metadata(md)
{
    try {
        this->initialize_widgets(md);
        EventLoopIsActive=false;
        block_till_exit_pushed=true;
    } catch(...) {throw;}
}
/* Destructor  is not trivial here */
SeismicPlot::~SeismicPlot()
{
    cerr << "Entering destructor"<<endl;
    /* This may not be necessary, but probably prudent to avoid a deadlock */
    if(EventLoopIsActive) this->ExitDisplay();
    if(comp0!=NULL) delete comp0;
    if(comp1!=NULL) delete comp1;
    if(comp2!=NULL) delete comp2;
    /* As I read it destroy the top level widget and destroy all the children. 
       Hopefully this will not create a seg fault */
    /* Using this creates problems.  Web research suggests this is probably 
       because motif widgets are built with their own destructors.  With this
       the program seg faults when this destructor is called. 
    XtDestroyWidget(toplevel);
    */
}
void SeismicPlot::plot(TimeSeriesEnsemble& d,bool block_for_event)
{
    try{
    //manually put killmethodkey metadata.
    vector<TimeSeries>::iterator iptr;
    for(iptr=d.member.begin();iptr!=d.member.end();++iptr)
    {
    	iptr->put(killmethodkey, string("Manual"));
    }
    
    this->original_ensemble[0]=d;
    cerr<<"> ";
    //cerr<<original_ensemble[0].member.size()<<endl;
	XtAppLock(AppContext);
        /* Make an internal copy of d managed by the object to be 
           consistent with 3C data.  We intentionally do not manage
           the comp1 and comp2 pointers.   */
        if(comp0!=NULL) delete comp0;
        comp0=new TimeSeriesEnsemble(d);
     //debug
     //cout<<"test 0"<<endl;   
        plotboxmarkers.beam_tw=TimeWindow(d.member[0].t0,d.member[0].endtime());
		plotboxmarkers.beam_color="red";
		plotboxmarkers.robust_tw=plotboxmarkers.beam_tw;
		plotboxmarkers.robust_color="blue";
		plotboxmarkers.title="plotbox";
        
        XtVaSetValues(seisw[0],ExmNseiswEnsemble, (XtPointer) (comp0),
            ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), 
            ExmNdisplayMarkers,(XtPointer)(&(plotboxmarkers)),NULL);
		//debug
     ///cout<<"test 00"<<endl;
		XtAppUnlock(AppContext);
	//debug
     ///cout<<"test 000"<<endl;
        if(block_for_event) 
            block_till_exit_pushed=true;
        else
            block_till_exit_pushed=false;
        if(!EventLoopIsActive) 
            this->launch_Xevent_thread_handler();
    //debug
     //cout<<"test 0000"<<endl;
    }catch(SeisppError& serr)
    {cerr<<"**Error when plotting data."<<endl;
    	serr.what();
    	exit(-1);
    }
}
void SeismicPlot::refresh()
{
    try {
	XtAppLock(AppContext);
        int i;
        for(i=0;i<3;++i)
        {
            if(seisw[i]!=NULL) XtVaSetValues(seisw[i],ExmNseiswMetadata, 
                    (XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
        }
        XtAppUnlock(AppContext);
    }catch(...) {throw;}
}
/* TimeSeries and ThreeComponentSeismograms are plotted in window 1
   using the same method as above.  Do this by simply creating a temporary
   ensemble object and calling that method */
void SeismicPlot::plot(TimeSeries& d,bool block_for_event)
{
    try {
        TimeSeriesEnsemble e;
        e.member.push_back(d);
        this->original_ensemble[0]=e;
        this->plot(e,block_for_event);
    } catch(...){throw;}
}
void SeismicPlot::plot(ThreeComponentSeismogram& d,bool block_for_event)
{
    try {
        ThreeComponentEnsemble dtmp;
        dtmp.member.push_back(d);
        /*this assumes dtmp will be copied to 3 components "comp" in private 
          are of this object.  This is a potential maintenance issue if that implementation
          changes to be aware */
        this->plot(dtmp,block_for_event);
    }catch(...){throw;}
}
void SeismicPlot::plot(ThreeComponentEnsemble& d,bool block_for_event)
{
    try {
        if(!ThreeComponentMode) throw SeisppError(string("SeismicPlot::plot method:  ")
                + "Trying to plot 3c mode with ThreeComponentMode not set.  Fix pf");
        cerr << "Entering 3c plot method"<<endl;
        int k;
	XtAppLock(AppContext);
        for(k=0;k<3;++k)
        {
            auto_ptr<TimeSeriesEnsemble> c;
            c=ExtractComponent(d,k);
            /* This proved necessary to get around a mysterious behaviour I never figured out
               if I tried to save the raw auto_ptr. I suspect it is related to the single owner
               property of an auto_ptr which does not mesh well with passing data to the widget or
               keeping it stored in this object.  We thus do the very inefficient thing and copy
               the ensemble to a raw pointer.  */
            TimeSeriesEnsemble *tstmp=new TimeSeriesEnsemble(*c);
            this->original_ensemble[k]=*tstmp;
            switch (k)
            {
                case 1:
                    if(comp1!=NULL) delete comp1;
                    comp1=tstmp;
                    XtVaSetValues(seisw[k],ExmNseiswEnsemble, (XtPointer) (comp1),
                        ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
                    break;
                case 2:
                    if(comp2!=NULL) delete comp2;
                    comp2=tstmp;
                    XtVaSetValues(seisw[k],ExmNseiswEnsemble, (XtPointer) (comp2),
                        ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
                    break;
                case 0:
                default:
                    if(comp0!=NULL) delete comp0;
                    comp0=tstmp;
                    XtVaSetValues(seisw[k],ExmNseiswEnsemble, (XtPointer) (comp0),
                        ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
            }
        }

	XtAppUnlock(AppContext);
        if(block_for_event) 
            block_till_exit_pushed=true;
        else
            block_till_exit_pushed=false;
        if(!EventLoopIsActive) 
            this->launch_Xevent_thread_handler();
    }catch(...){throw;}
}
void EventHandler(SeismicPlot *plot_handle)
{
    XtAppLock(plot_handle->AppContext);
        do {
            XEvent event;
            XtAppNextEvent(plot_handle->AppContext,&event);
            XtDispatchEvent(&event);
        }while (plot_handle->WindowIsActive());
    XtAppUnlock(plot_handle->AppContext);
}
void SeismicPlot::launch_Xevent_thread_handler()
{
    EventLoopIsActive=true;  // This may need a mutex
    boost::thread Xevthrd(boost::bind(&EventHandler,this));
    if(block_till_exit_pushed)
    {
        Xevthrd.join();
        EventLoopIsActive=false;
    }
}
/*
*******************************************
*******************************************
***** BEGIN TraceEditPlot code ************
*******************************************
*******************************************
*/
/* This is a pair of labels that are posted on the edit menu 
   displaying edit mode.  Edit is a toggle and this label will change
   back and forth between these two codes */
const string edit_menu_label("Edit");
const string cutlabel("Single Trace Edit (ST)");
const string stlabel("Cutoff Edit Mode (CO)");
const string edit_mode_ST("Edit (ST)");
const string edit_mode_CO("Edit (CO)");
const string prefix_message("> ");

void print_help_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->help();
}
void TraceEditPlot::help()
{
	//this->print_prefix();
	cerr<<endl
	<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Tips <<<<<<<<<<<<<<<<<<<<<<<<<"<<endl
	<<"**NOTE: this help message may be out-of-date. **"<<endl
	<<"1. Use mouse botton 2 (middle button) to edit traces or to pick reference trace."<<endl
	<<"2. File"<<endl
	<<"3. Edit"<<endl
	<<"  (a) Manual editing modes"<<endl
	<<"    I:"<<endl<<"	Manual editing by single trace."<<endl
	<<"    O:"<<endl<<"	Manual editing by cutoff traces below the picking trace."<<endl
	<<"  (b) Trace-Based Procedures"<<endl
	<<"    A: Apply all Trace-Based Procedures excluding LFC procedure."<<endl
	<<"  (c) Decon-Based Procedures"<<endl
	<<"    D: Apply all cutoffs on all Decon-Attributes excluding DSI cutoff, which is listed seperately."<<endl
	<<"  (d) DSI Cutoff"<<endl
	<<"  (e) Other Procedures"<<endl
	<<"4. View"<<endl
	<<"5. Sort"<<endl
	<<"  (a) R: By Reference Trace"<<endl
	<<"      Go to 'Sort->Sort By Reference Trace'."<<endl
	<<"        P: 'Pick Reference Trace' before first <Apply SBRT>."<<endl
	<<"      K: Use Robust stack trace as reference trace."<<endl
	<<"  (b) W: By Stack Weight:"<<endl
	<<"      Go to 'Sort->Sort By Stack Weight'."<<endl
	<<"      You can customize the robust time window (optional) before <Apply SBSW>."<<endl
	<<"6. Tools"<<endl;
	this->print_prefix();
}

/* This callback does nothing.  It is purely a placeholder required
   by the X interface to make btn2 active.  At least I don't see a way
   to avoid it.  This is null because the widget knows about time series 
   objects and the concept of the live boolean.  It has wired in it a
   feature that toggles the live varibles when btn2 is pushed. */
void ecb(Widget w, XtPointer client_data, XtPointer call_data)
{
    cerr << "ecb callback entered"<<endl;
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->killed_trace_excluded=false;
}
void TraceEditPlot::backup_original_tse()
{
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try
	{
		for(i=0;i<kmax;++i)
		{
			if(!tse_backup_set[i])
			{
					TimeSeriesEnsemble * sensemble;
					XtVaGetValues(this->seisw[0],ExmNseiswEnsemble,&sensemble,NULL);
					original_ensemble[i]=*sensemble;
					tse_backup_set[i]=true;
					cerr<<"TimeSeriesEnsemble backup set, ensemble size = "
						<<original_ensemble[i].member.size()<<endl;
			}
		}
	}catch(...){throw;};
}
void TraceEditPlot::backup_undo_tse()
{
	size_of_statistics=teo_global.sizeof_statistics();
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			TimeSeriesEnsemble * sensemble;
			XtVaGetValues(this->seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
			switch(i)
			{
				case 0:
					tse_bkp_for_undo0=new TimeSeriesEnsemble(*sensemble);
					break;
				case 1:
					tse_bkp_for_undo1=new TimeSeriesEnsemble(*sensemble);
					break;
				case 2:
					tse_bkp_for_undo2=new TimeSeriesEnsemble(*sensemble);
					break;
			}
		}
		XtAppUnlock(AppContext);
	}catch(...){throw;};
}
void ecb_editoff(Widget w, XtPointer client_data, XtPointer call_data)
{
    cerr << "Edit is disabled."<<endl<<"> ";
}

void TraceEditPlot::print_prefix()
{
	cerr<<prefix_message;
}

void TraceEditPlot::edit_enable()
{
    /*Always initialize the widget in single trace mode which is 
      what editon==1 means.  Set to 2 to enable cutoff edit function */
    int editon(1);
    this->allow_edit=true;
    for(int k=0;k<3;++k)
    {
        if(seisw[k]!=NULL)
        {
            XtVaSetValues(seisw[k],ExmNeditEnable,editon,NULL);
            XtRemoveAllCallbacks(seisw[k],ExmNbtn2Callback);
            // data pointer or this callback is intentionally NULL because
            // callback does nothing
            XtAddCallback(seisw[k],ExmNbtn2Callback,ecb,this);
        }
    }
}

//enable edit mode to trace widget.
//edit_enable() is a private method. this function calls it to enable edit.
void TraceEditPlot::edit_enable_public()   
{ 
	this->edit_enable();
}

void edit_enable_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->edit_enable_public();
}

void TraceEditPlot::edit_disable()
{
    /*Always initialize the widget in single trace mode which is 
      what editon==1 means.  Set to 2 to enable cutoff edit function */
    int editon(0);
    this->allow_edit=false;
    for(int k=0;k<3;++k)
    {
        if(seisw[k]!=NULL)
        {
            XtVaSetValues(seisw[k],ExmNeditEnable,editon,NULL);
            XtRemoveAllCallbacks(seisw[k],ExmNbtn2Callback);
			XtVaSetValues(this->menu_edit,XmNlabelString,
					XmStringCreateLocalized(const_cast<char *>(edit_menu_label.c_str())),NULL);
            // data pointer or this callback is intentionally NULL because
            // callback does nothing
            XtAddCallback(seisw[k],ExmNbtn2Callback,ecb_editoff,NULL);
        }
    }
}

//disable edit mode to trace widget.
//edit_disable() is a private method. this function calls it to disable edit.
void TraceEditPlot::edit_disable_public()   
{ 
	this->edit_disable();
}

void edit_disable_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->edit_disable_public();
}

//Start of ********************************************************************
//toggle edit mode and edit menu.
void toggle_edit_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->toggle_edit_mode();
}
void TraceEditPlot::toggle_edit_mode()
{
    int k;
    XmString str;
    int kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    for(k=0;k<kmax;++k)
    {
			XtRemoveAllCallbacks(this->seisw[k],ExmNbtn2Callback);
			/* In each block below this is a toggle.  i.e. if one mode
			   switch to the other */
		if(edit_mode==SINGLE_TRACE_EDIT_MODE)
		{
				edit_mode=CUTOFF_EDIT_MODE;
				//DEBUG. Xiaotao Yang
				cout<<"Switched to cutoff edit mode."<<endl;
				str = XmStringCreateLocalized(const_cast<char *>(cutlabel.c_str()));
		}
		else if(edit_mode==CUTOFF_EDIT_MODE)
		{
				edit_mode=SINGLE_TRACE_EDIT_MODE;
				//DEBUG. Xiaotao Yang
				cout<<"Switched to single trace edit mode."<<endl;
				str = XmStringCreateLocalized(const_cast<char *>(stlabel.c_str()));
		}
		else
		{
			/* Perhaps should throw an exception here, but instead we force
			   default of single trace edit mode if edit_mode is mangled.
			   This makes the result more robust but potentially mysterious
			   if edit_mode were overwritten */
				edit_mode=SINGLE_TRACE_EDIT_MODE;
				str = XmStringCreateLocalized(const_cast<char *>(stlabel.c_str()));
		}
        XtVaSetValues(this->seisw[k],ExmNeditEnable,edit_mode,NULL);
        /* This is currently a do nothing callback.  In edit mode nothing
           really needs to be reported with seisw as the plot is updated with
           the killed traces zeroed and displayed red. */
        XtAddCallback(this->seisw[k],ExmNbtn2Callback,ecb,this);
        /* Set the label for the edit menu whether we toggle or not. */
        
        XtVaSetValues(this->menu_edit,XmNlabelString,str,NULL);
        }
}
//Xiaotao Yang 1/20/2015
void turn_on_single_trace_mode_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->turn_on_single_trace_mode();
}
void turn_on_cutoff_mode_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->turn_on_cutoff_mode();
}
void TraceEditPlot::turn_on_single_trace_mode()
{
	int k;
    XmString str;
    int kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    for(k=0;k<kmax;++k)
    {
		edit_mode=SINGLE_TRACE_EDIT_MODE;
		//DEBUG. Xiaotao Yang
		cout<<"Switched to single trace edit mode (ST)."<<endl;
		cerr<<"> ";
		str=XmStringCreateLocalized(const_cast<char *>(edit_mode_ST.c_str()));
		XtVaSetValues(this->seisw[k],ExmNeditEnable,edit_mode,NULL);
		/* This is currently a do nothing callback.  In edit mode nothing
		   really needs to be reported with seisw as the plot is updated with
		   the killed traces zeroed and displayed red. */
		XtAddCallback(this->seisw[k],ExmNbtn2Callback,ecb,this);
		/* Set the label for the edit menu whether we toggle or not. */
	
		XtVaSetValues(this->menu_edit,XmNlabelString,str,NULL);
	}
}
//Xiaotao Yang 1/20/2015
void TraceEditPlot::turn_on_cutoff_mode()
{
	int k;
    XmString str;
    int kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    for(k=0;k<kmax;++k)
    {
		edit_mode=CUTOFF_EDIT_MODE;
		//DEBUG. Xiaotao Yang
		cout<<"Switched to cutoff edit mode (CO)."<<endl;
		cerr<<"> ";
		str=XmStringCreateLocalized(const_cast<char *>(edit_mode_CO.c_str()));						
		XtVaSetValues(this->seisw[k],ExmNeditEnable,edit_mode,NULL);
		/* This is currently a do nothing callback.  In edit mode nothing
		   really needs to be reported with seisw as the plot is updated with
		   the killed traces zeroed and displayed red. */
		XtAddCallback(this->seisw[k],ExmNbtn2Callback,ecb,this);
		/* Set the label for the edit menu whether we toggle or not. */
	
		XtVaSetValues(this->menu_edit,XmNlabelString,str,NULL);
	}
}
//End of toggle edit mode and edit menu *******************************************

//Start of editing procedures.
void apply_all_autokill_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	if(plot_handle->allow_edit)
		plot_handle->apply_all_autokill();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_all_autokill()
{
	this->backup_undo_tse();
		undo_for_kill=true;
	//backup original data.
	killed_trace_excluded=false;
	//this->backup_original_tse();
	TimeSeriesEnsemble *comp_tmp;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    try 
    {
		XtAppLock(AppContext);
        for(i=0;i<kmax;++i)
        {
            if(seisw[i]!=NULL)
            {
				TimeSeriesEnsemble * sensemble;
				set<long> tempkill_list;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);	
				//cout<<"knfa size 0: "<<last_autokill_list.size()<<endl;
				if(NFA_tolerance_window.length()>0)
					last_autokill_list=teo_global.kill_negative_FAs_traces(
							*sensemble,NFA_tolerance_window);
				else
				{
					cerr<<"ERROR in applying kill_negative_FAs_traces:"
						<<"length of the NFA_tolerance_window has to be greater than 0.0."<<endl;
					last_autokill_list.clear();
				}
				//DEBUG
				//cout<<"knfa size: "<<last_autokill_list.size()<<endl;
				tempkill_list=teo_global.kill_large_amplitude_traces(
							*sensemble,max_trace_abs_amplitude);
				if(tempkill_list.size()>0)
				{	
					last_autokill_list.insert(tempkill_list.begin(),
												tempkill_list.end());
				}
				/*
				tempkill_list=teo_global.kill_small_FA_traces(*sensemble,
							min_normalized_FA_amplitude,FA_search_window,
							amplitude_normalization_method,FA_detect_length);
				if(tempkill_list.size()>0)
				{	
					last_autokill_list.insert(tempkill_list.begin(),
												tempkill_list.end());
				}
				*/
				//Temporarily remove kill LFC from apply_all.
				/*
				tempkill_list=teo_global.kill_LowFrequencyContaminated_traces(
							*sensemble,LowFrequency_min, LowFrequency_max);
				if(tempkill_list.size()>0)
				{	
					last_autokill_list.insert(tempkill_list.begin(),
												tempkill_list.end());
				}
				*/
				//if(FA_search_window.length()>MYZERO &&
				//	PCoda_search_window.length()>MYZERO)
				
				if(PCoda_search_window.length()>MYZERO)
				{		
					tempkill_list=teo_global.kill_growing_PCoda_traces(*sensemble,
							PCoda_search_window,PCoda_grow_tolerance);
					if(tempkill_list.size()>0)
					{	
						last_autokill_list.insert(tempkill_list.begin(),
													tempkill_list.end());
					}
				}
				if(CodaCA_tolerance_twin_length>MYZERO &&
					CodaCA_search_window.length()>MYZERO)
				{		
					tempkill_list=teo_global.kill_ClusteredArrivals_traces(*sensemble,
							CodaCA_search_window,CodaCA_tolerance_twin_length);
					if(tempkill_list.size()>0)
					{	
						last_autokill_list.insert(tempkill_list.begin(),
													tempkill_list.end());
					}
				}
				cerr<<"TraceEditPlot: number of auto-killed traces (No LFC) = "
					<<last_autokill_list.size()<<endl;
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
            }
        }
        XtAppUnlock(AppContext);
    }catch(...) {throw;};	
}
void customize_kill_negative_FA_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data); 
	plot_handle->customize_kill_negative_FA_trace();
}
void TraceEditPlot::customize_kill_negative_FA_trace()
{
	this->timewindow_typename="NFA_TOLERANCE_WINDOW";
	this->customize_time_window_by_WINDOWTYPE();
	//cerr<<"FA search length after first non-zero point: ";
	//cin>>this->FA_detect_length;
	cerr<<"> Apply 'Kill Negative FAs Traces' procedure? (y/n) ";
	string ctemp;
	cin>>ctemp;
	if(ctemp=="y" || ctemp=="Y")
	{	
		this->apply_kill_negative_FA_trace();
		cerr<<"> ";
	}
	else
		cerr<<"> ";
}
void apply_kill_negative_FA_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data); 
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_negative_FA_trace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}

void TraceEditPlot::apply_kill_negative_FA_trace()
{
	this->backup_undo_tse();
		undo_for_kill=true;
	killed_trace_excluded=false;
	//backup original data.
	this->backup_original_tse();
	
	TimeSeriesEnsemble *comp_tmp;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    try 
    {
		XtAppLock(AppContext);
        for(i=0;i<kmax;++i)
        {
            if(seisw[i]!=NULL)
            {
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(NFA_tolerance_window.length()>0)
					last_autokill_list=teo_global.kill_negative_FAs_traces(
							*sensemble,NFA_tolerance_window);
				else
				{
					cerr<<"ERROR in applying kill_negative_FAs_traces:"
						<<"length of the NFA_tolerance_window has to be greater than 0.0."<<endl;
					last_autokill_list.clear();
				}
				cerr<<"TraceEditPlot: number of negative traces killed = "
					<<last_autokill_list.size()<<endl;
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
            }
        }
        XtAppUnlock(AppContext);
    }catch(...) {throw;};
}
//kill_large amplitude traces
void customize_kill_large_amplitude_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->customize_kill_large_amplitude_trace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_large_amplitude_trace()
{
	cerr<<"Please type in your parameters for procedure <Kill Large Amplitude Trace>."<<endl;
	cerr<<"Amplitude cutoff threshold (maximum abs amplitude): ";
	cin>>max_trace_abs_amplitude;
	cerr<<"> Customization completed!"<<endl
		<<"Applying <Kill Large Amplitude Trace> ..."<<endl;
	
	this->apply_kill_large_amplitude_trace();
	
}
void apply_kill_large_amplitude_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_large_amplitude_trace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_large_amplitude_trace()
{
	this->backup_undo_tse();
		undo_for_kill=true;
	killed_trace_excluded=false;
	//backup original data.
	this->backup_original_tse();
	
	TimeSeriesEnsemble *comp_tmp;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    try 
    {
		XtAppLock(AppContext);
        for(i=0;i<kmax;++i)
        {
            if(seisw[i]!=NULL)
            {
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=
					teo_global.kill_large_amplitude_traces(
							*sensemble,max_trace_abs_amplitude);
				cerr<<"TraceEditPlot: number of large traces killed = "
					<<last_autokill_list.size()<<endl;
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
            }
        }
        XtAppUnlock(AppContext);
    }catch(...) {throw;};
}
//kill_small_FA traces
void customize_kill_small_FA_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->customize_kill_small_FA_trace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_small_FA_trace()
{
	cerr<<"Please type in your parameters for procedure <Kill Small FA Trace>."<<endl;
	cerr<<"FA cutoff threshold (normalized minimum): ";
	cin>>min_normalized_FA_amplitude;
	cerr<<"FA amplitude normalization method "<<endl;
		
	int itmp=-1;
	while(itmp==-1)
	{
		cout<<"1-LOCAL_MAX; "<<endl
			<<"2-GLOBAL_MAX; "<<endl
			<<"3-GLOBAL_FA_MAX. "<<endl<<"> Your choice: ";
		cin>>itmp;
		switch(itmp)
		{
			case 1:
				amplitude_normalization_method="LOCAL_MAX";
				break;
			case 2:
				amplitude_normalization_method="GLOBAL_MAX";
				break;
			case 3:
				amplitude_normalization_method="GLOBAL_FA_MAX";
				break;
			default:
				cerr<<"Unknown nomalization method. Please re-choose from below."<<endl;
				itmp=-1;
		}
	}
	this->timewindow_typename="FA_SEARCH_WINDOW";
	this->customize_time_window_by_WINDOWTYPE();
	cerr<<"FA search length after first non-zero point: ";
	cin>>this->FA_detect_length;
	cerr<<" Apply 'Kill Small FA Traces' procedure? (y/n) ";
	string ctemp;
	cin>>ctemp;
	if(ctemp=="y" || ctemp=="Y")
	{	this->apply_kill_small_FA_trace();
		cerr<<"> ";
	}
	else
		cerr<<"> ";
}
void apply_kill_small_FA_trace_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_small_FA_trace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_small_FA_trace()
{
	this->backup_undo_tse();
		undo_for_kill=true;
	killed_trace_excluded=false;
	//backup original data.
	//this->backup_original_tse();
	
	TimeSeriesEnsemble *comp_tmp;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;
    try 
    {
		XtAppLock(AppContext);
        for(i=0;i<kmax;++i)
        {
            if(seisw[i]!=NULL)
            {
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=
					teo_global.kill_small_FA_traces(*sensemble,
							min_normalized_FA_amplitude,FA_search_window,
							amplitude_normalization_method,FA_detect_length);
				cerr<<"TraceEditPlot: number of small FA traces killed = "
					<<last_autokill_list.size()<<endl;
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
            }
        }
        XtAppUnlock(AppContext);
    }catch(...) {throw;};
}
void customize_kill_growing_PCoda_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data); 
	if(plot_handle->allow_edit)
	{	
		plot_handle->customize_kill_growing_PCoda();
	}
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
	
}
void TraceEditPlot::customize_kill_growing_PCoda()
{
	/*
	this->timewindow_typename="FA_SEARCH_WINDOW";
	this->customize_time_window_by_WINDOWTYPE();
	cerr<<"FA search length after first non-zero point: ";
	cin>>this->FA_detect_length;
	*/
	this->timewindow_typename="PCODA_SEARCH_WINDOW";
	this->customize_time_window_by_WINDOWTYPE();
	cerr<<"PCoda_grow_tolerance: ";
	cin>>this->PCoda_grow_tolerance;
	cerr<<"> Apply 'Kill Growing P-Coda Traces' procedure? (y/n) ";
	string ctemp;
	cin>>ctemp;
	if(ctemp=="y" || ctemp=="Y")
	{	this->apply_kill_growing_PCoda();
		cerr<<"> ";
	}
	else
		cerr<<"> ";	
}
void apply_kill_growing_PCoda_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_growing_PCoda();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_growing_PCoda()
{
	if(this->PCoda_search_window.length()>MYZERO ) //&&
		//this->FA_search_window.length()>MYZERO)
	{
		this->backup_undo_tse();
		undo_for_kill=true;
		killed_trace_excluded=false;
		//backup original data.
		//this->backup_original_tse();
	
		TimeSeriesEnsemble *comp_tmp;
		//stringstream ss;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					TimeSeriesEnsemble * sensemble;
					//set<long> tmp;
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					last_autokill_list=
						teo_global.kill_growing_PCoda_traces(*sensemble,
								PCoda_search_window,PCoda_grow_tolerance);
					cerr<<"TraceEditPlot: number of growing P-coda traces killed = "
						<<last_autokill_list.size()<<endl;
					if(last_autokill_list.size()>0)
					{
						comp_tmp=new TimeSeriesEnsemble(*sensemble);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
							ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
			}
			XtAppUnlock(AppContext);
		}catch(...) {throw;};
    }
    else cerr<<"ERROR: P-Coda or FA Search Window not set. "<<endl
			<<"Can't apply procedure: apply_kill_growing_Pcoda()."
			<<endl;
}
void customize_kill_ClusteredArrivals_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data); 
	if(plot_handle->allow_edit)
		plot_handle->customize_kill_ClusteredArrivals();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_ClusteredArrivals()
{
	this->timewindow_typename="CODACA_SEARCH_WINDOW";
	this->customize_time_window_by_WINDOWTYPE();
	cerr<<"Input your tolerance window length (current = "<<CodaCA_tolerance_twin_length<<" ): ";
	cin>>CodaCA_tolerance_twin_length;
	cerr<<"> Apply 'Kill Clustered Arrivals Traces' procedure? (y/n) ";
	string ctemp;
	cin>>ctemp;
	if(ctemp=="y" || ctemp=="Y")
	{	this->apply_kill_ClusteredArrivals();
		cerr<<"> ";
	}
	else
		cerr<<"> ";
}
void apply_kill_ClusteredArrivals_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_ClusteredArrivals();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_ClusteredArrivals()
{
	if(this->CodaCA_tolerance_twin_length>MYZERO &&
		this->CodaCA_search_window.length()>MYZERO)
	{
		this->backup_undo_tse();
		undo_for_kill=true;
		killed_trace_excluded=false;
		//backup original data.
		//this->backup_original_tse();
	
		TimeSeriesEnsemble *comp_tmp;
		//stringstream ss;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					TimeSeriesEnsemble * sensemble;
					//set<long> tmp;
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					last_autokill_list=
						teo_global.kill_ClusteredArrivals_traces(*sensemble,
								CodaCA_search_window,CodaCA_tolerance_twin_length);
					cerr<<"TraceEditPlot: number of 'Clustered Arrivals' traces killed = "
						<<last_autokill_list.size()<<endl;
					if(last_autokill_list.size()>0)
					{
						comp_tmp=new TimeSeriesEnsemble(*sensemble);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
							ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
			}
			XtAppUnlock(AppContext);
		}catch(...) {throw;};
    }
    else cerr<<"ERROR: Coda Clustered Arrivals Search Window not set or "<<
    		"tolerance window length is zero. "<<endl
			<<"Can't apply procedure: apply_kill_ClusteredArrivals()."
			<<endl;
}
void customize_kill_LFC_traces_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->customize_kill_LFC_traces();
}
void TraceEditPlot::customize_kill_LFC_traces()
{
	cout<<"Current Low-Frequency range: < "
		<<LowFrequency_min<<", "<<LowFrequency_max<<" >."<<endl
		<<"> Please type-in your new range (min max): ";
	cin>>LowFrequency_min>>LowFrequency_max;
	cout<<"Your input: < "
		<<LowFrequency_min<<", "<<LowFrequency_max<<" >."<<endl;
	if(this->allow_edit)
	{
		cout<<"> Apply 'Kill Low-Frequency-Contaminated Traces' procedure? (y/n) ";
		string ctemp;
		cin>>ctemp;
		if(ctemp=="y" || ctemp=="Y")
		{	
			this->apply_kill_LFC_traces();
			cout<<"> ";
		}
		else
			cout<<"> ";
	}
	else
		cout<<"Edit is disabled. Customized parameters only!"<<endl<<"> ";
}
void apply_kill_LFC_traces_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_LFC_traces();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_LFC_traces()
{
	this->backup_undo_tse();
	undo_for_kill=true;
	killed_trace_excluded=false;
	//backup original data.
	//this->backup_original_tse();
	cerr<<"Current Low-Frequency range: < "
		<<LowFrequency_min<<", "<<LowFrequency_max<<" >."<<endl<<"> ";
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=
					teo_global.kill_LowFrequencyContaminated_traces(*sensemble,
							LowFrequency_min, LowFrequency_max);
				cerr<<"TraceEditPlot: number of 'Low-Frequency-Contaminated' traces killed = "
					<<last_autokill_list.size()<<endl;
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError& serr) 
	{cerr<<"Error in applying 'kill Low-Frequency-Contaminated traces: '"<<endl
		<<serr.what()<<endl;};
}
//a series of procedures of killing the trace by setting decon parameters
//threshold.
//costomizing decon threshold.
void customize_decon_threshold_for_all_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="ALLDECONPAR";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_niteration_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="niteration";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_nspike_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="nspike";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_epsilon_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="epsilon";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_peakamp_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="peakamp";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_averamp_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="averamp";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void customize_decon_threshold_rawsnr_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="rawsnr";
	plot_handle->customize_decon_threshold_by_PARNAME();
}
void TraceEditPlot::customize_decon_threshold_by_PARNAME()
{
	if(this->allow_edit)
	{
		cerr<<"Current Decon Parameter Thresholds -> "<<endl
			<<"niteration < "<<niteration_min<<", "<<niteration_max<<" >"<<endl
			<<"nspike < "<<nspike_min<<", "<<nspike_max<<" >"<<endl
			<<"epsilon < "<<epsilon_min<<", "<<epsilon_max<<" >"<<endl
			<<"peakamp < "<<peakamp_min<<", "<<peakamp_max<<" >"<<endl
			<<"averamp < "<<averamp_min<<", "<<averamp_max<<" >"<<endl
			<<"rawsnr < "<<rawsnr_min<<", "<<rawsnr_max<<" >"<<endl;
		cerr<<"Type in your decon threshold. Format: <parameter>: min max"<<endl;
		if(this->decon_kill_parname == "niteration")
		{
			cerr<<"<niteration>: ";
			cin>>niteration_min>>niteration_max;
			cerr<<"Your input: "<<niteration_min<<","<<niteration_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "nspike")
		{
			cerr<<"<nspike>: ";
			cin>>nspike_min>>nspike_max;
			cerr<<"Your input: "<<nspike_min<<","<<nspike_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "epsilon")
		{
			cerr<<"<epsilon>: ";
			cin>>epsilon_min>>epsilon_max;
			cerr<<"Your input: "<<epsilon_min<<","<<epsilon_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "peakamp")
		{
			cerr<<"<peakamp>: ";
			cin>>peakamp_min>>peakamp_max;
			cerr<<"Your input: "<<peakamp_min<<","<<peakamp_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "averamp")
		{
			cerr<<"<averamp>: ";
			cin>>averamp_min>>averamp_max;
			cerr<<"Your input: "<<averamp_min<<","<<averamp_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "rawsnr")
		{
			cerr<<"<rawsnr>: ";
			cin>>rawsnr_min>>rawsnr_max;
			cerr<<"Your input: "<<rawsnr_min<<","<<rawsnr_max<<endl;
			cerr<<"Decon threshold is set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
		else if(this->decon_kill_parname == "ALLDECONPAR")
		{
			cerr<<"<niteration>: ";
			cin>>niteration_min>>niteration_max;
			cerr<<"Your input: "<<niteration_min<<","<<niteration_max<<endl;
			cerr<<"<nspike>: ";
			cin>>nspike_min>>nspike_max;
			cerr<<"Your input: "<<nspike_min<<","<<nspike_max<<endl;
			cerr<<"<epsilon>: ";
			cin>>epsilon_min>>epsilon_max;
			cerr<<"Your input: "<<epsilon_min<<","<<epsilon_max<<endl;
			cerr<<"<peakamp>: ";
			cin>>peakamp_min>>peakamp_max;
			cerr<<"Your input: "<<peakamp_min<<","<<peakamp_max<<endl;
			cerr<<"<averamp>: ";
			cin>>averamp_min>>averamp_max;
			cerr<<"Your input: "<<averamp_min<<","<<averamp_max<<endl;
			cerr<<"<rawsnr>: ";
			cin>>rawsnr_min>>rawsnr_max;
			cerr<<"Your input: "<<rawsnr_min<<","<<rawsnr_max<<endl;
			cerr<<"All decon thresholds are set. Applying to the ensemble ..."<<endl<<"> ";
			this->apply_kill_decon_by_PARNAME();
		}
	}
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
//callbacks for killing by decon
void apply_kill_decon_par_for_all_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="ALLDECONPAR";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_niteration_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="niteration";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_nspike_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="nspike";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_epsilon_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="epsilon";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_peakamp_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="peakamp";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_averamp_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="averamp";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void apply_kill_decon_rawsnr_callback(Widget w, 
					XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->decon_kill_parname="rawsnr";
	plot_handle->apply_kill_decon_by_PARNAME();
}
void TraceEditPlot::apply_kill_decon_by_PARNAME()
{
	if(this->allow_edit)
	{
		if(use_decon_in_editing)
		{
			this->backup_undo_tse();
			undo_for_kill=true;
			killed_trace_excluded=false;
			//backup original data.
			this->backup_original_tse();
	
			//print out parameter threshold before killing
			cerr<<"Decon Parameter Threshold:"<<endl;

			TimeSeriesEnsemble *comp_tmp;
			//stringstream ss;
			int i,kmax;
			if(ThreeComponentMode)
				kmax=3;
			else
				kmax=1;
			try 
			{
				XtAppLock(AppContext);
				for(i=0;i<kmax;++i)
				{
					if(seisw[i]!=NULL)
					{
						TimeSeriesEnsemble * sensemble;
						//set<long> tmp;
						XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
						if(this->decon_kill_parname == "niteration")
						{
							cerr<<"niteration < "<<niteration_min<<
								", "<<niteration_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_niteration(*sensemble,
											niteration_min,niteration_max);
						}
						else if(this->decon_kill_parname == "nspike")
						{
							cerr<<"nspike < "<<nspike_min<<
								", "<<nspike_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_nspike(*sensemble,
											nspike_min,nspike_max);
						}
						else if(this->decon_kill_parname == "epsilon")
						{
							cerr<<"epsilon < "<<epsilon_min<<
								", "<<epsilon_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_epsilon(*sensemble,
											epsilon_min,epsilon_max);
						}
						else if(this->decon_kill_parname == "peakamp")
						{
							cerr<<"peakamp < "<<peakamp_min<<
								", "<<peakamp_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_peakamp(*sensemble,
											peakamp_min,peakamp_max);
						}
						else if(this->decon_kill_parname == "averamp")
						{
							cerr<<"averamp < "<<averamp_min<<
								", "<<averamp_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_averamp(*sensemble,
											averamp_min,averamp_max);
						}
						else if(this->decon_kill_parname == "rawsnr")
						{
							cerr<<"rawsnr < "<<rawsnr_min<<
								", "<<rawsnr_max<<" >"<<endl;
							last_autokill_list=
								teo_global.kill_by_decon_rawsnr(*sensemble,
											rawsnr_min,rawsnr_max);
						}
						else if(this->decon_kill_parname == "ALLDECONPAR")
						{
							cerr<<"niteration < "<<niteration_min<<", "<<niteration_max<<" >"<<endl
								<<"nspike < "<<nspike_min<<", "<<nspike_max<<" >"<<endl
								<<"epsilon < "<<epsilon_min<<", "<<epsilon_max<<" >"<<endl
								<<"peakamp < "<<peakamp_min<<", "<<peakamp_max<<" >"<<endl
								<<"averamp < "<<averamp_min<<", "<<averamp_max<<" >"<<endl
								<<"rawsnr < "<<rawsnr_min<<", "<<rawsnr_max<<" >"<<endl;
						
							last_autokill_list=teo_global.kill_by_decon_ALL(*sensemble,
											niteration_min,niteration_max,
											nspike_min,nspike_max,
											epsilon_min,epsilon_max,
											peakamp_min,peakamp_max,
											averamp_min,averamp_max,
											rawsnr_min,rawsnr_max);
						}
						cerr<<"TraceEditPlot: number of traces killed by decon threshold = "
							<<last_autokill_list.size()<<endl;
						if(last_autokill_list.size()>0)
						{
							comp_tmp=new TimeSeriesEnsemble(*sensemble);
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
										(XtPointer) (comp_tmp),ExmNseiswMetadata,
										(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
						}
					}
				}
				XtAppUnlock(AppContext);
			}catch(...) {throw;};
		}
		else
			cerr<<"ERROR: please turn on 'use_decon_in_editing' if you would like to"<<endl
				<<"kill traces by setting decon parameter threshold."<<endl;
	}
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}

//apply threshold to stackweight.
void apply_kill_low_stackweight_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_low_stackweight();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_low_stackweight()
{
	killed_trace_excluded=false;
	this->backup_undo_tse();
	
	//sort by stackweight before running this procedure.
	cerr<<"Sorting by stackweight before running this procedure ..."<<endl<<"> ";
	this->sort_by_stack_weight();
	//print out parameter threshold before killing
	cerr<<"Minimum Robust Stack Weight = "<<this->min_stackweight<<endl<<"> ";
	undo_for_kill=true;
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=teo_global.kill_low_stackweight_traces(
							*sensemble,min_stackweight);
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
				cerr<<"TraceEditPlot: number of traces killed by stackweight threshold = "
						<<last_autokill_list.size()<<endl<<"> ";
			}
		}
		XtAppUnlock(AppContext);
	}catch(...) {throw;};
}
void customize_kill_low_stackweight_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->customize_kill_low_stackweight();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_low_stackweight()
{
	cerr<<"Current minimum stack weight: "<<min_stackweight<<endl;
	cerr<<"Please type in your minimum stack weight: ";
	cin>>min_stackweight;
	cerr<<"New minimum stack weight: "<<min_stackweight<<endl<<"> ";
	this->apply_kill_low_stackweight();
}
//apply threshold to ref trace correlation.
void apply_kill_low_ref_correlation_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_low_ref_correlation();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void apply_kill_low_ref_correlation_stacktrace_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_low_ref_correlation_stacktrace();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_low_ref_correlation()
{
	killed_trace_excluded=false;
	this->backup_undo_tse();
		
	if(!ref_trace_picked)
		cerr<<"ERROR: can't run procedure 'apply_kill_low_ref_correlation'."<<endl
			<<"Reference trace not picked!"<<endl<<"> ";
	else
	{
		//print out parameter threshold before killing
		cerr<<"Minimum correlation coefficience = "<<this->min_xcorcoe<<endl<<"> ";
		undo_for_kill=true;
		TimeSeriesEnsemble *comp_tmp;
		//stringstream ss;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					TimeSeriesEnsemble * sensemble;
					//set<long> tmp;
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					if(!ref_trace_xcor_twin_set)
					{	
						cerr<<"Computing xcorcoe for the whole trace ..."<<endl
							<<"Use RobustStack trace as reference."
							<<endl<<"> ";
						TimeWindow common_twin=teo_global.find_common_timewindow(*sensemble);
						last_autokill_list=teo_global.kill_low_ref_correlation_traces(
									*sensemble,ref_trace_evid,common_twin,min_xcorcoe);
					}
					else
					{
						cerr<<"Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start
							<<", "<<ref_trace_xcor_twin.end<<" >"<<endl;
						last_autokill_list=teo_global.kill_low_ref_correlation_traces(
									*sensemble,ref_trace_evid,ref_trace_xcor_twin,min_xcorcoe);
					}
					if(last_autokill_list.size()>0)
					{
						comp_tmp=new TimeSeriesEnsemble(*sensemble);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
									(XtPointer) (comp_tmp),ExmNseiswMetadata,
									(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
					cerr<<"TraceEditPlot: number of traces killed by ref trace correlation threshold = "
							<<last_autokill_list.size()<<endl<<"> ";
				}
			}
			XtAppUnlock(AppContext);
		}catch(...) {throw;};
	}
}
void TraceEditPlot::apply_kill_low_ref_correlation_stacktrace()
{
	killed_trace_excluded=false;
	this->backup_undo_tse();
	//print out parameter threshold before killing
	cerr<<"Minimum correlation coefficience = "<<this->min_xcorcoe<<endl<<"> ";
	undo_for_kill=true;
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(!ref_trace_xcor_twin_set)
				{	
					cerr<<"Computing xcorcoe for the whole trace ..."<<endl
						<<"Use RobustStack trace as reference."
						<<endl<<"> ";
					cerr<<"Robust stacking window = < "<<robust_twin.start<<
							", "<<robust_twin.end<<" >"<<endl<<"> ";

					TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
					cerr<<"Computing stack ..."<<endl<<"> ";
					TimeSeries stack_tmp=teo_global.get_stack(*sensemble,stack_twin,robust_twin,
											stacktype);
					last_autokill_list=teo_global.kill_low_ref_correlation_traces(
								*sensemble,stack_tmp,stack_twin,min_xcorcoe);
				}
				else
				{
					cerr<<"Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start
						<<", "<<ref_trace_xcor_twin.end<<" >"<<endl;
					cerr<<"Robust stacking window = < "<<robust_twin.start<<
							", "<<robust_twin.end<<" >"<<endl<<"> ";

					TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
					cerr<<"Computing stack ..."<<endl<<"> ";
					TimeSeries stack_tmp=teo_global.get_stack(*sensemble,stack_twin,robust_twin,
											stacktype);
					last_autokill_list=teo_global.kill_low_ref_correlation_traces(
								*sensemble,stack_tmp,ref_trace_xcor_twin,min_xcorcoe);
				}
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
				cerr<<"TraceEditPlot: number of traces killed by ref trace correlation threshold = "
						<<last_autokill_list.size()<<endl<<"> ";
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError& serr) 
	{cerr<<"** Error when applying kill by low ref_xcor."<<endl;serr.what();};
}
void customize_kill_low_ref_correlation_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->customize_kill_low_ref_correlation();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_low_ref_correlation()
{
	cerr<<"Current minimum xcorcoe with ref trace: "<<min_xcorcoe<<endl;
	cerr<<"Please type in your minimum correlation coefficience: ";
	cin>>min_xcorcoe;
	cerr<<"New minimum xcorcoe with ref trace: "<<min_xcorcoe<<endl<<"> ";	
	int itmp=-1;
	while(itmp!=1 && itmp!=2 && itmp!=0)
	{
		cerr<<endl
		<<"    1- Xcorcoe with picked reference trace;"<<endl
		<<"    2- Xcorcoe with stack trace;"<<endl
		<<"    0- Cancel and apply later."<<endl
		<<"> Please input your choice: ";
		cin>>itmp;
		switch(itmp)
		{
			case 1:
				this->apply_kill_low_ref_correlation();
				break;
			case 2:
				this->apply_kill_low_ref_correlation_stacktrace();
				break;
			case 0:
				cerr<<"Canceled. You can manually apply this procedure later."<<endl<<"> ";
				break;
			default:
				cerr<<"Wrong method code."<<endl;
		}
	}
}
//procedure: kill by low DSI.
void customize_kill_low_dsi_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
	{	if(plot_handle->use_decon_in_editing)
			plot_handle->customize_kill_low_dsi();
		else
			cerr<<"ERROR: use_decon_in_editing is set to false."<<endl<<"> ";
	}
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::customize_kill_low_dsi()
{
	cerr<<"Current minimum decon-success index: "<<min_dsi<<endl;
	cerr<<"Please type in your minimum decon-success index: ";
	cin>>min_dsi;
	cerr<<"New minimum decon-success index: "<<min_dsi<<endl<<"> ";
	this->apply_kill_low_dsi();
}

//apply threshold to dsi.
void apply_kill_low_dsi_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
	{	if(plot_handle->use_decon_in_editing) 
			plot_handle->apply_kill_low_dsi();
		else
			cerr<<"ERROR: use_decon_in_editing is set to false."<<endl<<"> ";
	}
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_low_dsi()
{
	killed_trace_excluded=false;
	this->backup_undo_tse();
	
	//print out parameter threshold before killing
	cerr<<"Minimum decon-success index = "<<this->min_dsi<<endl<<"> ";
	undo_for_kill=true;
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=teo_global.kill_low_dsi_traces(
							*sensemble,min_dsi);
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
				cerr<<"TraceEditPlot: number of traces killed by DSI threshold = "
						<<last_autokill_list.size()<<endl<<"> ";
			}
		}
		XtAppUnlock(AppContext);
	}catch(...) {throw;};
}

//procedure: kill by low RFQI
void apply_kill_low_RFQualityIndex_callback(Widget w, 
			XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->allow_edit)
		plot_handle->apply_kill_low_RFQualityIndex();
	else
		cerr<<"Edit is disabled."<<endl<<"> ";
}
void TraceEditPlot::apply_kill_low_RFQualityIndex()
{
	killed_trace_excluded=false;
	this->backup_undo_tse();
	undo_for_kill=true;
	double qi_min(-9999.9);
	this->customize_weights_for_quality_index();
	
	while(abs(qi_min)>1.0)
	{
		cerr<<"Please input Quality Index cutoff (0.0<=minimum<=1.0): ";
		cin>>qi_min;
		if(qi_min<0.0 || qi_min>1.0) 
			cerr<<"Quality Index cutoff must between 0 and 1."<<endl<<"> ";
		else
			cerr<<"> ";
	}
	cerr<<"Applying kill procedure: apply_kill_low_RFQualityIndex() ..."<<endl<<"> ";

	TimeSeriesEnsemble *comp_tmp;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				TimeSeriesEnsemble * sensemble;
				//set<long> tmp;
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				last_autokill_list=teo_global.kill_low_RFQualityIndex_traces(
							*sensemble,RF_quality_index_weights[0],
							RF_quality_index_weights[1],
							RF_quality_index_weights[2],qi_min,true);
				if(last_autokill_list.size()>0)
				{
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
				cerr<<"TraceEditPlot: number of traces killed by quality index threshold = "
						<<last_autokill_list.size()<<endl<<"> ";
			}
		}
		XtAppUnlock(AppContext);
	}catch(...) {throw;};
}
//restore original ensemble.
void restore_ensemble_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->killed_review_mode)
		cerr<<"ERROR: can't restore ensemble. You might be under 'Review Killed Traces' mode."<<endl
			<<"Exit that mode before restoring the original ensemble."<<endl<<"> ";
	else
		plot_handle->restore_ensemble();
}
void TraceEditPlot::restore_ensemble()
{
	this->backup_undo_tse();
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	//set<long> tmp;
	int i,kmax;

	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				if(tse_backup_set[i])
				{
					cerr<<original_ensemble[i].member.size()<<endl;
					comp_tmp=new TimeSeriesEnsemble(original_ensemble[i]);
					cerr<<"Number of traces restored = "<<comp_tmp->member.size()<<endl;
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
						ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)),
						 NULL);
					//clear kills after restoring the original data.
					kills.clear();
					teo_global.reset_statistics();
					killed_traces.member.clear();
				}
				else
				{
					cerr<<"ERROR: original ensemble not backed up. "<<
						"Cannot restore the data!"<<endl;
				}
			}
		}
		XtAppUnlock(AppContext);		
	}catch(...){throw;}	
}

//undo last kills
void undo_auto_edits_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(!plot_handle->allow_edit)
		cerr<<"ERROR: can't restore ensemble. You might be under 'Review Killed Traces' mode."<<endl
			<<"Exit that mode before any editings."<<endl<<"> ";
	else
		plot_handle->undo_auto_edits();
}

void TraceEditPlot::undo_auto_edits()
{
	this->report_kills();
	cerr<<"Last kill set size = "<<last_autokill_list.size()<<endl;
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	set<long>::iterator iptr;
	int i,kmax;

	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				//XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				//teo_global.undo_kills_to_trace(*sensemble,last_autokill_list);
				switch(i)
				{
					case 0:
						if(tse_bkp_for_undo0->member.size()>0)
						{	
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_for_undo0);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_for_undo0->member.clear();
							if(undo_for_kill)
							{	for(iptr=last_autokill_list.begin();
										iptr!=last_autokill_list.end();++iptr)
									kills.erase(*iptr);
								last_autokill_list.clear();
								int itmp=teo_global.sizeof_statistics()-this->size_of_statistics;
								for(int kk=0; kk<itmp; ++kk)
									teo_global.remove_last_statistics();
							}
							if(undo_for_live)
							{
								kills.insert(restored_killed_traces_list.begin(),
									restored_killed_traces_list.end());
								restored_killed_traces_list.clear();
								teo_global.remove_last_statistics();
							}
							cerr<<"Number of kills left = "<<kills.size()<<endl<<"> ";
						}
						else
							cerr<<"Warning: no edits to undo for component 1."<<endl<<"> ";
						break;
					case 1:
						if(tse_bkp_for_undo1->member.size()>0)
						{
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_for_undo1);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_for_undo1->member.clear();
							if(undo_for_kill)
							{	for(iptr=last_autokill_list.begin();
										iptr!=last_autokill_list.end();++iptr)
									kills.erase(*iptr);
								last_autokill_list.clear();
							}
							cerr<<"Number of kills left = "<<kills.size()<<endl<<"> ";
						}
						else
							cerr<<"Warning: no edits to undo for component 2."<<endl<<"> ";
						break;
					case 2:
						if(tse_bkp_for_undo2->member.size()>0)
						{
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_for_undo2);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_for_undo2->member.clear();
							if(undo_for_kill)
							{	for(iptr=last_autokill_list.begin();
										iptr!=last_autokill_list.end();++iptr)
									kills.erase(*iptr);
								last_autokill_list.clear();
							}
							cerr<<"Number of kills left = "<<kills.size()<<endl<<"> ";
						}
						else
							cerr<<"Warning: no edits to undo for component 3."<<endl<<"> ";
						break;
				}
			}
		}
		XtAppUnlock(AppContext);

	}catch(SeisppError& serr)
	{
		serr.what();
	}
}
//end of editing procedures.
//////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Build Sort Menu functions ////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//*********************************************************************************
//*************************** start of pick reference trace code.
//pick reference trace.
void select_trace(Widget w, XtPointer client_data, XtPointer userdata)
{
    SeismicPick * spick;
    TimeSeriesEnsemble * sensemble;
    //stringstream ss;
    long tmp, evid;
	try
	{
		XtVaGetValues(w,ExmNseiswPick,&spick,NULL);
		XtVaGetValues(w,ExmNseiswEnsemble,&sensemble,NULL);
		if (spick->type == POINT) {
			//DEBUG
			TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
			tmp=spick->get_trace_number();
			evid=sensemble->member[tmp].get_int(evidkey);
			if(plot_handle->select_trace_typename=="XCOR_REF_TRACE")
			{
				plot_handle->ref_trace_evid=evid;
				plot_handle->ref_trace_picked=true;
				cerr<< "Reference trace number = "<<tmp+1
				<<", evid = "<<evid<<endl;
				plot_handle->allow_edit=true;
				plot_handle->edit_enable_public();
			}
			else if(plot_handle->select_trace_typename=="RESTORE_KILLED_TRACE")
			{
				plot_handle->restored_killed_traces_list.insert(evid);
				cerr<< "Restored trace number = "<<tmp+1
				<<", evid = "<<evid<<endl<<"> ";
				cerr<<"Continue restoring killed trace? (y/n) ";
				string ctmp;
				cin>>ctmp;
				if(ctmp!="y" && ctmp!="Y")
				{
					cerr<<"> Finished restoring killed traces. Back to review mode."<<endl<<"> ";
					plot_handle->edit_disable_public();
				}	
			}
			else if(plot_handle->select_trace_typename=="SHOW_TRACE_METADATA")
			{
				plot_handle->teo_global.show_metadata(*sensemble, 
								evid,plot_handle->use_decon_in_editing);
				cerr<<"> ";
				cerr<<"Show metadata for another trace? (y/n) ";
				string ctmp;
				cin>>ctmp;
				if(ctmp!="y" && ctmp!="Y")
				{
					if(plot_handle->killed_review_mode)
					{
						cerr<<"> Back to 'Review Killed Traces' mode."<<endl<<"> ";
						plot_handle->allow_edit=false;
						plot_handle->edit_disable_public();
					}
					else
					{
						cerr<<"> Back to trace edit mode."<<endl<<"> ";
						plot_handle->allow_edit=true;
						plot_handle->edit_enable_public();
					}
				}
				else
					cerr<<"> ";
			}
			else if(plot_handle->select_trace_typename=="SAVE_TIMESERIES_TO_FILE")
			{
				FILE * fh;
				stringstream ss;
				string sta=sensemble->get_string("sta");
				string chantemp=sensemble->get_string("chan");
				ss<<sta<<"_"<<evid<<"_"<<chantemp<<".dat"<<'\0';
				string fname=ss.str();
				fh=fopen(fname.c_str(),"w");
				TimeSeries ts=sensemble->member[tmp];
				int metadata_version(2);
				plot_handle->teo_global.save_metadata(ts,fh,plot_handle->use_decon_in_editing,
						metadata_version);
				vector<double>::iterator iptr;
				for(iptr=ts.s.begin();iptr!=ts.s.end();++iptr)
        		{
					fprintf(fh,"%12.5f\n",*iptr);
				}
				fclose(fh);
				cerr<<"Picked trace has been saved as text file to: "<<fname<<endl
					<<"Metadata Version/Specifier: "<<metadata_version<<endl<<endl;
				//ask for picking another trace
				cerr<<"> ";
				cerr<<"Save another trace to file? (y/n) ";
				string ctmp;
				cin>>ctmp;
				if(ctmp!="y" && ctmp!="Y")
				{
					if(plot_handle->killed_review_mode)
					{
						cerr<<"> Back to 'Review Killed Traces' mode."<<endl<<"> ";
						plot_handle->allow_edit=false;
						plot_handle->edit_disable_public();
					}
					else
					{
						cerr<<"> Back to trace edit mode."<<endl<<"> ";
						plot_handle->allow_edit=true;
						plot_handle->edit_enable_public();
					}
				}
				else
					cerr<<"> ";
			}
			else if(plot_handle->select_trace_typename=="PLOT_SELECTED_TRACE")
			{
				TimeSeries ts=sensemble->member[tmp];
				plot_handle->beam_tse.member.clear();
    			plot_handle->beam_tse.member.push_back(ts);
    			stringstream beam_title;
    			beam_title<<"Trace="<<tmp+1<<", evid="<<evid<<" for station: "
    				<<sensemble->get_string("sta")<<'\0';
    			plot_handle->beam_tse.put("beam_title",beam_title.str());
    			plot_handle->do_beam_plot();
    			if(plot_handle->killed_review_mode)
				{
					cerr<<"> Back to 'Review Killed Traces' mode."<<endl<<"> ";
					plot_handle->allow_edit=false;
					plot_handle->edit_disable_public();
				}
				else
				{
					cerr<<"> Back to trace edit mode."<<endl<<"> ";
					plot_handle->allow_edit=true;
					plot_handle->edit_enable_public();
				}
			}
		} else {
			cerr << "Wrong pick type "<<spick->type<<endl;
		}
	}catch(...){throw;};
}
//pick reference trace callback
void pick_rt_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->select_trace_typename="XCOR_REF_TRACE";
	plot_handle->add_select_trace_callback();
}
//remove all btn2 calbacks and add pick reference trace callback to trace widget.
void TraceEditPlot::add_select_trace_callback()   
{ 
    int k;
    int kmax;
    if(ThreeComponentMode)
        kmax=3;
    else
        kmax=1;   
	//cerr<<"it's me"<<endl;
    for(int k=0;k<kmax;++k)
    {
        if(seisw[k]!=NULL)
        {
            cerr<<"*** Edit is disabled before picking the trace."<<endl;
            XtVaSetValues(seisw[k],ExmNeditEnable,0,NULL); //disable edit.
            this->allow_edit=false;
            cerr<<"> Please pick a trace using M2 ..."<<endl;
            XtRemoveAllCallbacks(seisw[k],ExmNbtn2Callback);
            // restore the initial edit menu.
            XtVaSetValues(this->menu_edit,XmNlabelString,
            			XmStringCreateLocalized(const_cast<char *>(edit_menu_label.c_str())),NULL);
            XtAddCallback(seisw[k],ExmNbtn2Callback,select_trace,this);  
        }
    }

	//plot_handle->pick_ref_trace();
}
//*************************** end of pick reference trace code.
//*********************************************************************************

//Start of sorting procedures.
void sort_by_magnitude_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	if(plot_handle->use_netmag_table) plot_handle->sort_by_magnitude();
	else
	{
		cerr<<"ERROR: netmag table is not used. magnitude information is not available!"<<endl;
	}
}
void TraceEditPlot::sort_by_magnitude()
{
	//cout<<"This is a call test!"<<endl;
	this->backup_undo_tse();
	undo_for_kill=false;
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(trace_order.size()>0) trace_order.clear();

				trace_order=teo_global.sort_by_ascend_magnitude(*sensemble);

				if(trace_order.size()>0)
				{
					sort_method=sensemble->get_string(sort_method_key);
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
						(XtPointer) (comp_tmp),ExmNseiswMetadata,
						(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError& serr) 
	{cerr<<"**Error in sorting by magnitude!"<<endl;
	serr.what();};
}
void customize_xcor_window_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->timewindow_typename="REF_TRACE_XCOR";
	plot_handle->customize_time_window_by_WINDOWTYPE();
}
void sort_by_ref_trace_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->sort_by_ref_trace();
}
void sort_by_xcor_to_stacktrace_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->sort_by_xcor_to_stacktrace();
}
void TraceEditPlot::sort_by_ref_trace()
{
    if(ref_trace_picked)
    {
		this->backup_undo_tse();
		undo_for_kill=false;
		TimeSeriesEnsemble * sensemble;
		TimeSeriesEnsemble *comp_tmp;
		//stringstream ss;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					if(trace_order.size()>0) trace_order.clear();
					if(!ref_trace_xcor_twin_set)
					{	
						cerr<<"Applying sort procedure for the whole trace ..."
							<<endl<<"> ";
						trace_order=teo_global.sort_by_less_xcorcoef(
								*sensemble,ref_trace_evid);
					}
					else
					{	
						cerr<<"Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start
							<<", "<<ref_trace_xcor_twin.end<<" >"<<endl;
						trace_order=teo_global.sort_by_less_xcorcoef(
								*sensemble,ref_trace_evid,ref_trace_xcor_twin);
					}
					if(trace_order.size()>0)
					{
						sort_method=sensemble->get_string(sort_method_key);
						comp_tmp=new TimeSeriesEnsemble(*sensemble);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
							(XtPointer) (comp_tmp),ExmNseiswMetadata,
							(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
			}
			XtAppUnlock(AppContext);
		}catch(...) {throw;};
    }
    else
    	cerr<<"ERROR: can't sort by reference trace! Reference trace not set. "<<endl
    		<<"Please press key 'P' to start picking reference trace."<<endl;
}
void TraceEditPlot::sort_by_xcor_to_stacktrace()
{
	this->backup_undo_tse();
	undo_for_kill=false;
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	//stringstream ss;
	int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(trace_order.size()>0) trace_order.clear();
				if(!ref_trace_xcor_twin_set)
				{	
					cerr<<"Applying sort to the whole trace ..."<<endl
						<<"Use RobustStack trace as reference."
						<<endl<<"> ";
					cerr<<"Robust stacking window = < "<<robust_twin.start<<
							", "<<robust_twin.end<<" >"<<endl<<"> ";

					TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
					cerr<<"Computing stack ..."<<endl<<"> ";
					TimeSeries stack_tmp=teo_global.get_stack(*sensemble,stack_twin,robust_twin,
											stacktype);
					trace_order=teo_global.sort_by_less_xcorcoef(
							*sensemble,stack_tmp,stack_twin);
				}
				else
				{	
					cerr<<"Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start
						<<", "<<ref_trace_xcor_twin.end<<" >"<<endl;
					cerr<<"Robust stacking window = < "<<robust_twin.start<<
							", "<<robust_twin.end<<" >"<<endl<<"> ";

					TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
					cerr<<"Computing stack ..."<<endl<<"> ";
					TimeSeries stack_tmp=teo_global.get_stack(*sensemble,stack_twin,robust_twin,
											stacktype);
					trace_order=teo_global.sort_by_less_xcorcoef(
							*sensemble,stack_tmp,ref_trace_xcor_twin);
				}
				if(trace_order.size()>0)
				{
					sort_method=sensemble->get_string(sort_method_key);
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
						(XtPointer) (comp_tmp),ExmNseiswMetadata,
						(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError& serr) 
	{cerr<<"**Error in sorting by xcor to stack trace!"<<endl;
	serr.what();};
}
void customize_robust_window_callback(Widget w, 
			XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->timewindow_typename="ROBUST";
	plot_handle->customize_time_window_by_WINDOWTYPE();
}
void TraceEditPlot::customize_time_window_by_WINDOWTYPE()
{
	if(this->timewindow_typename=="ROBUST")
	{	
		cerr<<"Current Robust Time Window = < "<<robust_twin.start<<
				", "<<robust_twin.end<<" >"<<endl;
		cerr<<"Please type in your values (start end): ";
		cin>>robust_twin.start>>robust_twin.end;
		cerr<<"New Robust Time Window = < "<<robust_twin.start<<
				", "<<robust_twin.end<<" >"<<endl;
		//apply sort or not?
		cerr<<"> Apply sort by Stack Weight (y/n)? ";
		string ctemp;
		cin>>ctemp;
		if(ctemp=="y" || ctemp=="Y")
		{	this->sort_by_stack_weight();
			cerr<<"> ";
		}
		else
			cerr<<"> ";
	}
	else if(this->timewindow_typename=="REF_TRACE_XCOR")
	{
		if(ref_trace_xcor_twin_set)
			cerr<<"Current Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start<<
				", "<<ref_trace_xcor_twin.end<<" >"<<endl;
		cerr<<"Please type in your Time Window (start end): ";
		cin>>ref_trace_xcor_twin.start>>ref_trace_xcor_twin.end;
		cerr<<"New Ref Trace Xcor Time Window = < "<<ref_trace_xcor_twin.start<<
				", "<<ref_trace_xcor_twin.end<<" >"<<endl;
		ref_trace_xcor_twin_set=true;
		//apply sort or not?
		cerr<<"> Apply sort by Correlation With Ref-Trace? (y/n) ";
		string ctemp;
		cin>>ctemp;
		if(ctemp=="y" || ctemp=="Y")
		{	
			cerr<<"> Use stack trace as reference trace? (y/n) ";
			string ctmp2;
			cin>>ctmp2;
			if(ctmp2=="y" || ctmp2=="Y")
				this->sort_by_xcor_to_stacktrace();
			else
				this->sort_by_ref_trace();
			cerr<<"> ";
		}
		else
			cerr<<"> ";
	}
	else if(this->timewindow_typename=="FA_SEARCH_WINDOW")
	{
		if(FA_search_window.length()>0)
			cerr<<"Current FA Search Time Window = < "<<FA_search_window.start<<
				", "<<FA_search_window.end<<" >"<<endl;
		cerr<<"Please type in your Time Window (start end): ";
		cin>>FA_search_window.start>>FA_search_window.end;
		cerr<<"New FA Search Time Window = < "<<FA_search_window.start<<
			", "<<FA_search_window.end<<" >"<<endl;
		cerr<<"> ";
	}
	else if(this->timewindow_typename=="NFA_TOLERANCE_WINDOW")
	{
		if(FA_search_window.length()>0)
			cerr<<"Current Negative FAs tolerance Time Window = < "
				<<NFA_tolerance_window.start<<
				", "<<NFA_tolerance_window.end<<" >"<<endl;
		cerr<<"Please type in your Time Window (start end): ";
		cin>>NFA_tolerance_window.start>>NFA_tolerance_window.end;
		cerr<<"New Negative FAs tolerance Time Window = < "<<NFA_tolerance_window.start<<
			", "<<NFA_tolerance_window.end<<" >"<<endl;
		cerr<<"> ";
	}
	else if(this->timewindow_typename=="PCODA_SEARCH_WINDOW")
	{
		if(PCoda_search_window.length()>0)
			cerr<<"Current P-Coda Search Time Window = < "<<PCoda_search_window.start<<
				", "<<PCoda_search_window.end<<" >"<<endl;
		cerr<<"Please type in your Time Window (start end): ";
		cin>>PCoda_search_window.start>>PCoda_search_window.end;
		cerr<<"New P-Coda Search Time Window = < "<<PCoda_search_window.start<<
			", "<<PCoda_search_window.end<<" >"<<endl;
		cerr<<"> ";
	}
	else if(this->timewindow_typename=="CODACA_SEARCH_WINDOW")
	{
		if(CodaCA_search_window.length()>0)
			cerr<<"Current Coda ClusteredArrivals Search Time Window = < "<<CodaCA_search_window.start<<
				", "<<CodaCA_search_window.end<<" >"<<endl;
		cerr<<"Please type in your Time Window (start end): ";
		cin>>CodaCA_search_window.start>>CodaCA_search_window.end;
		cerr<<"New Coda ClusteredArrivals Search Time Window = < "<<CodaCA_search_window.start<<
			", "<<CodaCA_search_window.end<<" >"<<endl;
		cerr<<"> ";
	}
}
void sort_by_stack_weight_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->sort_by_stack_weight();
}
void TraceEditPlot::sort_by_stack_weight()
{
	this->backup_undo_tse();
		undo_for_kill=false;
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	cerr<<"Robust stacking window = < "<<robust_twin.start<<
			", "<<robust_twin.end<<" >"<<endl;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
    	kmax=3;
    else
    	kmax=1;

	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
				//cout<<stack_twin.start<<", "<<stack_twin.end<<endl;
				if(trace_order.size()>0) trace_order.clear();
				trace_order=teo_global.stack_and_sort(
							*sensemble,stack_twin,robust_twin,
							stacktype);
				if(trace_order.size()>0)
				{
					sort_method=sensemble->get_string(sort_method_key);
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
						(XtPointer) (comp_tmp),ExmNseiswMetadata,
						(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError &err)
	{
		cerr<<"**Error when sorting by stack_weight:"<<endl;
		err.what();
		//cout << err.message << endl;
		//exit(-1);
	}
}
//sort by decon parameters.
void sort_by_decon_niteration_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="niteration";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_nspike_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="nspike";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_epsilon_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="epsilon";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_peakamp_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="peakamp";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_averamp_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="averamp";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_rawsnr_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="rawsnr";
	plot_handle->sort_by_DECONPARNAME();
}
void sort_by_decon_successindex_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->decon_sort_parname="success_index";
	plot_handle->sort_by_DECONPARNAME();
}
void TraceEditPlot::sort_by_DECONPARNAME()
{
	if(use_decon_in_editing)
	{
		this->backup_undo_tse();
			undo_for_kill=false;
		TimeSeriesEnsemble * sensemble;
		TimeSeriesEnsemble *comp_tmp;
		cerr<<"Sorting by decon parameter: "<<decon_sort_parname<<endl<<"> ";
		//stringstream ss;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;

		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					if(trace_order.size()>0) trace_order.clear();
					trace_order=teo_global.sort_by_decon_parameters(
								*sensemble,decon_sort_parname);
					if(trace_order.size()>0)
					{
						sort_method=sensemble->get_string(sort_method_key);
						comp_tmp=new TimeSeriesEnsemble(*sensemble);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
							(XtPointer) (comp_tmp),ExmNseiswMetadata,
							(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
			}
			XtAppUnlock(AppContext);
		}catch(...) {throw;};
	}
	else
		cerr<<"ERROR: use_decon_in_editing is set to false. Can't sort by decon parameters."<<endl;
}
void sort_by_quality_index_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->sort_by_quality_index();
}
void customize_weights_for_quality_index_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->customize_weights_for_quality_index();
}
void TraceEditPlot::customize_weights_for_quality_index()
{
	cerr<<endl<<"  <RF_quality_index_weights>"<<endl
		<<"  StackWeight(a1)        "<<RF_quality_index_weights[0]<<endl
		<<"  RefXcorcoe(a2)         "<<RF_quality_index_weights[1]<<endl
		<<"  DeconSuccessIndex(a3)  "<<RF_quality_index_weights[2]<<endl;
	cerr<<"> Set new weights? (y/n) ";
	char ctemp;
	cin>>ctemp;
	if(ctemp=='y' || ctemp=='Y')
	{
		cerr<<"> Please input new weights (a1 a2 a3): ";
		cin>>RF_quality_index_weights[0]
			>>RF_quality_index_weights[1]
			>>RF_quality_index_weights[2];
	
		cerr<<endl<<"  <RF_quality_index_weights>"<<endl
			<<"  StackWeight(a1)        "<<RF_quality_index_weights[0]<<endl
			<<"  RefXcorcoe(a2)         "<<RF_quality_index_weights[1]<<endl
			<<"  DeconSuccessIndex(a3)  "<<RF_quality_index_weights[2]<<endl;
		cerr<<" > New weights are set. "<<endl<<"> ";
	}
	else
		cerr<<"> ";
	//this->sort_by_quality_index();
}
void TraceEditPlot::sort_by_quality_index()
{
	this->backup_undo_tse();
		undo_for_kill=false;
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	cerr<<endl<<"  <RF_quality_index_weights>"<<endl
		<<"  StackWeight(a1)        "<<RF_quality_index_weights[0]<<endl
		<<"  RefXcorcoe(a2)         "<<RF_quality_index_weights[1]<<endl
		<<"  DeconSuccessIndex(a3)  "<<RF_quality_index_weights[2]<<endl;
	
	if(!use_decon_in_editing)
	{
		cerr<<"Warning: use_decon_in_editing is set to false."<<endl
			<<"Can't give weight to DeconSuccessIndex!"<<endl
			<<"Weights are rescaled to the following with 0 weight for DSI:"<<endl;
		RF_quality_index_weights[2]=0.0;
		double dtmp=RF_quality_index_weights[0]+RF_quality_index_weights[1];
		double dtmp0=RF_quality_index_weights[0],dtmp1=RF_quality_index_weights[1];
		RF_quality_index_weights[0]=dtmp0/dtmp;
		RF_quality_index_weights[1]=dtmp1/dtmp;
		
		cerr<<endl<<"  <RF_quality_index_weights>"<<endl
			<<"  StackWeight(a1)        "<<RF_quality_index_weights[0]<<endl
			<<"  RefXcorcoe(a2)         "<<RF_quality_index_weights[1]<<endl
			<<"  DeconSuccessIndex(a3)  "<<RF_quality_index_weights[2]<<endl;
	}
	cerr<<"> Sorting by RF Quality Index ..."<<endl;
    //stringstream ss;
    int i,kmax;
    if(ThreeComponentMode)
    	kmax=3;
    else
    	kmax=1;

	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(trace_order.size()>0) trace_order.clear();
				trace_order=teo_global.sort_by_RF_quality_index(
							*sensemble,RF_quality_index_weights[0],
							RF_quality_index_weights[1],
							RF_quality_index_weights[2],true);
				if(trace_order.size()>0)
				{
					sort_method=sensemble->get_string(sort_method_key);
					comp_tmp=new TimeSeriesEnsemble(*sensemble);
					XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
						(XtPointer) (comp_tmp),ExmNseiswMetadata,
						(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
				}
			}
		}
		XtAppUnlock(AppContext);
	}catch(...) {throw;};
	cerr<<"> ";
}
//print trace order (event ids in rank order consistent with the current view).
void print_trace_order_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->print_trace_order();
}
void TraceEditPlot::print_trace_order()
{
	if(this->sort_method.length()>0)
	{
		cerr<<endl<<sort_method<<endl;
		cerr<<"evid    rank"<<endl;
		vector<int>::iterator iptr;
		int i=0;
		for(iptr=trace_order.begin();iptr!=trace_order.end();++i,++iptr)
			cerr<<*iptr<<"    "<<trace_order.size()-i<<endl;
		cerr<<"> ";
	}
	else
		cerr<<"ERROR: sort_method not set. Sort by some methods before printing the trace order."
			<<endl<<"> ";
}
void reverse_trace_order_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->reverse_trace_order();
}
void TraceEditPlot::reverse_trace_order()
{
	//this->backup_undo_tse();
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	cerr<<"Reversing trace order ..."<<endl<<"> ";

    int i,kmax;
    if(ThreeComponentMode)
    	kmax=3;
    else
    	kmax=1;
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
				if(trace_order.size()>0) trace_order.clear();
				trace_order=teo_global.reverse_order(*sensemble);
				comp_tmp=new TimeSeriesEnsemble(*sensemble);
				XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
							(XtPointer) (comp_tmp),ExmNseiswMetadata,
							(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
			}
		}
		XtAppUnlock(AppContext);
	}catch(...) {throw;};
}
//////////////////////////////////////////////////////////////
///////////Build View Menu////////////////////////////////////
//////////////////////////////////////////////////////////////
/*
plot stack trace. Modified from dbxcor method (the same name).
*/
/* This procedure creates the beam plot window and manages it */
void do_beam_plot_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->get_stack_trace();
    plot_handle->do_beam_plot();
}

void do_beam_plot_average_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    StackType st_tmp=plot_handle->stacktype;
    plot_handle->stacktype=BasicStack;
    plot_handle->get_stack_trace();
    plot_handle->stacktype=st_tmp;
    plot_handle->do_beam_plot();
}

void TraceEditPlot::get_stack_trace()
{
	try{
		TimeSeriesEnsemble * sensemble;
		if(this->stacktype==RobustSNR)
			cerr<<"Robust stacking window = < "<<robust_twin.start<<
				", "<<robust_twin.end<<" >"<<endl<<"> ";

		XtVaGetValues(seisw[0],ExmNseiswEnsemble,&sensemble,NULL);
		TimeWindow stack_twin=teo_global.find_common_timewindow(*sensemble);
		TimeSeries stack_tmp(sensemble->member[0]);
		cerr<<"Computing stack ..."<<endl<<"> ";
		stack_tmp=teo_global.get_stack(*sensemble,stack_twin,robust_twin,
								stacktype);

		//if(beam_tse!=NULL) delete beam_tse;
		beam_tse.member.clear();
		//beam_tse=new TimeSeriesEnsemble(0,stack_tmp.ns);
		beam_tse.member.push_back(stack_tmp);
		string beam_title;
		if(this->stacktype==RobustSNR)
			beam_title=string("Robust Stack Trace for Station: ")+sensemble->get_string("sta");
		else if(this->stacktype==BasicStack)
			beam_title=string("Average Stack Trace for Station: ")+sensemble->get_string("sta");
		beam_tse.put("beam_title",beam_title);
	}catch(SeisppError& serr)
	{
		cout<<"Error in getting stack trace! "<<endl
			<<serr.what()<<endl;
	}
}
//do selected single trace plot
void do_selectedtrace_plot_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
    TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
    plot_handle->select_trace_typename="PLOT_SELECTED_TRACE";
    plot_handle->add_select_trace_callback();
}

Widget get_top_shell(Widget w)
{
    while(w && !XtIsWMShell(w)) {
	w=XtParent(w);
    }
    return w;
}
void TraceEditPlot::do_beam_plot()
{
	Widget rshell_beam, beam_widget, pane, form, btn_arrival;
    XWindowAttributes xwa;
    Dimension h;
    int n;
    Arg args[10];
    
    try
    {
		int beam_hbox=250;
		beam_hbox=metadata.get_int("beam_hbox");
		bool beam_clip_data=metadata.get_bool("beam_clip_data");
		double beam_trace_spacing=metadata.get_double("beam_trace_spacing");
		double beam_xcur=metadata.get_double("beam_xcur");
		string beam_trace_axis_scaling=metadata.get_string("beam_trace_axis_scaling");
		int wbox=metadata.get_int("wbox");
		rshell_beam=XtVaCreatePopupShell ("Beam Plot",topLevelShellWidgetClass, 
					get_top_shell(seisw[0]),XmNtitle,"Beam Plot",XmNallowShellResize,
					True,XmNwidth,wbox+50,XmNheight,beam_hbox+100,XmNdeleteResponse,XmUNMAP,NULL);
		pane = XtVaCreateWidget("Beam Plot",xmPanedWindowWidgetClass, 
					rshell_beam,NULL);
	
		Metadata beam_display_md(metadata);
		beam_display_md.put("title",beam_tse.get_string("beam_title"));
		beam_display_md.put("hbox",beam_hbox);
		beam_display_md.put("clip_data",beam_clip_data);
		beam_display_md.put("blabel",(char *)"Beam Window");
		beam_display_md.put("blabel2",(char *)"Beam Window");
		beam_display_md.put("trace_spacing",beam_trace_spacing);
		beam_display_md.put("xcur",beam_xcur);
		beam_display_md.put("trace_axis_scaling",beam_trace_axis_scaling);
   
		TimeSeriesEnsemble *comp_tmp;
		comp_tmp=new TimeSeriesEnsemble(beam_tse);
		n=0;
		/*Previous had this.  This disables callbacks for MB3
		XtSetArg(args[n],(char *) ExmNdisplayOnly,1); n++;
		**************************************/
		XtSetArg(args[n],XmNpaneMinimum,100); n++;
		XtSetArg(args[n],XmNmarginHeight,100);n++;
		// Don't think this is really necessary or desirable
		//XtSetArg(args[n],(char *) ExmNcleanupData, static_cast<XtPointer>(beam_tse));n++;

		beam_widget=ExmCreateSeisw(pane,(char *) "Beam Plot",args,n);

		XtManageChild(beam_widget);
		/* the Seisw widget always initializes with a NULL data pointer.
		The following is necessary to get the data to display and to
		show pick markers */
		//DisplayMarkerDataRec tracemarkers;
		/*
		tracemarkers.beam_tw=TimeWindow(10,10);
		tracemarkers.beam_color="red";
		tracemarkers.title="testpick";
		tracemarkers.robust_tw=TimeWindow(8,12);
		tracemarkers.robust_color="blue";
		*/
		//XtVaSetValues(beam_widget,ExmNdisplayMarkers,(XtPointer)(&(tracemarkers)),NULL);
	
		XtVaSetValues(beam_widget,
			ExmNseiswEnsemble,static_cast<XtPointer>(comp_tmp),
			ExmNseiswMetadata,&beam_display_md,
			ExmNdisplayMarkers,(XtPointer)(&(plotboxmarkers)),NULL);
		//display tracemarkers.
		//XtVaSetValues(beam_widget,ExmNdisplayMarkers,(XtPointer)(&(tracemarkers)),NULL);
	
		n=0;
		XtSetArg(args[n],XmNuserData,this); n++;
		XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget, pane); n++;
		//XtSetArg(args[n],XmNpaneMinimum,50); n++;
		XtSetArg(args[n],XmNpaneMaximum,100); n++;
		/* These attach the button widgets to the top and bottom of the parent form
		XtSetArg(args[n],XmNtopAttachment,XmATTACH_FORM); n++;
		XtSetArg(args[n],XmNbottomAttachment,XmATTACH_FORM); n++;
		*/
	
		XtManageChild(pane);
		XtPopup (rshell_beam, XtGrabNone);
    
    	}catch(SeisppError& serr)
	{
		cerr<<"Error in TraceEditPlot::do_beam_plot():"<<endl;
		cerr<<serr.what()<<endl;
		exit(-1);
	}; 
}
/* Show Trace Metadata for each selected trace (one at a time).
*/
void show_trace_metadata_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->show_metadata_typename="FOR_PICKED_TRACE";
	plot_handle->show_trace_metadata();
}
void show_trace_metadata_evid_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->show_metadata_typename="SEARCH_FOR_EVID";
	plot_handle->show_trace_metadata();
}
void show_trace_metadata_all_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->show_metadata_typename="METADATA_FOR_ALL";
	plot_handle->show_trace_metadata();
}
void TraceEditPlot::show_trace_metadata()
{
	if(this->show_metadata_typename=="FOR_PICKED_TRACE")
	{
		this->select_trace_typename="SHOW_TRACE_METADATA";
		this->add_select_trace_callback();
	}
	else
	{
		TimeSeriesEnsemble * sensemble;
		bool btmp(true);
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					if(show_metadata_typename=="SEARCH_FOR_EVID")
					{
						long evidtmp;
						cerr<<"Evid of the trace you want to show metadata for: ";
						cin>>evidtmp;
						teo_global.show_metadata(*sensemble,evidtmp,use_decon_in_editing);
					}
					else if(show_metadata_typename=="METADATA_FOR_ALL")
					{
						cerr<<"Choose from below (attribute code):"<<endl
							<<"  1    Stack Weight"<<endl
							<<"  2    Ref-Xcorcoe "<<endl
							<<"  3    Decon-Success Index"<<endl
							<<"  4    RF Quality Index"<<endl
							<<"  5    decon.niteration"<<endl
							<<"  6    decon.nspike"<<endl
							<<"  7    decon.epsilon"<<endl
							<<"  8    decon.peakamp"<<endl
							<<"  9    decon.averamp"<<endl
							<<"  10   decon.rawsnr"<<endl
							<<"  11   magnitude"<<endl
							//debug
							<<"  12   seaz"<<endl<<endl
							<<"> Your choice: ";
						int itmp;
						MDtype mdt;
						string mdtag;
						cin>>itmp;
						switch(itmp)
						{
							case 1:
								mdtag=SEISPP::stack_weight_keyword;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 2:
								mdtag=xcorcoekey;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 3:
								mdtag=decon_success_index_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 4:
								mdtag=RF_quality_index_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 5:
								mdtag=decon_niteration_key;
								mdt=MDint;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 6:
								mdtag=decon_nspike_key;
								mdt=MDint;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 7:
								mdtag=decon_epsilon_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 8:
								mdtag=decon_peakamp_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 9:
								mdtag=decon_averamp_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 10:
								mdtag=decon_rawsnr_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							case 11:
								mdtag=magnitude_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							//debug
							case 12:
								mdtag=seaz_key;
								mdt=MDreal;
								btmp=teo_global.show_metadata(*sensemble,mdtag,mdt);
								break;
							default:
								cerr<<"Error in show_trace_metadata(): wrong attribute code."<<endl;
								btmp=false;
						}			
					}
				}
			}
			XtAppUnlock(AppContext);
			cerr<<"> ";
		}catch(...) {throw;};
	}
}
//enter mode of reviewing killed traces.
void review_killed_traces_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	if(!plot_handle->killed_trace_excluded)
		cerr<<"Please exclude killed traces before entering this mode."<<endl<<"> ";
	else
		plot_handle->review_killed_traces();
}
void TraceEditPlot::review_killed_traces()
{
	this->backup_undo_tse();
		undo_for_kill=false;
	killed_review_mode=true;
	// backup tse for normal trace edit.
	tse_bkp_trace_edit0=tse_bkp_for_undo0;
	tse_bkp_trace_edit1=tse_bkp_for_undo1;
	tse_bkp_trace_edit2=tse_bkp_for_undo2;
	
	//report kills to 'kills' container.
	this->report_kills();
	
	//TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp, tse_tmp;
	cerr<<"Entering mode of 'Review Killed Traces' ..."<<endl;
	cerr<<"Edit is disabled in this mode."<<endl<<"> ";
	this->edit_disable_public();
    int i,kmax;
    if(ThreeComponentMode)
    	kmax=3;
    else
    	kmax=1;
    //set<TimeSeries>::iterator iptr;
    //if(tse_tmp.member.size()>0) tse_tmp.member.clear();
	try 
	{
		XtAppLock(AppContext);
		for(i=0;i<kmax;++i)
		{
			if(seisw[i]!=NULL)
			{
				teo_global.remove_duplicates(killed_traces,true);
				//after undo edits, kills may have less members than killed_traces ensemble.
				if(kills.size()<killed_traces.member.size())
					killed_traces=teo_global.extract_traces(killed_traces,kills);
				if(killed_traces.member.size()>0)
				{
					//For some unknown reason, I cannot directly reverse killed trace live_status to live.
					//So here I am using an insufficient way to accomplish this. I am extracting from the
					//original ensemble (to keep the live status true) and copying the metadata from the 
					//killed_traces ensemble to keep killed_trace_key & killmethodkey.
					//build mdlist used to extract/copy metadata from killed_traces to new extracted
					//traces for plotting.
					Metadata_typedef mdtmp;
					MetadataList mdlist; //mdlist to copy from killed_traces to new extracted traces.
					mdtmp.tag=string(killed_trace_key);
					//mdtmp.tag=string("test_key");
					mdtmp.mdt=MDboolean;
					mdlist.push_back(mdtmp);
					mdtmp.tag=string(killmethodkey);
					mdtmp.mdt=MDstring;
					mdlist.push_back(mdtmp);
					tse_tmp=teo_global.extract_traces(original_ensemble[i],killed_traces,mdlist);
					if(comp_tmp->member.size()>0)
					{	comp_tmp=new TimeSeriesEnsemble(tse_tmp);
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
				else
					cerr<<"ERROR: Can't enter killed trace review mode."
						<<" Killed traces extracted = "<<killed_traces.member.size()<<endl
						<<"> ";
			}
		}
		XtAppUnlock(AppContext);
	}catch(SeisppError& serr) 
	{cerr<<"**Error in review_killed_trace()"<<endl;
	serr.what();
	};
}
void restore_killed_traces_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	if(plot_handle->restored_killed_traces_list.size()>0) 
		plot_handle->restored_killed_traces_list.clear();
	plot_handle->select_trace_typename="RESTORE_KILLED_TRACE";
	plot_handle->add_select_trace_callback();
}
void TraceEditPlot::restore_killed_traces()
{
	//select traces to restore
	TimeSeriesEnsemble tse_tmp;
	undo_for_live=false;
	if(restored_killed_traces_list.size()>0)
	{
		undo_for_live=true;
		int i,kmax;
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try 
		{
			for(i=0;i<kmax;++i)
			{
				tse_tmp=teo_global.extract_traces(original_ensemble[i],
										restored_killed_traces_list,false);
				switch(i)
				{
					case 0:
						teo_global.merge_ensembles(*tse_bkp_trace_edit0,tse_tmp,true);
						break;
					case 1:
						teo_global.merge_ensembles(*tse_bkp_trace_edit1,tse_tmp,true);
						break;
					case 2:
						teo_global.merge_ensembles(*tse_bkp_trace_edit2,tse_tmp,true);
						break;
				}
			}
		}catch(...) {throw;};
		cerr<<"Restored [ "<<restored_killed_traces_list.size()<<" ] traces"<<endl<<">"; 
		TraceEditStatistics tes;
		tes.station=tse_bkp_trace_edit0->get_string("sta");
		tes.method=ManualRestore;
		tes.nkilled=-1*restored_killed_traces_list.size();
		teo_global.add_to_statistics(tes);
	}
	else
		cerr<<"No trace needs to be restored."<<endl;
}
void exit_review_killed_traces_callback(Widget w, 
				XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->exit_review_killed_traces();	
}
void restore_and_exit_review_killed_traces_callback(Widget w, 
				XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->restore_killed_traces();
	plot_handle->exit_review_killed_traces();	
}
void TraceEditPlot::exit_review_killed_traces()
{
	if(killed_review_mode)
	{
		//this->undo_auto_edits();
		TimeSeriesEnsemble * sensemble;
		TimeSeriesEnsemble *comp_tmp;
		int i,kmax;

		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		try
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					//XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					//teo_global.undo_kills_to_trace(*sensemble,last_autokill_list);
					switch(i)
					{
						case 0:
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_trace_edit0);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_trace_edit0->member.clear();
							break;
						case 1:
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_trace_edit1);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_trace_edit1->member.clear();
							break;
						case 2:
							comp_tmp=new TimeSeriesEnsemble(*tse_bkp_trace_edit2);
							cerr<<"Number of traces = "<<comp_tmp->member.size()<<endl;
							XtVaSetValues(seisw[i],ExmNseiswEnsemble, 
								(XtPointer) (comp_tmp),ExmNseiswMetadata,
								(XtPointer)(dynamic_cast<Metadata*>(this)),NULL);
							tse_bkp_trace_edit2->member.clear();
							break;
					}
				}
			}
			XtAppUnlock(AppContext);

		}catch(SeisppError& serr)
		{
			serr.what();
		}
		cerr<<"Exiting mode of 'Review Killed Traces' ..."<<endl;
		cerr<<"Edit is enabled now."<<endl<<"> ";
		this->edit_enable_public();
		killed_review_mode=false;
	}
	else
		cerr<<"ERROR: Not in the mode of 'Review Killed Traces'."<<endl;
}
//save selected trace to file as timeseries
/*
void save_timeseries_to_file_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->select_trace_typename="SAVE_TIMESERIES_TO_FILE";
	plot_handle->add_select_trace_callback();
}
*/
//Exclude killed traces.
void exclude_killed_trace_callback(Widget w, XtPointer client_data, XtPointer userdata)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);  
	plot_handle->exclude_killed_trace();	
}
void TraceEditPlot::exclude_killed_trace()
{
	this->backup_undo_tse();
		undo_for_kill=false;
	//backup original data.
	this->backup_original_tse();
	
	TimeSeriesEnsemble * sensemble;
	TimeSeriesEnsemble *comp_tmp;
	TimeSeriesEnsemble tse_tmp;
	//report kills to 'kills' container.
	int itmp=kills.size();
	this->report_kills();
	if(kills.size()>itmp)
		{
		int i,kmax;
	
		if(ThreeComponentMode)
			kmax=3;
		else
			kmax=1;
		vector<TimeSeries>::iterator dptr;
		try 
		{
			XtAppLock(AppContext);
			for(i=0;i<kmax;++i)
			{
				if(seisw[i]!=NULL)
				{
					//cerr<<"ex test1"<<endl;
					XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
					tse_tmp=teo_global.extract_false_traces(*sensemble,true);
					if(tse_tmp.member.size()>0)
						for(dptr=tse_tmp.member.begin();dptr!=tse_tmp.member.end();++dptr)
							killed_traces.member.push_back(*dptr);
					comp_tmp=new TimeSeriesEnsemble(teo_global.exclude_false_traces(*sensemble));
					if(comp_tmp->member.size()==0) 
					{
						cout<<"Warning: ensemble is empty after excluding killed traces. "
							<<"Keep current view for display purpose only!"<<endl;
						//comp_tmp=NULL;
						//XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (NULL), NULL);
					}
					else
					{
						XtVaSetValues(seisw[i],ExmNseiswEnsemble, (XtPointer) (comp_tmp),
							ExmNseiswMetadata,(XtPointer)(dynamic_cast<Metadata*>(this)), NULL);
					}
				}
			}
			XtAppUnlock(AppContext);
		}catch(SeisppError& serr) 
		{cerr<<"**Error when excluding killed traces!"<<endl;
			serr.what();exit(-1);};
		killed_trace_excluded=true;
	}
	else
		cerr<<"No killed traces in current view. No need to exclude killed traces."<<endl<<"> ";
}

void TraceEditPlot::report_kills()
{
    int i,kmax;
	if(ThreeComponentMode)
		kmax=3;
	else
		kmax=1;
    try
    {
		for(i=0;i<kmax;++i)
		{
			TimeSeriesEnsemble * sensemble;
			XtVaGetValues(seisw[i],ExmNseiswEnsemble,&sensemble,NULL);
			set<long> local_kills=teo_global.find_false_traces(*sensemble);
			if(local_kills.size()>0)
				this->kills.insert(local_kills.begin(),local_kills.end());  
		}  
		cerr<<"Total number of kills: "<<kills.size()<<"> "<<endl;
	}catch(...){throw;};
}
void report_statistics_callback(Widget w, 
		XtPointer client_data, XtPointer call_data)
{
	TraceEditPlot *plot_handle=reinterpret_cast<TraceEditPlot *>(client_data);
	plot_handle->report_statistics();
}
/*
        Statistical data: number of kills, number of total traces left, 
        auto-killing procedures applied, filters applied,
        reference trace xcor time window, robust timewindow,etc
        */
void TraceEditPlot::report_statistics()
{
	this->report_kills();
	Metadata summary=
		teo_global.get_statistics_summary();
	int nkilled_auto=summary.get_int(TOTAL_NKILLED_AUTO);
	int nkilled_manual=kills.size() - nkilled_auto;
	int ntraces_original=original_ensemble[0].member.size();
	int nleft=ntraces_original-kills.size();
	double acceptance_rate=100.0*((double)nleft/(double)ntraces_original);
	TimeSeriesEnsemble * sensemble;
	XtVaGetValues(seisw[0],ExmNseiswEnsemble,&sensemble,NULL);
	cerr<<"--- Statistics from TraceEditPlot ---"<<endl;
	cerr<<"Working on station       "<<sensemble->get_string("sta")<<endl;
	cerr<<"Number of kills          "<<kills.size()<<endl
		<<"Traces in current view   "<<sensemble->member.size()<<endl
		<<"Traces in original data  "<<ntraces_original<<endl
		<<"Auto-killed traces       "<<nkilled_auto<<endl
		<<"Manual-killed traces     "<<nkilled_manual<<endl
		<<"Traces left              "<<nleft<<endl
		<<"Acceptance Rate (%)      "<<acceptance_rate<<endl<<endl;
	teo_global.report_statistics();
	cerr<<"------ End of statistical data ------"<<endl<<"> ";
}
//modified by Xiaotao Yang adding two pull down menues.
// Default forces single trace edit mode so use that label here.
void TraceEditPlot::build_edit_menu()
{
    //build "Edit" menu.
    MenuItem edit_submenu_manual[]={
        {const_cast<char *>(cutlabel.c_str()),
        &xmPushButtonGadgetClass,(char)'i',
        (char *)"<Key>I",(char *)"I",
        turn_on_single_trace_mode_callback,
        this,NULL,(MenuItem *)NULL},
        {const_cast<char *>(stlabel.c_str()),
        &xmPushButtonGadgetClass,(char)'o',
        (char *)"<Key>O",(char *)"O",
        turn_on_cutoff_mode_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem kill_large_amplitude_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_large_amplitude_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_large_amplitude_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem kill_negative_FA_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_negative_FA_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_negative_FA_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    /*
    MenuItem kill_small_FA_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_small_FA_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_small_FA_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    */
    MenuItem kill_LFC_traces_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_LFC_traces_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_LFC_traces_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem kill_growing_PCoda_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_growing_PCoda_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize P-Coda Search Window",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_growing_PCoda_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem kill_ClusteredArrivals_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_ClusteredArrivals_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_ClusteredArrivals_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_submenu_auto[]={
    	{(char *)"<Apply All Procedures>",
        &xmPushButtonGadgetClass,'A',
        (char *)"<Key>A",(char *)"A",
        apply_all_autokill_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Kill Large Amplitude Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_large_amplitude_submenu},
        {(char *)"Kill Negative FAs Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_negative_FA_submenu},
        /*{(char *)"Kill Small ABS FA Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_small_FA_submenu},
        */
        {(char *)"Kill Growing P-Coda Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_growing_PCoda_submenu},
        {(char *)"Kill Clustered-Arrivals Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_ClusteredArrivals_submenu},
        /*
        {(char *)"*Kill Low-Frequency-Contaminated Traces*",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,kill_LFC_traces_submenu},
        */
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_niteration_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_niteration_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_niteration_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_nspike_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_nspike_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_nspike_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_epsilon_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_epsilon_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_epsilon_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_peakamp_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_peakamp_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_peakamp_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_averamp_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_averamp_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_averamp_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_decon_rawsnr_submenu[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_decon_rawsnr_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_rawsnr_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    //submenu for autokill procedure: Decon Parameter Threshold
    //Decon parameter list: niteration nspike epsilon peakamp averamp rawsnr
    MenuItem edit_submenu_decon_threshold[]={
    	{(char *)"<Cutoffs On All Decon-Attributes>",
        &xmPushButtonGadgetClass,(char)'D',
        (char *)"<Key>D",(char *)"D",
        apply_kill_decon_par_for_all_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"niteration",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_niteration_submenu},
    	{(char *)"nspike",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_nspike_submenu},
        {(char *)"epsilon",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_epsilon_submenu},
        {(char *)"peakamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_peakamp_submenu},
        {(char *)"averamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_averamp_submenu},
        {(char *)"rawsnr",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_decon_rawsnr_submenu},
        {(char *)"<Customize & Apply All>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_decon_threshold_for_all_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_submenu_stack_weight[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_low_stackweight_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_low_stackweight_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_submenu_dsi[]={
    	{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_low_dsi_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_low_dsi_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_submenu_ref_correlation[]={
    	{(char *)"<Apply (With Picked Ref-Trace)>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_low_ref_correlation_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"<Apply (With Stack-Trace)>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_low_ref_correlation_stacktrace_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Set Threshold & Apply",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_kill_low_ref_correlation_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    MenuItem edit_menu[]={
        //undo last auto-kill
        {(char *)"Undo Auto-Edits",
        &xmPushButtonGadgetClass,(char)'U',
        (char *)"<Key>U",(char *)"U",
        undo_auto_edits_callback,
        this,NULL,(MenuItem *)NULL},
        //manual edit mode and its submenus
        {(char *)"Manual-Editing Modes",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_manual},
        //auto edit mode and its submenus
        {(char *)"Trace-Based Procedures",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_auto},
        {(char *)"Decon-Based Procedures",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_decon_threshold},
        {(char *)"  >> DSI Cutoff",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_dsi},
        {(char *)"Stack Weight Cutoff",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_stack_weight},
        {(char *)"Ref-Trace Correlation Cutoff",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,edit_submenu_ref_correlation},
        {(char *)"RF Quality Index Cutoff",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        apply_kill_low_RFQualityIndex_callback,
        this,NULL,(MenuItem *)NULL},
        //enable edit.
        {(char *)"Enable Editing",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        edit_enable_callback,
        this,NULL,(MenuItem *)NULL},
        //disable edit.
        {(char *)"Disable Editing",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        edit_disable_callback,
        this,NULL,(MenuItem *)NULL},
        //undo all kills
        {(char *)"<Restore Ensemble>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        restore_ensemble_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
        
    menu_edit=BuildMenu(menu_bar,XmMENU_PULLDOWN,
            const_cast<char *>(edit_menu_label.c_str()),'E',
            false,edit_menu);  
    XtManageChild(menu_bar);
}
//build view menu
void TraceEditPlot::build_view_menu()
{
	MenuItem plot_stack_trace_submenu[]={
		{(char *)"Robust Stack",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        do_beam_plot_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Simple Average",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        do_beam_plot_average_callback,
        this,NULL,(MenuItem *)NULL},
		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	
	MenuItem killed_trace_review_submenu[]={
		{(char *)"Enter",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        review_killed_traces_callback,
        this,NULL,(MenuItem *)NULL},
        //restore_killed_traces_callback
        {(char *)"Select Restore Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        restore_killed_traces_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Apply Restoration & Exit",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        restore_and_exit_review_killed_traces_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Exit Only",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        exit_review_killed_traces_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem view_show_metadata_submenu[]={
		{(char *)"For Picked Trace",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)"<Key>M",(char *)"M",
        show_trace_metadata_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Search For Given Evid",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        show_trace_metadata_evid_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"For All With Given Attribute",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        show_trace_metadata_all_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem view_menu[]={
        {(char *)"Plot Stack Trace",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,plot_stack_trace_submenu},
        {(char *)"Plot Selected Single Trace",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        do_selectedtrace_plot_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Show Trace Metadata",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,view_show_metadata_submenu},
        {(char *)"Review Killed Traces",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,killed_trace_review_submenu},
        {(char *)"Exclude Killed Traces",
        &xmPushButtonGadgetClass,(char)'x',
        (char *)"<Key>X",(char *)"X",
        exclude_killed_trace_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
    menu_view=BuildMenu(menu_bar,XmMENU_PULLDOWN,
            (char *)"View",'V',
            false,view_menu);  
    XtManageChild(menu_bar);
}
//build sort menu
void TraceEditPlot::build_sort_menu()
{
	MenuItem sort_by_ref_trace_submenu[]={
		{(char *)"<Apply SBRT>",
        &xmPushButtonGadgetClass,(char)'R',
        (char *)"<Key>R",(char *)"R",
        sort_by_ref_trace_callback,
        this,NULL,(MenuItem *)NULL},
        //
        {(char *)"Pick Reference Trace",
        &xmPushButtonGadgetClass,(char)'P',
        (char *)"<Key>P",(char *)"P",
        pick_rt_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Use Stack Trace as Reference",
        &xmPushButtonGadgetClass,(char)'k',
        (char *)"<Key>K",(char *)"K",
        sort_by_xcor_to_stacktrace_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize Correlation Window",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_xcor_window_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem sort_by_stack_weight_submenu[]={
		{(char *)"<Apply SBSW>",
        &xmPushButtonGadgetClass,(char)'W',
        (char *)"<Key>W",(char *)"W",
        sort_by_stack_weight_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Customize Robust Window",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_robust_window_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem sort_by_decon_submenu[]={
		{(char *)"niteration",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_niteration_callback,
        this,NULL,(MenuItem *)NULL},
    	{(char *)"nspike",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_nspike_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"epsilon",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_epsilon_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"peakamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_peakamp_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"averamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_averamp_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"rawsnr",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_decon_rawsnr_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"<Decon-Success Index>",
        &xmPushButtonGadgetClass,(char)'c',
        (char *)"<Key>C",(char *)"C",
        sort_by_decon_successindex_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem sort_by_quality_index_submenu[]={
		{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)"<Key>L",(char *)"L",
        sort_by_quality_index_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Set Weights",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        customize_weights_for_quality_index_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem sort_menu[]={
        {(char *)"By Event Magnitude",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        sort_by_magnitude_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"By Correlation With Ref-Trace",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,sort_by_ref_trace_submenu},
        //
        {(char *)"By Stack Weight",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,sort_by_stack_weight_submenu},
        {(char *)"By Decon Parameters",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,sort_by_decon_submenu},
        {(char *)"By RF Quality Index",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,sort_by_quality_index_submenu},
        {(char *)"Print Trace Order",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        print_trace_order_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"<Reverse Trace Order>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        reverse_trace_order_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
    menu_sort=BuildMenu(menu_bar,XmMENU_PULLDOWN,
            (char *)"Sort",'S',
            false,sort_menu);  
    XtManageChild(menu_bar);
}
// void TraceEditPlot::build_filter_menu()
// {
// 	MenuItem filter_gaussian_submenu[]={
// 		{(char *)"<Apply>",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
//         {(char *)"Set Filter Specs",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
// 		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
// 	};
// 	MenuItem filter_ricker_submenu[]={
// 		{(char *)"<Apply>",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
//         {(char *)"Set Filter Specs",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
// 		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
// 	};
// 	MenuItem filter_customize_submenu[]={
// 		{(char *)"<Apply Customized Filter>",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
//         {(char *)"Set Filter Specs",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
// 		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
// 	};
// 	MenuItem filter_menu[]={
// 		{(char *)"0.02 ~ 1.0 Hz 2-Pole BP BW",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
//         {(char *)"0.02 ~ 5.0 Hz 2-Pole BP BW",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,(MenuItem *)NULL},
//         {(char *)"Gaussian Wavelet",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,filter_gaussian_submenu},
//         {(char *)"Ricker Wavelet",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,filter_ricker_submenu},
//         {(char *)"<Customize Filter>",
//         &xmPushButtonGadgetClass,(char)NULL,
//         (char *)NULL,(char *)NULL,
//         NULL,
//         this,NULL,filter_customize_submenu},
// 		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
// 	};
// 	menu_filter=BuildMenu(menu_bar,XmMENU_PULLDOWN,
//             (char *)"Filter",'l',
//             false,filter_menu);  
//     XtManageChild(menu_bar);
// }
void TraceEditPlot::build_tools_menu()
{
	MenuItem tools_filter_gaussian_submenu[]={
		{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Set Filter Specs",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem tools_filter_ricker_submenu[]={
		{(char *)"<Apply>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Set Filter Specs",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem tools_filter_customize_submenu[]={
		{(char *)"<Apply Customized Filter>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Set Filter Specs",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem tools_filter_submenu[]={
		{(char *)"0.02 ~ 1.0 Hz 2-Pole BP BW",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"0.02 ~ 5.0 Hz 2-Pole BP BW",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Gaussian Wavelet",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,tools_filter_gaussian_submenu},
        {(char *)"Ricker Wavelet",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,tools_filter_ricker_submenu},
        {(char *)"<Customize Filter>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,tools_filter_customize_submenu},
		{NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem tools_compute_decon_submenu[]={
		{(char *)"nspike",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"peakamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"averamp",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"rfsnr",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {(char *)"<Set Data Window>",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
	};
	MenuItem tools_menu[]={
        //call report_statistics()
        {(char *)"Statistics",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        report_statistics_callback,
        this,NULL,(MenuItem *)NULL},
        {(char *)"Compute Decon-Attributes",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,tools_compute_decon_submenu},
        /*
        {(char *)"Apply Filter",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        NULL,
        this,NULL,tools_filter_submenu},
        */
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    menu_tools=BuildMenu(menu_bar,XmMENU_PULLDOWN,
            (char *)"Tools",'T',
            false,tools_menu);  
    XtManageChild(menu_bar);
}
void TraceEditPlot::build_help_menu()
{
	MenuItem help_menu[]={
        {(char *)"Print Help Message",
        &xmPushButtonGadgetClass,(char)NULL,
        (char *)NULL,(char *)NULL,
        print_help_callback,
        this,NULL,(MenuItem *)NULL},
        {NULL,NULL,(char)NULL,(char *)NULL,(char *)NULL,NULL,NULL,NULL,(MenuItem *)NULL}
    };
    menu_help=BuildMenu(menu_bar,XmMENU_PULLDOWN,
            (char *)"Help",'H',
            false,help_menu);  
    XtManageChild(menu_bar);
}
void TraceEditPlot::reset_local_variables()
{
    edit_mode=SINGLE_TRACE_EDIT_MODE;
    allow_edit=true;
    this->edit_enable();
    ref_trace_picked=false;
    killed_review_mode=false;
    for(int i=0;i<3;++i) tse_backup_set[i]=true;
    ref_trace_evid=-1;
    ref_trace_number=-1;
    last_autokill_list.clear();
    restored_killed_traces_list.clear();
    killed_traces.member.clear();
    sort_method="";
//    ref_trace_xcor_twin_set=false;
    if(initialize_by_metadata)
    	this->set_defaults(metadata);
    else
    	this->set_defaults();
}
void TraceEditPlot::set_defaults()
{
	stacktype=RobustSNR;
	//RFeditor 3.0 removed/hided kill small FA method
	
	min_normalized_FA_amplitude=0.00;
	amplitude_normalization_method="LOCAL_MAX";
			//#could only be either GLOBAL_MAX, GLOBAL_FA_MAX or LOCAL_MAX
	FA_detect_length=0.8;
	FA_search_window=TimeWindow(0.0, 0.0);
	LowFrequency_min=0.00001, LowFrequency_max=0.1;
	max_trace_abs_amplitude=1000;
	NFA_tolerance_window=TimeWindow(0.0,120.0);
	PCoda_search_window=TimeWindow(0.0,MYZERO*1e-5);
	CodaCA_search_window=TimeWindow(0.0,MYZERO*1e-5);
	CodaCA_tolerance_twin_length=1e5;
	PCoda_grow_tolerance=0.0;
    robust_twin=TimeWindow(0.0,120.0);
    use_decon_in_editing=false;
    use_netmag_table=false;
    ref_trace_xcor_twin_set=false;
    //default decon thresholds.
    nspike_min=5;
	nspike_max=1000;
	niteration_min=5;
	niteration_max=1000;
	rawsnr_min=1;
	rawsnr_max=1000;
	averamp_min=0.001;
	averamp_max=1.0;
	epsilon_min=0.0;
	epsilon_max=80.0;
	peakamp_min=0.001;
	peakamp_max=1;
	min_stackweight=0.0;
	min_xcorcoe=-9999.0;
	min_dsi=0.0;
	RF_quality_index_weights[0]=0.33;
	RF_quality_index_weights[1]=0.33;
	RF_quality_index_weights[2]=0.33;
	decon_nspike_key="decon.nspike";
	decon_rawsnr_key="decon.rawsnr";
	decon_averamp_key="decon.averamp";
	decon_epsilon_key="decon.epsilon";
	decon_niteration_key="decon.niteration";
	decon_peakamp_key="decon.peakamp";
}
void TraceEditPlot::set_defaults(Metadata& md)
{
	double FA_reference_time=md.get_double("FA_reference_time");
	stacktype=RobustSNR;
	//string st_tmp=md.get_string("stacktype");
	/*
	if(st_tmp=="RobustSNR")
	{
		stacktype=RobustSNR;
	}
	else if(st_tmp=="BasicStack")
	{
		stacktype=BasicStack;
		cerr<<"Warning: for BasicStack, all traces have the same stack weight!"<<endl;
	}
	else if(st_tmp=="MedianStack")
	{
		stacktype=MedianStack;
		cerr<<"Warning: for MedianStack, all traces have the same stack weight!"<<endl;
	}
	else
	{
		cerr<<"ERROR: unknown stacktype in parameter file."<<endl;
		exit(-1);
	}
	*/
	//RFeditor 3.0 removed/hided kill small FA method
	//for kill_small_FA_amplitude procedure.
	/*
	FA_search_window=TimeWindow(0.0, 0.0);
	min_normalized_FA_amplitude=0.0;
	amplitude_normalization_method="LOCAL_MAX";
	FA_detect_length=0.8;

	if(md.is_attribute_set((char *)"min_normalized_FA_amplitude")) 
		min_normalized_FA_amplitude=md.get_double("min_normalized_FA_amplitude");
	if(md.is_attribute_set((char *)"amplitude_normalization_method"))
		amplitude_normalization_method=md.get_string("amplitude_normalization_method");
			//#could only be either GLOBAL_MAX, GLOBAL_FA_MAX or LOCAL_MAX
	if(md.is_attribute_set((char *)"FA_detect_length"))
		FA_detect_length=md.get_double("FA_detect_length");
	if(md.is_attribute_set((char *)"FA_search_TW_start") 
		&& md.is_attribute_set((char *)"FA_search_TW_end"))
		FA_search_window=TimeWindow(md.get_double("FA_search_TW_start")+FA_reference_time,
								md.get_double("FA_search_TW_end")+FA_reference_time);

	if(min_normalized_FA_amplitude>0.0)
	{
		cout<<"!!!Warning: min_normalized_FA_amplitude is detected to be greater than 0.0."<<endl
			<<"kill_small_FA_traces() is NOT recommended in this version of RFeditor (3.x)"<<endl
			<<"This procedure is kept in this version mainly for compatibility purpose."<<endl;
	}
	*/
	//for killing large amplitude traces.
	max_trace_abs_amplitude=md.get_double("max_trace_abs_amplitude");
	NFA_tolerance_window=TimeWindow(md.get_double("NFA_tolerance_TW_start")+FA_reference_time,
								md.get_double("NFA_tolerance_TW_end")+FA_reference_time);
	
	PCoda_search_window=TimeWindow(md.get_double("PCoda_search_TW_start")+FA_reference_time,
										md.get_double("PCoda_search_TW_end")+FA_reference_time);
    PCoda_grow_tolerance=md.get_double("PCoda_grow_tolerance");//normalized amplitude growth.
    CodaCA_search_window=TimeWindow(md.get_double("CodaCA_search_TW_start")+FA_reference_time,
										md.get_double("CodaCA_search_TW_end")+FA_reference_time);
	CodaCA_tolerance_twin_length=md.get_double("CodaCA_tolerance_twin_length");// 12.5
    if(CodaCA_tolerance_twin_length>CodaCA_search_window.length())
    {
    	cerr<<"ERROR: 'CodaCA_tolerance_twin_length'"<<
    		" must be shorter than 'CodaCA_search_window' length."<<endl;
    	exit(-1);
    }
    if(md.is_attribute_set((char *)("RefXcor_search_TW_start")) 
    		&& md.is_attribute_set((char *)("RefXcor_search_TW_end")))
    {
		ref_trace_xcor_twin=TimeWindow(md.get_double("RefXcor_search_TW_start")+FA_reference_time,
											md.get_double("RefXcor_search_TW_end")+FA_reference_time);
		ref_trace_xcor_twin_set=true;
    }
    else
    {
    	cerr<<"Warning: failed in reading RefXcor_search_TW_start or RefXcor_search_TW_end. Use whole data window."<<endl;
    	ref_trace_xcor_twin_set=false;
    }
//     try{
//     ref_trace_xcor_twin=TimeWindow(md.get_double("RefXcor_search_TW_start")+FA_reference_time,
// 										md.get_double("RefXcor_search_TW_end")+FA_reference_time);
// 	ref_trace_xcor_twin_set=true;
//     }catch(SeisppError& serr){
//     serr.what();
//     cerr<<"Warning: failed in reading RefXcor_search_TW_start or RefXcor_search_TW_end. Use whole data window."<<endl;
//     ref_trace_xcor_twin_set=false;
//     }
    //LowFrequency_min=md.get_double("LowFrequency_min");
    //LowFrequency_max=md.get_double("LowFrequency_max");
    
    robust_twin=TimeWindow(md.get_double("robust_window_start")+FA_reference_time,
        					md.get_double("robust_window_end")+FA_reference_time);
    //default decon thresholds.
    use_decon_in_editing=md.get_bool("use_decon_in_editing");
    use_netmag_table=md.get_bool("use_netmag_table");
    nspike_min=md.get_int("nspike_min"); //10
	nspike_max=md.get_int("nspike_max"); //400
	niteration_min=md.get_int("niteration_min"); //10
	niteration_max=md.get_int("niteration_max"); //400
	rawsnr_min=md.get_double("rawsnr_min"); //0.1
	rawsnr_max=md.get_double("rawsnr_max"); //100
	averamp_min=md.get_double("averamp_min"); //0.01
	averamp_max=md.get_double("averamp_max"); //10.0
	epsilon_min=md.get_double("epsilon_min"); //10.0
	epsilon_max=md.get_double("epsilon_max"); //80.0
	peakamp_min=md.get_double("peakamp_min"); //0.01
	peakamp_max=md.get_double("peakamp_max"); //100
	min_stackweight=0.0;
	min_dsi=0.0;
	min_xcorcoe=-9999.0;
	RF_quality_index_weights[0]=0.33;
	RF_quality_index_weights[1]=0.33;
	RF_quality_index_weights[2]=0.33;
	//cerr<<epsilon_min<<", "<<epsilon_max<<endl;
	decon_nspike_key=md.get_string("decon_nspike_key");
	decon_rawsnr_key=md.get_string("decon_rawsnr_key");
	decon_averamp_key=md.get_string("decon_averamp_key");
	decon_epsilon_key=md.get_string("decon_epsilon_key");
	decon_niteration_key=md.get_string("decon_niteration_key");
	decon_peakamp_key=md.get_string("decon_peakamp_key");
}

TraceEditPlot::TraceEditPlot() : SeismicPlot(),teo_global()
{
    // Always initialize in single trace edit mode
    edit_mode=SINGLE_TRACE_EDIT_MODE;
    allow_edit=true;
    ref_trace_picked=false;
    ref_trace_evid=-1;
    ref_trace_number=-1;
    initialize_by_metadata=false;
    ref_trace_xcor_twin_set=false;
    killed_trace_excluded=false;
    killed_review_mode=false;
    last_autokill_list.clear();
    sort_method="";
    //initialize tracemarkers
    tracemarkers.beam_tw=TimeWindow(0.0,0.0);
	tracemarkers.beam_color="red";
	tracemarkers.title="arrival";
	tracemarkers.robust_tw=TimeWindow(0.0,0.0);
	tracemarkers.robust_color="blue";
    if(killed_traces.member.size()>0) killed_traces.member.clear();
    this->set_defaults();
    this->edit_enable();
    //build trace edit plot menus.
    this->build_edit_menu();
    this->build_view_menu();
    this->build_sort_menu();
    //this->build_filter_menu();
    this->build_tools_menu();
    this->build_help_menu();
}
TraceEditPlot::TraceEditPlot(Metadata& md) : SeismicPlot(md),teo_global(md)
{
    /* Always initialize in single trace edit mode */
    edit_mode=SINGLE_TRACE_EDIT_MODE;
    allow_edit=true;
    ref_trace_picked=false;
    for(int i=0;i<3;++i) tse_backup_set[i]=true;
    ref_trace_evid=-1;
    ref_trace_number=-1;
    metadata=md;
    initialize_by_metadata=true;
    ref_trace_xcor_twin_set=false;
    killed_trace_excluded=false;
    killed_review_mode=false;
    last_autokill_list.clear();
    sort_method="";
    //initialize tracemarkers
    tracemarkers.beam_tw=TimeWindow(0.0,0.0);
	tracemarkers.beam_color="red";
	tracemarkers.title="arrival";
	tracemarkers.robust_tw=TimeWindow(0.0,0.0);
	tracemarkers.robust_color="blue";
    if(killed_traces.member.size()>0) killed_traces.member.clear();
    //TraceEditOperator temp(md);
    //teo_global=temp;
    //debug
    this->set_defaults();
    this->set_defaults(md); //call set_defaults() twice 
    			//in case the user doesn't want to customize all parameters.
    			// some of them could stay as default values.	
    //build trace edit plot menus.
    this->edit_enable();
    this->build_edit_menu();
    this->build_view_menu();
    this->build_sort_menu();
    //this->build_filter_menu();
    this->build_tools_menu();
    this->build_help_menu();
}