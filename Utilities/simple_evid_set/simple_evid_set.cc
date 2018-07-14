#include <iostream>
#include "db.h"
using namespace std;
void usage()
{
    cerr << "simple_evid_set db"<<endl;
    exit(-1);
}
int main(int argc, char **argv)
{
    if(argc!=2)usage();
    Dbptr db,dbe;
    if(dbopen(argv[1],"r+",&db))
    {
        cerr << "dbopen failed for database with name="<<argv[1]<<endl;
        usage();
    }
    /* Make sure no event table exists */
    dbe=dblookup(db,0L,"event",0L,0L);
    long nrec;
    dbquery(dbe,dbRECORD_COUNT,&nrec);
    if(nrec>0)
    {
        cerr << "event table found in database with "<<nrec<<" rows"<<endl
            << "Fatal error:  event table must be empty"<<endl;
        exit(-1);
    }
    db=dblookup(db,0L,"origin",0L,0L);
    dbquery(db,dbRECORD_COUNT,&nrec);
    cout << "Processing origin table with "<<nrec<<" rows"<<endl;
    long evid(1);
    for(db.record=0;db.record<nrec;++db.record)
    {
        long orid;
        dbgetv(db,0L,"orid",&orid,NULL);
        dbaddv(dbe,0L,"evid",evid,"prefor",orid,NULL);
        dbputv(db,0L,"evid",evid,NULL);
        ++evid;
    }
}


