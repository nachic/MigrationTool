/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2011 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list ofa
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
//
// This tool prints the execution trace of a program at thread granularity
//

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <cctype>
#include <stdlib.h>
#include <stdlib.h>
#include <iomanip>
#include <map> 
#include "pin.H"
#include <list>


// Command line switches for this tool.

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "Model_trace.out", "specify output file name");

KNOB<string> KnobExecutionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ExecFile","Execution_trace.out","specify output file name");



KNOB<string> KnobExceptionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ExceptionFile","Exception_trace.out","specify output file name");
KNOB<string> KnobModelExecutionOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelExecFile","ModelExecution_trace.out","specify output file name");

KNOB<string> KnobModelMapOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelMapFile","ModelMap_trace.out","specify output file name");


KNOB<string> KnobDataOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "DataFile","Data_trace.out","specify output file name");


KNOB<string> KnobMutexOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "MutexFile","Mutex_trace.out","specify output file name");


KNOB<string> KnobModelMutexOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelMutexFile","ModelMutex_trace.out","specify output file name");

KNOB<string> KnobDataSeqOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "DataseqFile","DataSeq_trace.out","specify output file name");

KNOB<string> KnobModelDataSeqOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelDataSeqFile","ModelDataSeq_trace.out","specify output file name");

KNOB<string> KnobThreadOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ThreadFile","Thread_trace.out","specify output file name");

KNOB<string> KnobModelThreadOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelThreadFile","ModelThread_trace.out","specify output file name");

KNOB<string> KnobModelDataOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "ModelDataFile","ModelData_trace.out","specify output file name");
struct threadregion
{
	THREADID tid;
  int node_type;  // 0 for thread, 1 for global variable, 2 for mutexes, 3 for conditions
	ADDRINT mutex_var;
     ADDRINT cond_var;
     ADDRINT global_var;
        char* name;// to specify routine
	string order;
        string cont;
OS_THREAD_ID oid;
};



struct globalregion
{
	THREADID tid;
  ADDRINT global_var;
	ADDRINT mutex_var;
     
	string type;
        string sync;
};

 
struct aglobalinregion
{

ADDRINT var;
THREADID tid[50];
int count;
};

struct bglobalinregion
{

ADDRINT var;
THREADID tid[50];
int count;
};

struct afbfglobalinregion
{

ADDRINT var;
THREADID tid[50];
string occ;
int count;
};
struct globalinregion
{
	

        THREADID tid;
       
	ADDRINT mutex_var;
     
	string type;
        string sync;
};

 struct cglobalinregion
{
 int scount;
 int count;
int bcount;
 THREADID tid;
};
struct countmutexregion
{
    THREADID tid;

	int count;
};
struct createregion
{
	THREADID tid;

	THREADID tid1;
    
    
};
struct signalregion
{
	THREADID tid;

	THREADID tid1;
        ADDRINT cond_var;
        ADDRINT mutex_var;
        string type;
    
    
};

struct exitregion
{
	THREADID tid;

	string type;
    
    
};

struct mutexregion
{
	THREADID tid;

	int flag;
       ADDRINT addr;
    
};


struct gmutexregion
{
	THREADID tid;

	string occ;
       ADDRINT addr;
    
};

/*struct signalregion
{
	THREADID tid;
  int node_type;  
	THREADID tid1[100];
        int count;
    
};

*/

ADDRINT lowaddr,highaddr;
int counter;
int creatcount=1;
int signalcount=1;
int exitcount=1;
int varcount=1;
int mutcount=1;
//int mutcount,datacount,sigcount;
//creatcount=0;
//mutcount=0;
//datacount=0;
//sigcount=0;
int dcount=0;
THREADID strthread;
THREADID mainthread;
static bool EnableModelDetection=false;
static bool EnableExecutionDetection=false;
static bool EnableDataSeqDetection=false;
static bool EnableDataDetection=false;
static bool EnableThreadDetection=false;
static bool EnableMutexDetection=false;
//extern REG RegSkipNextR,RegSkipNextW;

#define PADSIZE 56
std::map<int,std::list<struct threadregion> > accessmap;
//std::map<int,std::list<struct mutexregion> > mutexmap;
//std::map<int,std::list<struct condregion> > condmap;
//std::map<int,std::list<struct globalregion> > globalmap;
//std::map<int,std::list<struct dataregion> > datamap;
//std::map<int,std::list<struct dataregionsync> > datasyncmap;
std::map<THREADID,std::list<THREADID> > createmap;
std::map<THREADID,std::list<THREADID> > signalmap;
std::map<THREADID,std::list<ADDRINT> > condmap;
std::list<THREADID> thlist;
std::list<ADDRINT> glist;
std::list<ADDRINT> mlist;
std::list<ADDRINT> blist;
std::map<ADDRINT,std::list<THREADID> > lmap;
std::map<ADDRINT,std::list<ADDRINT> > checkmap;
std::map<ADDRINT,std::list<struct afbfglobalinregion> > afbfbarrierglobalinmap;



std::map<ADDRINT,std::list<struct cglobalinregion> > cglobalinmap;
std::map<int,std::list<struct globalregion> > globalmap;
std::map<int,std::list<struct exitregion> > exitmap;
std::map<ADDRINT,std::list<struct globalinregion> > globalinmap;
std::map<ADDRINT,std::list<struct aglobalinregion> > mutexglobalinmap;


std::map<ADDRINT,std::list<struct bglobalinregion> > barrierglobalinmap;
std::map<ADDRINT,std::list<struct mutexregion> > globalmutmap;

std::map<int,std::list<struct gmutexregion> > gglobalmutmap;
std::map<ADDRINT,std::list<struct countmutexregion> > countmap;
std::map<ADDRINT,std::list<THREADID> > globalsyncmap;

std::map<int,std::list<struct createregion> >createseqmap;
std::map<int,std::list<struct signalregion> >signalseqmap;
//std::map<int,std::list<struct signalregion> > signalmap;
//std::map<char*,std::list<struct memregion> >::iterator it;
std::list<THREADID>::iterator itth;
std::map<int , std::list<struct threadregion> >::iterator it2;
//std::map<int , std::list<struct mutexregion> >::iterator itm;
//std::map<int , std::list<struct condregion> >::iterator itco;

std::map<int,std::list<struct createregion> > ::iterator itcrsq;
std::map<THREADID,std::list<THREADID> > ::iterator itcr;

std::map<THREADID,std::list<ADDRINT> > ::iterator itcc;
std::map<ADDRINT,std::list<ADDRINT> > ::iterator itck;
std::map<int,std::list<struct exitregion> >::iterator itex;

std::list<ADDRINT>::iterator itg;
std::list<ADDRINT>::iterator itm;
std::list<ADDRINT>::iterator itb;

std::map<ADDRINT,std::list<struct aglobalinregion> > :: iterator itmgrin;
std::map<ADDRINT,std::list<struct cglobalinregion> > :: iterator itcgrin;


std::map<ADDRINT,std::list<struct bglobalinregion> >:: iterator itbgrin;
std::map<ADDRINT,std::list<THREADID> > :: iterator itl;

std::map<ADDRINT,std::list<struct afbfglobalinregion> >::iterator itafbfgrin;

std::map<int,std::list<struct signalregion> > ::iterator itsrsq;
std::map<THREADID,std::list<THREADID> > ::iterator itsr;
std::map<int,std::list<struct globalregion> > :: iterator itgr;
std::map<ADDRINT,std::list<struct globalinregion> > :: iterator itgrin;

std::map<ADDRINT,std::list<struct mutexregion> > ::iterator itgrmu;
std::map<int,std::list<struct gmutexregion> > ::iterator itggrmu;
std::map<ADDRINT,std::list<struct countmutexregion> > ::iterator itgrcu;

//std::map<int,std::list<struct signalregion> > ::iterator its;


//std::map<THREADID, std::list<ADDRINT> > barriermap;
//std::map<THREADID, std::list<ADDRINT> >::iterator bit;
//std::map<THREADID, std::list<ADDRINT> >::iterator bit2;

//std::map<THREADID,OS_THREAD_ID> threadmap;
//std::map<THREADID,OS_THREAD_ID> ::iterator tmp;

 ofstream OutFile;
ofstream ExecFile;

ofstream ExceptionFile;
ofstream DataFile;
ofstream DataseqFile;
ofstream ThreadFile;
ofstream MutexFile;
ofstream ModelMapFile;
ofstream ModelExecFile;
ofstream ModelDataSeqFile;
ofstream ModelThreadFile;
ofstream ModelDataFile;
ofstream ModelMutexFile;
PIN_LOCK lock;

INT32 numThreads = 0;
class thread_data_tsu
{
  public:
    thread_data_tsu() : _rtncount(0) {}
    UINT64 _rtncount;
    OS_THREAD_ID oid;
   
    char rtnlist[600];
   UINT64 _rdcount;
    UINT64 _wrcount;
    UINT8 _pad[PADSIZE];
   OS_THREAD_ID t;
    UINT64 _fcount;
   
};


static  TLS_KEY tls_key;

thread_data_tsu* get_tls(THREADID threadid)
{
    thread_data_tsu* tdata = 
          static_cast<thread_data_tsu*>(PIN_GetThreadData(tls_key, threadid));
    return tdata;
}


static std::string TrimWhitespace(const std::string &inLine)
{
    std::string outLine = inLine;

    bool skipNextSpace = true;
    for (std::string::iterator it = outLine.begin();  it != outLine.end();  ++it)
    {
        if (std::isspace(*it))
        {
            if (skipNextSpace)
            {
                it = outLine.erase(it);
                if (it == outLine.end())
                    break;
            }
            else
            {
                *it = ' ';
                skipNextSpace = true;
            }
        }
        else
        {
            skipNextSpace = false;
        }
    }
    if (!outLine.empty())
    {
        std::string::reverse_iterator it = outLine.rbegin();
        if (std::isspace(*it))
            outLine.erase(outLine.size()-1);
    }
    return outLine;
}

   
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}



static BOOL DebugInterpreter(THREADID tid, CONTEXT *ctxt, const string &cmd, string *result, VOID *)
{
    std::string line = TrimWhitespace(cmd);
    *result = "";

    if (line == "help")
    {
        result->append("Model Parameters -- Derive Model Parameters.\n");
 result->append("Complete Execution Sequence -- Derive Execution Snapshot.\n");
result->append("Data Access Sequence -- Derive Data Access Sequence.\n");
result->append("Thread Summary -- Derive Thread Summary with respect to data access and routine Access.\n");
result->append("Data Summary -- Derive Data Summary with respect to access by threads and type of access.\n");
result->append("Mutex Access Summary -- Derive Mutex Access Summary with respect to access by threads.\n");
     
		return TRUE;
    }
    else if (line == "Model Parameters")
    {
   
            EnableModelDetection = true;
            *result = "Model Parameters Derivation on.\n";
       
		
        return TRUE;
    }
    
     else if (line == "Complete Execution Sequence")
    {
   
            EnableExecutionDetection = true;
            *result = "Complete Execution Sequence Capture on.\n";
       
		
        return TRUE;
    }

     else if (line == "Data Access Sequence")
    {
   
            EnableDataSeqDetection = true;
            *result = "Data Access Sequence on .\n";
       
		
        return TRUE;
    }
    else if (line == "Data Summary")
    {
   
            EnableDataDetection = true;
            *result = "Data Summary on.\n";
       
		
        return TRUE;
    }
    


    else if (line == "Thread Summary")
    {
   
            EnableThreadDetection = true;
            *result = "Thread Summary on.\n";
       
		
        return TRUE;
    }

else if (line == "Mutex Access Summary")
    {
   
            EnableMutexDetection = true;
            *result = "Mutex Access Summary on.\n";
       
		
        return TRUE;
    }
    
    return FALSE;   /* Unknown command */
}

VOID start(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"STARTS");


//char * name;


	GetLock(&lock, tid+1);
       // cout<<"Routine Name is " <<rtnName  ;
        //cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="AFTER";
          tr.cont="STARTS";
          tr.oid=PIN_GetPid();
     

	std::list<struct threadregion> tlist;
	tlist.push_back(tr);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}



VOID create(char* rtnName, THREADID tid, THREADID tid1)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
tdata->_fcount=(tdata->_fcount)+1;
strcat(tdata->rtnlist,"  CREATES");


	GetLock(&lock, tid+1);
//GetLock(&lock, tid1+1);
      // cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';



         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=0;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=12;
  tr1.oid=PIN_GetTid();
   tr1.cont="CREATES";
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="CREATES";
  tr.oid=PIN_GetTid();
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
 //ReleaseLock(&lock);
}

VOID barrier_func(char* rtnName, THREADID tid,ADDRINT adr)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  PUTS MEMORY BAR");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=adr;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Puts Memory bar";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID join(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  JOINS");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';



         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
  tr.oid=PIN_GetPid();
   tr.cont="JOINS";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

  

VOID lock_func_bef(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);

strcat(tdata->rtnlist,"  REQUESTS FOR LOCK");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Requests for locking the mutex whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Requests for locking the mutex whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID lock_func_af(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  ACQUIRES LOCK");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
   tr1.cont="Acquires the lock for mutex whose address is";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
   tr.cont="Acquires the lock for mutex whose address is";
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}



VOID unlock_func(char* rtnName, THREADID tid,ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNLOCKS THE MUTEX");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=0;
         tr1.mutex_var=addr;
   tr1.cont="Unlocks the mutex variable whose address is";
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=1;
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=0;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Unlocks the mutex variable whose address is";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}


VOID main_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  MAIN");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Executes Main";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Executes Main";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}


VOID throw_func(char* rtnName, THREADID tid, ADDRINT addr)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  THROWS EXCEPTION");

//char * name;


	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=addr;
        tr1.cond_var=00000;
        tr1.node_type=2;
   tr1.cont="Throws Exception";
  tr1.oid=PIN_GetPid();
	  tr1.order="AFTER";


         tr.name=rtnName;
         tr.tid=tid;
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
  tr.oid=PIN_GetPid();
   tr.cont="Throws Exception";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
tlist.push_back(tr1);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}


VOID cond_wait_func(char* rtnName, THREADID tid, ADDRINT adr1, ADDRINT adr2)
 { 
OutFile<<"Ho";
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
OutFile<<"COUNTER VALUE "<<counter1 <<'\n';

//char * name;

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAITS OVER CONDITION VARIABLE");
	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Waits over condition variable";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";


         tr.name=rtnName;
         tr.tid=0;
         tr.mutex_var=adr2;
	   tr.global_var=00;
        tr.cond_var=adr1;
        tr.node_type=3;
	  tr.order="AFTER";
  tr.oid=PIN_GetPid();
   tr.cont="Waits over condition variable";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 OutFile<<"COUNTER VALUE "<<counter1 <<'\n';
                       
 ReleaseLock(&lock);}



VOID malloc_func(char* rtnName, THREADID tid, ADDRINT adr1)
 { 

struct threadregion tr1;
int counter1;
counter1=counter;
counter++;


//char * name;

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  MALLOC");
	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=adr1;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="MALLOC";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";


	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID cond_signal_func(char* rtnName, THREADID tid, ADDRINT adr1)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;


//char * name;

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  SIGNALS");
	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
   tr1.cont="Signals";
  tr1.oid=PIN_GetPid();
	  tr1.order="BEFORE";




 
  for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{


 //cout<<"Event"<<it2->first;				
std::list<struct threadregion>::iterator lit;
				

for(lit=it2->second.begin(); lit!=it2->second.end(); ++lit)
{
//cout<<"1st loop"<<tid<<'\n';
    

   if(lit->cond_var==adr1)
{



//cout<<"cond signal loop "<<tid<<'\n';
//cout << lit->tid;
tr.tid=lit->tid;
tr.mutex_var=lit->mutex_var;
//lit->node_type=0;
//flag=1;

}
}			
}


         tr.name=rtnName;
         //tr.tid=0;
         //tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=adr1;
        tr.node_type=3;
	  tr.order="AFTER";
  tr.oid=PIN_GetPid();
   tr.cont="Signals";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}


VOID child_start(char* rtnName, THREADID tid)
 { 

struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
//THREADID tid2;
int flag;
flag=0;
//char nme[15]="pthread_create";
//char ord[6]="AFTER";


//char * name;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  STARTS");

	GetLock(&lock, tid+1);
       // cout<<"Routine Name is " <<rtnName  ;
       //cout<<'\n';
       // cout<<"Thread id is "<<tid ;
       // cout<<'\n';
        // tid2=PIN_GetParentTid();
        // cout<<tid2<<" parent"<<'\n';
         tr1.name=rtnName;
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=6;
  tr1.oid=PIN_GetPid();
   tr1.cont="STARTS";
	  tr1.order="BEFORE";


         tr.name=rtnName;
         tr.tid=tid;
   tr.cont="STARTS";
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=6;
  tr.oid=PIN_GetPid();
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
       tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       


 
  for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{


 //cout<<"Event"<<it2->first;				
std::list<struct threadregion>::iterator lit;
				

for(lit=it2->second.begin(); lit!=it2->second.end(); ++lit)
{
//cout<<"1st loop"<<tid<<'\n';
    

   if(lit->oid==PIN_GetParentTid() && lit->node_type==12 && flag==0)
{



//cout<<"2nd loop "<<tid<<'\n';
lit->tid=tid;
lit->node_type=0;
flag=1;

}
			
}
 ReleaseLock(&lock);

}	

   // fprintf(out, "process %d\n",PIN_GetParentTid());

   //fprintf(out, "process %d\n",PIN_GetPid());
  //  fflush(out);
    ReleaseLock(&lock);
}

VOID exit_func(char* rtnName, THREADID tid, int AGR)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
if(AGR==1)
{

strcat(tdata->rtnlist,"  EXITS DUE TO EXCEPTION");
 tr.cont="EXITS DUE TO EXCEPTION";
}

if(AGR==0)
{

strcat(tdata->rtnlist,"  EXITS NORMALLY");
 tr.cont="EXITS NORMALLY";
}


	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=9;
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID exit_handler_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  RUNS EXIT HANDLERS");

	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="RUNS EXIT HANDLERS";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}
VOID catch_exit(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  EXITS THE CATCH BLOCK");

	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Exits the Catch Block";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID catch_begin(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  CATCHES THE EXCEPTION");
	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Catches the Exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID unregister_fork(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  UNREGISTERS FORK HANLERS");

	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   

         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Unregisters fork handlers";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID exp_begin(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  DETECTS EXCEPTION");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Detects Exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID unwindraise_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Starts Unwinding Stack for finding the exception handler";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID pers_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Runs the personality functions for each routine";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID rethrow_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Rethrows the exception";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID currentexc_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="checks the current exception type, returns NULL if not handled";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}


VOID term_function(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Calls the default terminate function";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}


VOID set_GR(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Stores the arguments to be passed to next phase of exception hanlding which can be the cleanup phase or the last handler i.e landing pad";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID set_IP(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Sets the instruction pointer";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID kill(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Kills the Process";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID abort_func(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Aborts the Process which is the action of the default terminate function";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}

VOID urethrow(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
//thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  Starts Unwinding Stack for finding the exception handler");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Starts the search phase once more after exception is rethrown";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}
VOID exp_end(char* rtnName, THREADID tid)
 { 

struct threadregion tr;
int counter1;
counter1=counter;
counter++;
 //thread_data_tsu* tdata = get_tls(tid);
//tdata->_rtncount=(tdata->_rtncount)+1;
//strcat(tdata->rtnlist,"  COMPLETES EXCEPTION HANDLING");

	GetLock(&lock, tid+1);
    
         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
  
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
           tr.cont="Completed Exception handling";
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );              
 ReleaseLock(&lock);
}
VOID lock_wait_func(char* rtnName, THREADID tid)
 { 

 thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAITS FOR ACQUIRING LOCK");
struct threadregion tr;
int counter1;

counter1=counter;
counter++;



//char * name;


	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;

     //  cout<<'\n';
         
   


         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
   tr.cont="Waits for acquiring lock";
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;

	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}


VOID unlock_wake_func(char* rtnName, THREADID tid)
 { 

thread_data_tsu* tdata = get_tls(tid);
tdata->_rtncount=(tdata->_rtncount)+1;
strcat(tdata->rtnlist,"  WAKE UP WAITING THREADS");
//cout<<tdata->rtnlist<<'\n';
struct threadregion tr;
int counter1;

counter1=counter;
counter++;



//char * name;


	GetLock(&lock, tid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;

     //  cout<<'\n';
         
   


         tr.name=rtnName;
         tr.tid=tid;
  tr.oid=PIN_GetPid();
   tr.cont="Wakes up waiting threads";
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;

	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    


    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

VOID Routine(RTN rtn, VOID *v)
{
    
  if (RTN_Name(rtn)== "_start")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)start, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "pthread_create")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)create ,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);
  RTN_Close(rtn);
}



 if (RTN_Name(rtn)== "__pthread_mutex_lock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)lock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__pthread_mutex_trylock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock_func_bef,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

  
RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)lock_func_af,IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);



     RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "pthread_mutex_unlock")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unlock_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}

if (RTN_Name(rtn)== "12malloc")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)malloc_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_cond_wait")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_FUNCARG_ENTRYPOINT_VALUE, 1,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "pthread_join")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)join, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "main")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)main_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__lll_lock_wait")
{


  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)lock_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);


     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__lll_unlock_wake")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unlock_wake_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "start_thread")
{

  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)child_start, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

     RTN_Close(rtn);
}
 /*if (RTN_Name(rtn)== "pthread_cond_wait")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_wait_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_FUNCARG_ENTRYPOINT_VALUE, 1,IARG_END);

    RTN_Close(rtn);
}*/
if (RTN_Name(rtn)== "pthread_cond_signal")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)cond_signal_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "exit")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exit_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}


if (RTN_Name(rtn)== "__cxa_throw")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)throw_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__run_exit_handlers")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exit_handler_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "_Unwind_RaiseException")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unwindraise_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "__cxa_rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)rethrow_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "_ZSt9terminatev")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)term_function, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)rethrow_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__gxx_personality_v0")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)pers_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}



if (RTN_Name(rtn)== "_Unwind_SetIP")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)set_IP, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "_Unwind_SetGR")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)set_GR, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_end_catch")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)catch_exit, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "__cxa_begin_catch")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)catch_begin, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_free_exception")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exp_end, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "gsignal")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)kill, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "abort")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)abort_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}
if (RTN_Name(rtn)== "_Unwind_Resume_or_Rethrow")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)urethrow, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "__cxa_current_exception_type")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)currentexc_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

 if (RTN_Name(rtn)== "__cxa_allocate_exception")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)exp_begin, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}


 if (RTN_Name(rtn)== "__unregister_atfork")
{

    RTN_Open(rtn);

    // Insert a call to printip before every instruction, and pass it the IP
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)unregister_fork, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_END);

    RTN_Close(rtn);
}

if (RTN_Name(rtn)== "pthread_barrier_wait")
{
 
  RTN_Open(rtn);
RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)barrier_func, IARG_ADDRINT,RTN_Name(rtn).c_str(),IARG_THREAD_ID,IARG_FUNCARG_ENTRYPOINT_VALUE, 0,IARG_END);

     RTN_Close(rtn);
}

}


VOID RecordMemWrite(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID tid)
{

ADDRINT address=reinterpret_cast<ADDRINT>(addr);
 if(address >= lowaddr && address <= highaddr)
	{
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;

thread_data_tsu* tdata = get_tls(tid);
tdata->_wrcount=(tdata->_wrcount) + 1;
tdata->_rtncount=(tdata->_rtncount)+1;
//char * name;

char  nme[8]=" WRITE";

   
	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=nme;
   tr1.cont="Writes to variable whose address is";
         tr1.tid=tid;
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
	  tr1.order="BEFORE";


        tr.name=nme;
         tr.tid=0;
         tr.mutex_var=00000;
	   tr.global_var=address;
        tr.cond_var=00000;
        tr.node_type=2;
   tr.cont="Writes to variable whose address is";
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

}

VOID RecordMemRead(VOID * ip, VOID * addr,CONTEXT *ctxt, THREADID tid)
{

ADDRINT address=reinterpret_cast<ADDRINT>(addr);
 if(address >= lowaddr && address <= highaddr)
	{
struct threadregion tr,tr1;
int counter1;
counter1=counter;
counter++;
 thread_data_tsu* tdata = get_tls(tid);
tdata->_rdcount=(tdata->_rdcount)+1;
tdata->_rtncount=(tdata->_rtncount)+1;

char nme[7]= " READ";
        
    
	GetLock(&lock, tid+1);
        //cout<<"Routine Name is " <<rtnName  ;
       // cout<<'\n';
        //cout<<"Thread id is "<<tid ;
       // cout<<'\n';
         tr1.name=nme;
         tr1.tid=tid;
   tr1.cont="Reads from variable whose address is";
         tr1.mutex_var=00000;
	   tr1.global_var=00000;
        tr1.cond_var=00000;
        tr1.node_type=0;
	  tr1.order="BEFORE";


         tr.name=nme;
         tr.tid=0;
         tr.mutex_var=00000;
	   tr.global_var=address;
        tr.cond_var=00000;
        tr.node_type=2;
   tr.cont="Reads from variable whose address is";
	  tr.order="AFTER";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr1);
tlist.push_back(tr);

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);
}

}

VOID Instruction(INS ins, VOID *v)
{
    
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
				IARG_CONST_CONTEXT,
				IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
				IARG_CONST_CONTEXT,
				IARG_THREAD_ID,
                IARG_END);
        }
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
//cout<<" inside thread start";    

GetLock(&lock, threadid+1);
//OS_THREAD_ID td;
//td=PIN_GetPid();
THREADID th;

    numThreads++;


  // accessmap.insert(std::pair<THREADID,OS_THREAD_ID>(threadid, td) );
    

  thread_data_tsu* tdata = new thread_data_tsu;
   tdata->_rtncount=0;


   
    tdata->oid=PIN_GetParentTid();
  tdata->_rdcount=0;
  tdata->_wrcount=0;
   tdata->_fcount=0;
   strcat(tdata->rtnlist," ");
  
  tdata->t=PIN_GetTid();
//cout<< "THREAD OS ID" <<tdata->t;
   tdata->_fcount=0;  
th=threadid;  


	thlist.push_back(th);

    PIN_SetThreadData(tls_key, tdata, threadid);


   ReleaseLock(&lock);
}

//This routine is executed every time a thread is destroyed.
VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{

struct threadregion tr;
int counter1;
counter1=counter;
counter++;

char nme[8]=" Exits";

//char * name;


	GetLock(&lock, threadid+1);
     //  cout<<"Routine Name is " <<rtnName  ;
     //  cout<<'\n';
     //  cout<<"Thread id is "<<tid ;
     //  cout<<'\n';
         
   
        // cout<< nme;
         tr.name=nme;
         tr.tid=threadid;
  tr.oid=PIN_GetPid();
   tr.cont="EXITS";
         tr.mutex_var=00000;
	   tr.global_var=00000;
        tr.cond_var=00000;
        tr.node_type=0;
	  tr.order="BEFORE";
	std::list<struct threadregion> tlist;
	tlist.push_back(tr);
    

    
    accessmap.insert(std::pair<int, std::list<struct threadregion> >(counter1, tlist) );
 
                       
 ReleaseLock(&lock);

	

//    fflush(out);
    ReleaseLock(&lock);
}

VOID Image(IMG Img, VOID *v)
{

    
		unsigned found;
		if(IMG_IsMainExecutable(Img))
		{
			for (SEC sec = IMG_SecHead(Img); SEC_Valid(sec); sec = SEC_Next(sec))
			{ 
				found=SEC_Name(sec).compare(".bss");
			
				if(found==0)
				{
					printf("Address: %x ",SEC_Address(sec));
					//std::cout << " =>Section: " << setw(8) << SEC_Address(sec) << " " << SEC_Name(sec) << endl;
					lowaddr=SEC_Address(sec);
					//lowaddr=134519568;
					//lowaddr=134519257;	//global2	
					//lowaddr=IMG_LowAddress(Img);	
					printf("%x\n",lowaddr);
					//std::cout << "Low addr: " << lowaddr << '\n';
					//OutFile  << "Low addr: " << lowaddr << '\n';
					highaddr=IMG_HighAddress(Img);
					//std::cout << "High addr: " << highaddr << '\n';
					//OutFile << "High addr: " << highaddr << '\n';
					printf("%x\n",highaddr);
				}			
			}


	
		}
	
}


VOID Fini(INT32 code, VOID *v)
{
//int i;
for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{

 //cout<<'\n'<<"Event"<<" "<<it2->first<<" : ";				
std::list<struct threadregion>::iterator lit;
std::list<struct threadregion>::reverse_iterator lit1;
lit=it2->second.begin();
lit1=it2->second.rbegin();
//cout<<lit->cont;
//cout<<lit1->cont;
if(lit->order=="AFTER" && lit->cont=="STARTS" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';
 strthread=lit->tid;
}


if(lit->order=="BEFORE" && lit->cont=="STARTS" and lit->node_type==6)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Puts Memory bar" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var<<'\n';

struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=9;
tr.addr=lit->global_var;


itgrmu=globalmutmap.find(lit->global_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit->global_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}




for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

				
std::list<struct globalregion>::iterator littgr2;
				
for(littgr2=itgr->second.begin(); littgr2!=itgr->second.end(); ++littgr2)

{
if(littgr2->tid==lit->tid)

{
littgr2->mutex_var=lit->global_var;
littgr2->type="BEFORE";
littgr2->sync="BARRIER";

}
}


}

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

				
std::list<struct globalinregion>::iterator littgrin2;
				
for(littgrin2=itgrin->second.begin(); littgrin2!=itgrin->second.end(); ++littgrin2)

{
if(littgrin2->tid==lit->tid)

{
littgrin2->mutex_var=lit->global_var;
littgrin2->type="BEFORE";
littgrin2->sync="BARRIER";

}
}


}

for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

				
std::list<struct cglobalinregion>::iterator littcgrin2;
				
for(littcgrin2=itcgrin->second.begin(); littcgrin2!=itcgrin->second.end(); ++littcgrin2)

{
if(littcgrin2->tid==lit->tid)

{
littcgrin2->bcount=(littcgrin2->bcount)+1;

}
}


}

}

if(lit->order=="BEFORE" && lit->cont=="Requests for locking the mutex whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
struct mutexregion tr;
tr.tid=lit->tid;
tr.flag=2;
tr.addr=lit1->mutex_var;




itgrmu=globalmutmap.find(lit1->mutex_var);
 if(itgrmu==globalmutmap.end())
{

std::list<struct mutexregion> tlist1;
	tlist1.push_back(tr);

 globalmutmap.insert(std::pair<ADDRINT, std::list<struct mutexregion> >(lit1->mutex_var, tlist1) );
}
else
{

itgrmu->second.push_back(tr);

}

}




if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for mutex whose address is" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==2)
littgrmu->flag=1;

}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="LOCKS";
tr.addr=lit1->mutex_var;
int counterm=mutcount;
mutcount++;



itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}

}
if(lit->order=="BEFORE" && lit->cont=="Executes Main" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
 mainthread=lit->tid;
}


if(lit->order=="BEFORE" && lit->cont=="Wakes up waiting threads" and lit->node_type==0)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Waits for acquiring lock" and lit->node_type==0)

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="JOINS" and lit->node_type==0)

{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

struct exitregion tr;
int count;
count=exitcount;
exitcount++;
tr.tid=lit->tid;
tr.type="JOINS";

std::list<struct exitregion> tlist1;
	tlist1.push_back(tr);
 exitmap.insert(std::pair<int, std::list<struct exitregion> >(count, tlist1) );

}


if(lit->order=="BEFORE" && lit->cont=="Unlocks the mutex variable whose address is" and lit->node_type==0)
{


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{

 				
std::list<struct mutexregion>::iterator littgrmu;
				
for(littgrmu=itgrmu->second.begin(); littgrmu!=itgrmu->second.end(); ++littgrmu)
{
if(littgrmu->tid==lit->tid && littgrmu->addr==lit1->mutex_var && littgrmu->flag==1)
littgrmu->flag=0;

}

}



struct gmutexregion tr;
tr.tid=lit->tid;
tr.occ="UNLOCKS";
tr.addr=lit1->mutex_var;


int counterm=mutcount;
mutcount++;

itggrmu=gglobalmutmap.find(lit1->mutex_var);
 if(itggrmu==gglobalmutmap.end())
{

std::list<struct gmutexregion> tlist1;
	tlist1.push_back(tr);

 gglobalmutmap.insert(std::pair<int, std::list<struct gmutexregion> >(counterm, tlist1) );
}
else
{

itggrmu->second.push_back(tr);

}




//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}
if(lit->order=="BEFORE" && lit->cont=="CREATES" and lit->node_type==0)
{
struct createregion tr;
int count;
count=creatcount;
creatcount++;
tr.tid=lit->tid;
tr.tid1=lit1->tid;

std::list<struct createregion> tlist1;
	tlist1.push_back(tr);
 createseqmap.insert(std::pair<int, std::list<struct createregion> >(count, tlist1) );

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

 itcr=createmap.find(lit->tid);
 if(itcr==createmap.end())
 {
std::list<THREADID> tlist;
tlist.push_back(lit1->tid);
 createmap.insert(std::pair<THREADID,std::list<THREADID> >(lit->tid, tlist) );
 }

else
{
 itcr->second.push_back(lit1->tid);
}

}

if(lit->order=="BEFORE" && lit->cont=="Waits over condition variable" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->cond_var <<"  "<<'\n';
struct signalregion tr1;
int count;
count=signalcount;
signalcount++;
tr1.tid=lit->tid;
tr1.tid1=0;
tr1.type="WAITS";
tr1.cond_var=lit1->cond_var;
tr1.mutex_var=lit1->mutex_var;

std::list<struct signalregion> tlist1;
	tlist1.push_back(tr1);
 signalseqmap.insert(std::pair<int, std::list<struct signalregion> >(count, tlist1) );


itcc=condmap.find(lit->tid);
 if(itcc==condmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(lit1->cond_var);
 condmap.insert(std::pair<THREADID,std::list<ADDRINT> >(lit->tid, tlist) );
 }

else
{
 itcc->second.push_back(lit1->cond_var);
}




for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu1;				
for(litgrmu1=itgrmu->second.begin(); litgrmu1!=itgrmu->second.end(); ++litgrmu1)
{
 
if(litgrmu1->addr==lit1->mutex_var && litgrmu1->flag==1)
{
 
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{
//OutFile<<"IST LOOP E DHUKEHCEH";			
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
//OutFile<<"2ST LOOP E DHUKEHCEH";
if(littgr1->mutex_var==litgrmu1->addr)
{
//OutFile<<"3ST LOOP E DHUKEHCEH";

itck=checkmap.find(lit1->cond_var);
 if(itck==checkmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(littgr1->global_var);
 checkmap.insert(std::pair<ADDRINT,std::list<ADDRINT> >(lit1->cond_var, tlist) );
 }

else
{
 itck->second.push_back(littgr1->global_var);
}

}
}
}
}
}
}

}




if(lit->order=="BEFORE" && lit->cont=="Signals" and lit->node_type==0)
{
//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

{
dcount++;
struct signalregion tr;
int count;
count=signalcount;
signalcount++;
tr.tid=lit->tid;
tr.tid1=lit1->tid;
tr.type="SIGNALS";
tr.cond_var=0;
tr.mutex_var=0;

std::list<struct signalregion> tlist1;
	tlist1.push_back(tr);
 signalseqmap.insert(std::pair<int, std::list<struct signalregion> >(count, tlist1) );

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

 itsr=signalmap.find(lit->tid);
 if(itsr==signalmap.end())
 {
std::list<THREADID> tlist;
tlist.push_back(lit1->tid);
 signalmap.insert(std::pair<THREADID,std::list<THREADID> >(lit->tid, tlist) );
 }

else
{
 itsr->second.push_back(lit1->tid);
}

}



for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu1;				
for(litgrmu1=itgrmu->second.begin(); litgrmu1!=itgrmu->second.end(); ++litgrmu1)
{
 
if(litgrmu1->addr==lit1->mutex_var && litgrmu1->flag==1)
{
 
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{
//OutFile<<"IST LOOP E DHUKEHCEH";			
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
//OutFile<<"2ST LOOP E DHUKEHCEH";
if(littgr1->mutex_var==litgrmu1->addr)
{
//OutFile<<"3ST LOOP E DHUKEHCEH";

itck=checkmap.find(lit1->cond_var);
 if(itck==checkmap.end())
 {
std::list<ADDRINT> tlist;
tlist.push_back(littgr1->global_var);
 checkmap.insert(std::pair<ADDRINT,std::list<ADDRINT> >(lit1->cond_var, tlist) );
 }

else
{
 itck->second.push_back(littgr1->global_var);
}

}
}
}
}
}
}

}

if(lit->order=="BEFORE" && lit->cont=="EXITS" and lit->node_type==0)
{

//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
struct exitregion tr1;
int count;
count=exitcount;
exitcount++;
tr1.tid=lit->tid;
tr1.type="EXITS";

std::list<struct exitregion> tlist1;
	tlist1.push_back(tr1);
 exitmap.insert(std::pair<int, std::list<struct exitregion> >(count, tlist1) );
}

if(lit->order=="BEFORE" && lit->cont=="Writes to variable whose address is" and lit->node_type==0)

{


//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';



 itgrmu=globalmutmap.find(lit1->global_var);
 if(itgrmu==globalmutmap.end())
 {

int count;
count=varcount;
varcount++;
struct globalregion tr;
tr.tid=lit->tid;
tr.type=" ";
 tr.mutex_var=0;
  tr.sync=" ";
tr.global_var=lit1->global_var;

struct globalinregion tr2;
tr2.tid=lit->tid;
tr2.type=" ";
 tr2.mutex_var=0;
  tr2.sync=" ";

struct cglobalinregion tr3;
tr3.tid=lit->tid;
tr3.count=0;
tr3.scount=0;
tr3.bcount=0;
//int sflag=0;
int flag=0;
int bflag=0;
int mflag=0;


for(itgrmu=globalmutmap.begin(); itgrmu!=globalmutmap.end(); ++itgrmu)
{
			
std::list<struct mutexregion>::iterator litgrmu;				
for(litgrmu=itgrmu->second.begin(); litgrmu!=itgrmu->second.end(); ++litgrmu)
{
 
if(litgrmu->tid==lit->tid && litgrmu->flag==1)
{
 tr.mutex_var=litgrmu->addr;
  tr.sync="MUTEX";

 tr2.mutex_var=litgrmu->addr;
  tr2.sync="MUTEX";

  tr3.scount=1;
mflag=1;
}


else if(litgrmu->tid==lit->tid && litgrmu->flag==9)
{
 tr.mutex_var=litgrmu->addr;
  tr.sync="BARRIER";
tr.type="AFTER";


 tr2.mutex_var=litgrmu->addr;
  tr2.sync="BARRIER";
tr2.type="AFTER";

tr3.bcount=1;
bflag=1;
  
}

}


}

std::list<struct globalregion> tlist;
tlist.push_back(tr);
globalmap.insert(std::pair<int,std::list<struct globalregion> >(count, tlist) );

itcgrin=cglobalinmap.find(lit1->global_var);

  if(itcgrin==cglobalinmap.end())
{
std::list<struct cglobalinregion> tlist3;
tlist3.push_back(tr3);
cglobalinmap.insert(std::pair<ADDRINT,std::list<struct cglobalinregion> >(lit1->global_var, tlist3) );


}
else
{
std::list<struct cglobalinregion> :: iterator litcgrin;
for(litcgrin=itcgrin->second.begin(); litcgrin!=itcgrin->second.end(); ++litcgrin)
{
if(litcgrin->tid==lit->tid)
{

if(mflag==1)

litcgrin->scount=(litcgrin->scount)+1;


else if(bflag==1)
litcgrin->bcount=(litcgrin->bcount)+1;

else
litcgrin->count=(litcgrin->count)+1;
flag=1;
}

}

if(flag==0)
itcgrin->second.push_back(tr3);

}




itgrin=globalinmap.find(lit1->global_var);
 if(itgrin==globalinmap.end())
{
std::list<struct globalinregion> tlist2;
tlist2.push_back(tr2);
globalinmap.insert(std::pair<ADDRINT,std::list<struct globalinregion> >(lit1->global_var, tlist2) );
}
else
{
itgrin->second.push_back(tr2);
}
 
}
else
{

 itgrcu=countmap.find(lit1->global_var);
if(itgrcu==countmap.end())
 {
   
 struct countmutexregion tr;
 tr.tid=lit->tid;
 tr.count=0;
std::list<struct countmutexregion> tlist;
tlist.push_back(tr);
countmap.insert(std::pair<ADDRINT,std::list<struct countmutexregion> >(lit1->global_var, tlist) );

}
else
{
int f=0;
 std::list<struct countmutexregion>::iterator litgrcu;

 for(litgrcu=itgrcu->second.begin(); litgrcu!=itgrcu->second.end(); ++litgrcu)
{
 if(litgrcu->tid==lit->tid)
{
  litgrcu->count=(litgrcu->count) +1;
  f=1;
}
}
if(f==0)
{
struct countmutexregion tr1;
 tr1.tid=lit->tid;
 tr1.count=0;
 itgrcu->second.push_back(tr1);
}
  
}

}
}





if(lit->order=="BEFORE" && lit->cont=="Reads from variable whose address is" and lit->node_type==0)



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';




if(lit->order=="BEFORE" && lit->node_type==9)



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="RUNS EXIT HANDLERS" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




if(lit->order=="BEFORE" && lit->cont=="Exits the Catch Block" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Starts Unwinding Stack for finding the exception handler" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Runs the personality functions for each routine" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Catches the Exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="Rethrows the exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="checks the current exception type, returns NULL if not handled" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
if(lit->order=="BEFORE" && lit->cont=="Calls the default terminate function" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Stores the arguments to be passed to next phase of exception hanlding which can be the cleanup phase or the last handler i.e landing pad" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Sets the instruction pointer" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Kills the Process" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Aborts the Process which is the action of the default terminate function" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Starts the search phase once more after exception is rethrown" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Unregisters fork handlers" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Completed Exception handling" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Detects Exception" and lit->node_type==0)


ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Throws Exception" and lit->node_type==0)



ExceptionFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="MALLOC" and lit->node_type==0)



//cout<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var <<"  "<<'\n';

if(1==1)
ExceptionFile<<endl;
}

				


	



//Population of Global Variables List and Local Variables List


for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

int flag=0;
THREADID td;

 //OutFile<<"Global Variable "<<itgrin->first<<"being accessed without any synchronisation by threads  " <<'\n';
			
std::list<struct globalinregion>::iterator littgrin1;
		
std::list<struct globalinregion>::iterator littgrin4;
littgrin4=itgrin->second.begin();
td=littgrin4->tid;
for(littgrin1=itgrin->second.begin(); littgrin1!=itgrin->second.end(); ++littgrin1)
{
if(td!=littgrin1->tid)
{
flag=1;
//cout<<"FLAG"<<flag;
}

 td=littgrin1->tid;
//cout<<" "<<littgrin1->tid<<"THREADS   ";

}
if(flag==1)

glist.push_back(itgrin->first);

if(flag==0)
{
std::list<THREADID> tlist;
tlist.push_back(littgrin4->tid);

lmap.insert(std::pair<ADDRINT,std::list<THREADID> >(itgrin->first, tlist) );

}


}


//Population of GlobalVars-Mutex and GlobalVars-Barrier maps


for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{


int mflag=0;
int bflag=0;
	
//int afbflag=0;
// OutFile<<"Global Variable "<<itgrin->first<<"being accessed without any synchronisation by threads  " <<'\n';
		
std::list<struct aglobalinregion> tlist1;
std::list<struct aglobalinregion> :: iterator limt;
std::list<struct bglobalinregion> tlist2;
std::list<struct bglobalinregion> :: iterator libt;
std::list<struct afbfglobalinregion> tlist3;
std::list<struct afbfglobalinregion> :: iterator lifft;

std::list<struct globalinregion>::iterator littgrin1;
		

for(littgrin1=itgrin->second.begin(); littgrin1!=itgrin->second.end(); ++littgrin1)
{
if(littgrin1->sync=="MUTEX")
{
 //mlist.push_back(littgrin1->mutex_var);

int e=0;
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 if((*itm)==littgrin1->mutex_var)
{
  e=1;
//cout<<"NEXT TIME";
}
}
if (e==0)
{
//cout<<"MUTEXESS"<<littgrin1->mutex_var<<"  ";
mlist.push_back(littgrin1->mutex_var);
}

int p=0;
for(limt=tlist1.begin();limt!=tlist1.end();++limt)
{
   if(limt->var==littgrin1->mutex_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(limt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      limt->count=(limt->count)+1;
      limt->tid[limt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
mflag=1;
struct aglobalinregion tr1;
tr1.var=littgrin1->mutex_var;
tr1.count=0;
tr1.tid[tr1.count]=littgrin1->tid;
tlist1.push_back(tr1);
mflag=1;

}

}




if(littgrin1->sync=="BARRIER" and littgrin1->type=="AFTER")
{

int p=0;
for(lifft=tlist3.begin();lifft!=tlist3.end();++lifft)
{
   if(lifft->var==littgrin1->mutex_var and lifft->occ=="AFTER")
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(lifft->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{    
  lifft->count=(lifft->count)+1;
      lifft->tid[lifft->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct afbfglobalinregion tr3;
tr3.var=littgrin1->mutex_var;
tr3.count=0;
tr3.tid[tr3.count]=littgrin1->tid;
tr3.occ="AFTER";
tlist3.push_back(tr3);
//afbflag=1;

}
}





if(littgrin1->sync=="BARRIER" and littgrin1->type=="BEFORE")
{

int p=0;
for(lifft=tlist3.begin();lifft!=tlist3.end();++lifft)
{
   if(lifft->var==littgrin1->mutex_var and lifft->occ=="BEFORE")
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(lifft->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      lifft->count=(lifft->count)+1;
      lifft->tid[lifft->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct afbfglobalinregion tr3;
tr3.var=littgrin1->mutex_var;
tr3.count=0;
tr3.tid[tr3.count]=littgrin1->tid;
tr3.occ="BEFORE";
tlist3.push_back(tr3);
//afbflag=1;
}

}



if(littgrin1->sync=="BARRIER" )
{

int e=0;
for(itb=blist.begin();itb!=blist.end();++itb)
{
 if(*itb==littgrin1->mutex_var)
  e=1;
}


if (e==0)
blist.push_back(littgrin1->mutex_var);

int p=0;
for(libt=tlist2.begin();libt!=tlist2.end();++libt)
{
   if(libt->var==littgrin1->mutex_var)
   {
     int i;
int k=0;
p=1;
     for(i=0;i<50;i++)

       {
         if(libt->tid[i]==littgrin1->tid)
         
          k=1;
        }
      if(k==0)
{
      libt->count=(libt->count)+1;
      libt->tid[libt->count]=littgrin1->tid;
}

    }

}
if(p==0)
{
struct bglobalinregion tr2;
tr2.var=littgrin1->mutex_var;
tr2.count=0;
tr2.tid[tr2.count]=littgrin1->tid;
tlist2.push_back(tr2);
bflag=1;
}

}







}


if(mflag==1)
mutexglobalinmap.insert(std::pair<ADDRINT,std::list<struct aglobalinregion> >(itgrin->first, tlist1) );

if(bflag==1)
{
barrierglobalinmap.insert(std::pair<ADDRINT,std::list<struct bglobalinregion> >(itgrin->first, tlist2) );

afbfbarrierglobalinmap.insert(std::pair<ADDRINT,std::list<struct afbfglobalinregion> >(itgrin->first, tlist3) );
}



}



	if (!EnableModelDetection)
{

OutFile<<"The Start Thread is :"<<strthread<<'\n'<<'\n';

OutFile<<"The Main Thread is :"<<mainthread<<'\n'<<'\n';

OutFile <<"The Create Sequence :"<<'\n'<<'\n';
ModelMapFile<<"digraph G {"<<'\n';

	

for(itcrsq=createseqmap.begin(); itcrsq!=createseqmap.end(); ++itcrsq)
{

 OutFile<<" "<<itcrsq->first<<" : ";				
std::list<struct createregion>::iterator litcrsq;
				
for(litcrsq=itcrsq->second.begin(); litcrsq!=itcrsq->second.end(); ++litcrsq)
{

OutFile<<"Thread "<<litcrsq->tid <<" Creates "<<"Thread "<<litcrsq->tid1<<'\n';
				
}
			
}

OutFile <<'\n'<<"The Create Summary:"<<'\n'<<'\n';

		ModelMapFile<<"node [style=filled,color=blue];"<<'\n';
           
            ModelMapFile<<"edge[label=creates]"<<'\n';


		

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{

 OutFile<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';				
std::list<THREADID>::iterator litcr;
				
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{

OutFile<<"Thread "<<*litcr<<'\n';
ModelMapFile<<"T"<<itcr->first<<"->"<<"T"<<*litcr<<";"<<'\n';	
		
}	


}


if(dcount>0)
{

OutFile<<'\n'<<"The communication sequence among threads using condition variables "<<'\n'<<'\n';



		ModelMapFile<<"node [style=filled,color=red];"<<'\n';
           
        


for(itsrsq=signalseqmap.begin(); itsrsq!=signalseqmap.end(); ++itsrsq)
{

 OutFile<<" "<<itsrsq->first<<" : " <<'\n';
				
std::list<struct signalregion>::iterator litsrsq;
				
for(litsrsq=itsrsq->second.begin(); litsrsq!=itsrsq->second.end(); ++litsrsq)
{

if(litsrsq->type=="WAITS")
 OutFile<<"Thread "<<litsrsq->tid <<"Waits over condition variable "<<litsrsq->cond_var<<"  protected by mutex "<<litsrsq->mutex_var<<'\n';
else
OutFile<<"Thread "<<litsrsq->tid <<"Signals Thread "<<litsrsq->tid1<<" "<<'\n';			
}
			
}

OutFile<<'\n'<<"The communication summary among threads using condition variables "<<'\n';


for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)
{

 OutFile<<"Thread"<<"    "<<itcc->first<<"is associated with condition variable ";				
std::list<ADDRINT>::iterator litcc;
				
for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

OutFile<<"  "<<*litcc<<'\n';
ModelMapFile<<"C"<<*litcc<<";"<<'\n';

				
}
			
}



for(itsr=signalmap.begin(); itsr!=signalmap.end(); ++itsr)
{

 OutFile<<"Thread"<<" "<<itsr->first<<"signals"<<'\n';				
std::list<THREADID>::iterator litsr;
				
for(litsr=itsr->second.begin(); litsr!=itsr->second.end(); ++litsr)
{

OutFile<<"Thread "<<*litsr<<'\n';
				
}
			
}

}

int flag=0;
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

flag++;

}
if(flag==0)
OutFile<<'\n'<<"There are no global variables in this program hence no communication via global variables"<<'\n';


if(flag>0)
{


OutFile<<'\n'<<"The communication summary among threads using Global variables "<<'\n'<<'\n';



ModelMapFile<<"style=filled;"<<'\n';
	
		ModelMapFile<<"node [style=filled,color=green];"<<'\n';
          
            


for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

if(*itg==itcgrin->first)
{

 OutFile<<"Global Variable "<<itcgrin->first<<"being accessed by   " <<'\n';


	ModelMapFile<<"D"<<itcgrin->first<<";"<<'\n';			
std::list<struct cglobalinregion>::iterator littcgrin1;
	int flag =0;			
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{
 
if(littcgrin1->count>0)
{
OutFile<<" "<<littcgrin1->tid<<" "<<"without synchronisation"<<'\n';
flag=1;
}
}
if(flag==0)
OutFile<< "No threads without Synchronisation "<<'\n';
}

}
}





		ModelMapFile<<"node [style=filled,color=yellow];"<<'\n';
            
          
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelMapFile<<"M"<<*itm<<";"<<'\n';
}




		ModelMapFile<<"node [style=filled,color=orange];"<<'\n';
            
          
for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelMapFile<<"B"<<*itb<<";"<<'\n';
}

//mutex
for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itmgrin=mutexglobalinmap.begin(); itmgrin!=mutexglobalinmap.end(); ++itmgrin)
{

if(*itg==itmgrin->first)
{

std::list<struct aglobalinregion>::iterator littm;
OutFile<<'\n'<<"Global Variable "<<itmgrin->first<<" accessed with  mutex	";			
for(littm=itmgrin->second.begin(); littm!=itmgrin->second.end(); ++littm)
{

OutFile<<littm->var<<"  by threads"<<'\n';
int i=0;
for(i=0;i<=littm->count;i++)
{
OutFile<<"Thread "<<littm->tid[i]<<'\n';
ModelMapFile<<"edge[label=locks];"<<'\n';
ModelMapFile<<"T"<<littm->tid[i]<<"->"<<"M"<<littm->var<<";"<<'\n';
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"M"<<littm->var<<"->"<<"D"<<itmgrin->first<<";"<<'\n';

}

}

}
}

}
//var
for(itg=glist.begin();itg!=glist.end();++itg)
{

for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{

if(*itg==itcgrin->first)
{

 		
std::list<struct cglobalinregion>::iterator littcgrin1;
				
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{
 
if(littcgrin1->count>0)
{
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"T"<<littcgrin1->tid<<"->"<<"D"<<itcgrin->first<<";"<<'\n';
}
}
}

}
}

//barrier


for(itg=glist.begin();itg!=glist.end();++itg)
{


for(itafbfgrin=afbfbarrierglobalinmap.begin(); itafbfgrin!=afbfbarrierglobalinmap.end(); ++itafbfgrin)
{

if(*itg==itafbfgrin->first)
{

std::list<struct afbfglobalinregion>::iterator lifftt;
OutFile<<'\n'<<"Global Variable accessed with  memory barriers	";			
for(lifftt=itafbfgrin->second.begin(); lifftt!=itafbfgrin->second.end(); ++lifftt)
{

OutFile<<lifftt->var<<"  by threads"<<"  "<<lifftt->occ<<"  its occurance "<<'\n';
int i=0;
for(i=0;i<=lifftt->count;i++)
{
OutFile<<"Thread "<<lifftt->tid[i]<<'\n';
if(lifftt->occ=="AFTER")
{
ModelMapFile<<"edge[label=barrier];"<<'\n';
ModelMapFile<<"T"<<lifftt->tid[i]<<"->"<<"B"<<lifftt->var<<";"<<'\n';
ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"B"<<lifftt->var<<"->"<<"D"<<itafbfgrin->first<<";"<<'\n';
}
if(lifftt->occ=="BEFORE")
{

ModelMapFile<<"edge[label=accesses];"<<'\n';
ModelMapFile<<"T"<<lifftt->tid[i]<<"->"<<"D"<<itafbfgrin->first<<";"<<'\n';

ModelMapFile<<"edge[label=barrier];"<<'\n';
ModelMapFile<<"D"<<itafbfgrin->first<<"->"<<"B"<<lifftt->var<<";"<<'\n';
}

}

}

}
}

}

}



for(itsr=signalmap.begin(); itsr!=signalmap.end(); ++itsr)
{

 //OutFile<<"Thread"<<" "<<itsr->first<<"signals"<<'\n';				
std::list<THREADID>::iterator litsr;
				
for(litsr=itsr->second.begin(); litsr!=itsr->second.end(); ++litsr)
{
ModelMapFile<<"edge[label=signals];"<<'\n';

ModelMapFile<<"T"<<itsr->first<<"->"<<"T"<<*litsr<<";"<<'\n';
//OutFile<<"Thread "<<*litsr<<'\n';
				
}
			
}

for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)

{

// OutFile<<"Thread"<<"    "<<itcc->first<<"is associated with condition variable ";				
std::list<ADDRINT>::iterator litcc;
				
for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

//OutFile<<"  "<<*litcc<<'\n';
ModelMapFile<<"edge[label=waits];"<<'\n';
ModelMapFile<<"T"<<itcc->first<<"->"<<"C"<<*litcc<<";"<<'\n';
				
}
			
}







ModelMapFile<<"}"<<endl<<'\n';
OutFile <<'\n'<<"The Exit Sequence:"<<'\n'<<'\n';

for(itex=exitmap.begin(); itex!=exitmap.end(); ++itex)
{

 OutFile<<" "<<itex->first<<" : ";				
std::list<struct exitregion>::iterator litex;
				
for(litex=itex->second.begin(); litex!=itex->second.end(); ++litex)
{

OutFile<<"Thread :"<<litex->tid <<"   "<<litex->type<<'\n'<<endl;
				
}
			
}








}






if (!EnableDataDetection)
{


ModelDataFile<<"digraph G {"<<'\n';	


DataFile<<" Data  and their Access History "<<'\n'<<'\n';

int gcount=0;
int scount=0;
int sflag=0;
//int icount=1;
int flag=0;
int lfag=0;

    for(itl=lmap.begin();itl!=lmap.end();++itl)
{
 lfag++;
}
if(lfag>0)
{

DataFile<<'\n'<<" Local variables Access History"<<'\n';
for(itth=thlist.begin();itth!=thlist.end();++itth)
{
    DataFile<<"Thread "<<*itth<<" Uses following local variables "<<'\n';
int flag = 0;
   for(itl=lmap.begin();itl!=lmap.end();++itl)
{
std::list< THREADID>::iterator litl;

for(litl=itl->second.begin();litl!=itl->second.end();++litl)
{
if(*litl==*itth)
{
flag=1;
 DataFile<<itl->first<<'\n';
 }
}

}
if(flag==0)
DataFile<<"No Local variables";
}
}

for(itg=glist.begin();itg!=glist.end();++itg)
{ 

flag++;

}
if(flag==0)
OutFile<<'\n'<<"There are no global variables in this program "<<'\n';


if(flag>0)
{
DataFile <<'\n'<<" Total List of Global Variables in this program are "<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{
if(*itg==itgrin->first)
{
 DataFile<<itgrin->first<<'\n';
	gcount++;
}
}
}			
DataFile<<'\n'<<" Total Count is : "<<gcount<<'\n'<<'\n'<<'\n';


DataFile <<" Total List of Global Variables in this program which are used among threads with synchronisation"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{ 

for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{

 if(*itg==itgrin->first)
{
	
std::list<struct globalinregion>::iterator littgrin2;
				
for(littgrin2=itgrin->second.begin(); littgrin2!=itgrin->second.end(); ++littgrin2)
{
 if(littgrin2->sync=="MUTEX" || littgrin2->sync=="BARRIER")
 sflag=1;
				
}
if(sflag==1)
{
DataFile<<itgrin->first<<'\n';
scount++;
}

}
}
}			
DataFile<<'\n'<<" Total Count is : "<<scount<<'\n'<<'\n';

DataFile<<" The sequence and mode of access of each global variable w.r.t to the active threads in the system "<<'\n'<<'\n';

int f=0;

for(itg=glist.begin();itg!=glist.end();++itg)
{ 
int icount=0;
for(itgrin=globalinmap.begin(); itgrin!=globalinmap.end(); ++itgrin)
{
if(*itg==itgrin->first)
{
f++;

icount++;
// DataFile<<icount<< " : "<<"Global Variable "<<itgrin->first<<" Access Sequence Order "<<'\n'<<'\n';

DataFile<<"Global Variable "<<itgrin->first<<" Access Sequence Order "<<'\n'<<'\n';
	int i=1;

ModelDataFile<<"subgraph cluster_"<<f<<" {"<<'\n';
 ModelDataFile<<"label="<<"D"<<itgrin->first<<'\n';
std::list<struct globalinregion>::iterator littgrin3;
				
for(littgrin3=itgrin->second.begin(); littgrin3!=itgrin->second.end(); ++littgrin3)
{
 if(littgrin3->sync=="MUTEX")
{


ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"M"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';



ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"M"<<littgrin3->mutex_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" with the help of mutex " <<littgrin3->mutex_var <<'\n'<<'\n';
i++;

}

if(littgrin3->sync==" ")
{

ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" without any synchronisation"<<'\n'<<'\n';
i++;
}
if(littgrin3->sync=="BARRIER")
{
 DataFile<<'\n'<<" is accessed by thread "<<littgrin3->tid <<" "<<littgrin3->type<<" the occurance with the help of barrier " <<littgrin3->mutex_var <<'\n'<<'\n';
if(littgrin3->type=="AFTER")
{
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
}

if(littgrin3->type=="BEFORE")
{
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataFile<<"T"<<littgrin3->tid<<"_"<<f<<"->"<<"B"<<littgrin3->mutex_var<<"_"<<f<<";"<<'\n';
ModelDataFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataFile<<"B"<<littgrin3->mutex_var<<"_"<<f<<"->"<<"D"<<itgrin->first<<"_"<<f<<";"<<'\n';
}
i++;
}
				
}

ModelDataFile<<"}"<<'\n';
}
}			
}

}

ModelDataFile<<"}"<<endl;
DataFile<<endl;

}




if (!EnableDataSeqDetection)
{
DataseqFile<<" The Sequence of Data Access of all the global variables in the system "<<'\n'<<'\n';
ModelDataSeqFile<<"digraph G {"<<'\n';	

ModelDataSeqFile<<"node [style=filled,color=blue];"<<'\n';

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{ 
//OutFile<<"Thread"<<" "<<itcr->first<<" creates "<<'\n';		
std::list<THREADID>::iterator litcr;		
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{
//OutFile<<"Thread "<<*litcr<<'\n';
ModelDataSeqFile<<"T"<<itcr->first<<"->"<<"T"<<*litcr<<";"<<'\n';

}	
}




  ModelDataSeqFile<<"node [style=filled,color=green];"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{
for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{
if(*itg==itcgrin->first)
{ 
//OutFile<<"Global Variable "<<itcgrin->first<<"being accessed by   " <<'\n';
ModelDataSeqFile<<"D"<<itcgrin->first<<";"<<'\n';			
std::list<struct cglobalinregion>::iterator littcgrin1;	
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{ 
//if(littcgrin1->count>0)
//OutFile<<" "<<littcgrin1->tid<<" "<<"without synchronisation"<<'\n';
}
}
}
}



ModelDataSeqFile<<"node [style=filled,color=yellow];"<<'\n';
          for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelDataSeqFile<<"M"<<*itm<<";"<<'\n';
}

		
ModelDataSeqFile<<"node [style=filled,color=orange];"<<'\n';     
     for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelDataSeqFile<<"B"<<*itb<<";"<<'\n';
}
int i=1;
for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

 DataseqFile<<itgr->first<<" : ";
				
std::list<struct globalregion>::iterator littgr1;
				
for(littgr1=itgr->second.begin(); littgr1!=itgr->second.end(); ++littgr1)
{
 
 
if(littgr1->sync=="MUTEX")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"with the help of mutex "<< littgr1->mutex_var<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';

ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"M"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';

ModelDataSeqFile<<"M"<<littgr1->mutex_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync==" ")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid <<"without synchronisation"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->sync=="BARRIER")
{
 DataseqFile<<"Global Variable " <<littgr1->global_var<<"accessed by thread "<<littgr1->tid << " "
<<littgr1->type<<" the occurance of memory barrier "<< littgr1->mutex_var<<'\n';
if(littgr1->type=="AFTER")
{
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataSeqFile<<"D"<<littgr1->global_var<<"->"<<"B"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

if(littgr1->type=="BEFORE")
{
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelDataSeqFile<<"T"<<littgr1->tid<<"->"<<"B"<<littgr1->mutex_var<<";"<<'\n';
ModelDataSeqFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelDataSeqFile<<"B"<<littgr1->mutex_var<<"->"<<"D"<<littgr1->global_var<<";"<<'\n';
}

}
				
}
i++;			
}

DataseqFile<<'\n'<<'\n'<<" The waiting time of each thread to acquire a mutex "<<'\n'<<'\n';
ModelDataSeqFile<<"}"<<'\n'<<endl;	

for(itgrcu=countmap.begin(); itgrcu!=countmap.end(); ++itgrcu)

{
int mcount=0;
int lowest=0;
THREADID td;
 DataseqFile<<"Mutex "<<" " <<itgrcu->first<<"has been acquired by "<<'\n';			
std::list<struct countmutexregion>::iterator litttgrcu;

			
for(litttgrcu=itgrcu->second.begin(); litttgrcu!=itgrcu->second.end(); ++litttgrcu)
{


DataseqFile<< "Thread "<<litttgrcu->tid<<"after "<< litttgrcu->count <<" trials "<<'\n';


mcount=litttgrcu->count;

if(lowest==0)
{
lowest=mcount;
td=litttgrcu->tid;
}
else
{
if(mcount<lowest)
{
lowest=mcount;
td=litttgrcu->tid;
}
}


}

DataseqFile<<" This mutex was first acquired by "<<td <<" After "<< lowest <<"trials "<<'\n'<<'\n'; 

}


if(1==1)
DataseqFile<<endl;
}




	if (!EnableThreadDetection)
{

ThreadFile<<" Thread Summary "<<'\n'<<'\n';

ThreadFile<<" The Sequence of Global Variable Access per thread is given below "<<'\n'<<'\n';

ModelThreadFile<<"digraph G {"<<'\n';

int f=0;
for (itth=thlist.begin(); itth!=thlist.end(); ++itth)
{
  ThreadFile<<" Thread " <<*itth <<'\n';
f++;
int i=1;
ModelThreadFile<<"subgraph cluster_"<<f<<" {"<<'\n';
 ModelThreadFile<<"label="<<"T"<<*itth<<'\n';

for(itgr=globalmap.begin(); itgr!=globalmap.end(); ++itgr)
{

				
std::list<struct globalregion>::iterator littgr2;
				
for(littgr2=itgr->second.begin(); littgr2!=itgr->second.end(); ++littgr2)
{


if(littgr2->tid==*itth && littgr2->sync=="MUTEX")
{

for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses Global Variable "<<littgr2->global_var<<" with mutex "<<littgr2->mutex_var <<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"M"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';

ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"M"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DG"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
 
i++;
}
}

for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{
ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<" with mutex "<<littgr2->mutex_var <<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"M"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';

ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"M"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
 
i++;
}
}
}


if(littgr2->tid== *itth && littgr2->sync==" ")
{
for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{

ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<"  "<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
i++;
}
}

for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses global Variable "<<littgr2->global_var<<" without synchronisation"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"acceses]"<<";"<<'\n';

ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DG"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
i++;
}
}
}


if(littgr2->tid== *itth && littgr2->sync=="BARRIER")
{

for(itl=lmap.begin();itl!=lmap.end();++itl)
{ 
if(littgr2->global_var==itl->first)
{
ThreadFile<<i<<" : "<<"Accesses global Variable "<<littgr2->global_var<<"  "<<littgr2->type <<" the occurance of memory barrier "<<littgr2->mutex_var<<"   "<<'\n';
if(littgr2->type=="AFTER")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
if(littgr2->type=="BEFORE")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"B"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
i++;
}
}
for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(littgr2->global_var==*itg)
{
ThreadFile<<i<<" : "<<"Accesses local Variable "<<littgr2->global_var<<"  "<<littgr2->type <<" the occurance of memory barrier "<<littgr2->mutex_var<<"   "<<'\n';
if(littgr2->type=="AFTER")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"D"<<itgrin->first<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
if(littgr2->type=="BEFORE")
{
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"barrier]"<<";"<<'\n';
ModelThreadFile<<"T"<<littgr2->tid<<"_"<<f<<"->"<<"B"<<littgr2->mutex_var<<"_"<<f<<";"<<'\n';
ModelThreadFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelThreadFile<<"B"<<littgr2->mutex_var<<"_"<<f<<"->"<<"DL"<<littgr2->global_var<<"_"<<f<<";"<<'\n';
}
i++;
}
}

}



}
}
ModelThreadFile<<"}";
}

ModelThreadFile<<"}";
ModelThreadFile<<endl;

ThreadFile<<'\n'<<'\n'<<"Thread Statistics"<<'\n'<<'\n';
for (itth=thlist.begin(); itth!=thlist.end(); ++itth)
    {
        thread_data_tsu* tdata = get_tls(*itth);


    ThreadFile<< '\n'<<" Thread   "<<*itth<<'\n';

    ThreadFile<< '\n'<<" OS Thread id "<<tdata->t<<'\n';
   // ThreadFile<<"  The number of routines executed by the thread (only required routines for capturing execution snapshot) are  "<< tdata->_rtncount<<'\n';
    ThreadFile<<" The Routines are(apart from read or write)"<<tdata->rtnlist<<'\n';
   
   ThreadFile<<" This thread was created by  thread "<<tdata->oid<<'\n';
    ThreadFile<<" The number of Memory Reads by this thread are  " <<tdata->_rdcount<<'\n';
    ThreadFile<<" The number of Memory Writes by this thread are  " <<tdata->_wrcount<<'\n';
    ThreadFile<<" The number of child threads created by this thread are  "<<tdata->_fcount<<'\n'<< '\n'<<'\n'; 
         
    }

ThreadFile<<endl;

}
			
if (!EnableMutexDetection)

{
ModelMutexFile<<"digraph G {"<<'\n';

MutexFile<<"Mutex Lock UnLock Sequence "<<'\n'<<'\n';
for(itggrmu=gglobalmutmap.begin(); itggrmu!=gglobalmutmap.end(); ++itggrmu)
{

 				
std::list<struct gmutexregion>::iterator littggrmu;
	MutexFile<<itggrmu->first<<" : ";			
for(littggrmu=itggrmu->second.begin(); littggrmu!=itggrmu->second.end(); ++littggrmu)

{

MutexFile<<"Thread "<<littggrmu->tid <<" "<<littggrmu->occ<<" mutex "<<littggrmu->addr<<'\n';
if(littggrmu->occ=="LOCKS")
{
ModelMutexFile<<"edge[label="<<"E"<<itggrmu->first<<"_"<<"locks]"<<";"<<'\n';
ModelMutexFile<<"T"<<littggrmu->tid<<"->"<<"M"<<littggrmu->addr<<";"<<'\n';

}

if(littggrmu->occ=="UNLOCKS")
{
ModelMutexFile<<"edge[label="<<"E"<<itggrmu->first<<"_"<<"unlocks]"<<";"<<'\n';
ModelMutexFile<<"M"<<littggrmu->addr<<"->"<<"T"<<littggrmu->tid<<";"<<'\n';
}

}

}


ModelMutexFile<<"}"<<'\n'<<endl;
MutexFile<<endl;

}

	if (!EnableExecutionDetection)
{
ModelExecFile<<"digraph G {"<<'\n';
ModelExecFile<<"node [style=filled,color=blue];"<<'\n';

for(itcr=createmap.begin(); itcr!=createmap.end(); ++itcr)
{ 
std::list<THREADID>::iterator litcr;		
for(litcr=itcr->second.begin(); litcr!=itcr->second.end(); ++litcr)
{
ModelExecFile<<"T"<<itcr->first<<";"<<'\n';
ModelExecFile<<"T"<<*litcr<<";"<<'\n';

}	
}

ModelExecFile<<"node [style=filled,color=red];"<<'\n'; 
for(itcc=condmap.begin(); itcc!=condmap.end(); ++itcc)
{ 
				
std::list<ADDRINT>::iterator litcc;		

for(litcc=itcc->second.begin(); litcc!=itcc->second.end(); ++litcc)
{

ModelExecFile<<"C"<<*litcc<<";"<<'\n';			
	}	
}

  ModelExecFile<<"node [style=filled,color=green];"<<'\n';
for(itg=glist.begin();itg!=glist.end();++itg)
{
for(itcgrin=cglobalinmap.begin(); itcgrin!=cglobalinmap.end(); ++itcgrin)
{
if(*itg==itcgrin->first)
{ 

ModelExecFile<<"D"<<itcgrin->first<<";"<<'\n';		
	std::list<struct cglobalinregion>::iterator littcgrin1;	
for(littcgrin1=itcgrin->second.begin(); littcgrin1!=itcgrin->second.end(); ++littcgrin1)
{ 

}
}
}
}

ModelExecFile<<"node [style=filled,color=yellow];"<<'\n';
for(itm=mlist.begin();itm!=mlist.end();++itm)
{
 ModelExecFile<<"M"<<*itm<<";"<<'\n';
}

		
ModelExecFile<<"node [style=filled,color=orange];"<<'\n';        
  for(itb=blist.begin();itb!=blist.end();++itb)
{
 ModelExecFile<<"B"<<*itb<<";"<<'\n';
}







int i=1;
ADDRINT mut=0;
THREADID td=0;
//ADDRINT bar;
//THREADID 

for(it2=accessmap.begin(); it2!=accessmap.end(); ++it2)
{

 ExecFile<<'\n'<<"Event"<<" "<<it2->first<<" : ";				
std::list<struct threadregion>::iterator lit;
std::list<struct threadregion>::reverse_iterator lit1;
lit=it2->second.begin();
lit1=it2->second.rbegin();
//ExecFile<<lit->cont;
//ExecFile<<lit1->cont;
if(lit->order=="AFTER" && lit->cont=="STARTS" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="STARTS" and lit->node_type==6)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Requests for locking the mutex whose address is" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';



if(lit->order=="BEFORE" && lit->cont=="Acquires the lock for mutex whose address is" and lit->node_type==0)

{
//for(itm=mlist.begin();itm!=mlist.end();++itm)
//{ 
//if(lit1->mutex_var==*itm)
//{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
//}
//}
td=lit->tid;
mut=lit1->mutex_var;

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';

}

if(lit->order=="BEFORE" && lit->cont=="Executes Main" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';
 



if(lit->order=="BEFORE" && lit->cont=="Wakes up waiting threads" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Waits for acquiring lock" and lit->node_type==0)

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="JOINS" and lit->node_type==0)



ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




if(lit->order=="BEFORE" && lit->cont=="Unlocks the mutex variable whose address is" and lit->node_type==0)
{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"unlocks]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<lit1->mutex_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->mutex_var <<"  "<<'\n';
}
if(lit->order=="BEFORE" && lit->cont=="CREATES" and lit->node_type==0)

{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"creates]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"T"<<lit1->tid<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';

}

if(lit->order=="BEFORE" && lit->cont=="Waits over condition variable" and lit->node_type==0)
{

ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"waits]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"C"<<lit1->cond_var<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->cond_var <<"  "<<'\n';

}


if(lit->order=="BEFORE" && lit->cont=="Signals" and lit->node_type==0)
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"signals]"<<";"<<'\n';
i++;
ModelExecFile<<"T"<<lit->tid<<"->"<<"T"<<lit1->tid<<";"<<'\n';
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"Thread "<<lit1->tid <<"  "<<'\n';
}


if(lit->order=="BEFORE" && lit->cont=="EXITS" and lit->node_type==0)


ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Writes to variable whose address is" and lit->node_type==0)

{





for(itg=glist.begin();itg!=glist.end();++itg)
{ 
if(lit1->global_var==*itg)
{
i++;
if(lit->tid==td)
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"locks]"<<";"<<'\n';
ModelExecFile<<"T"<<lit->tid<<"->"<<"M"<<mut<<";"<<'\n';
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelExecFile<<"M"<<mut<<"->"<<"D"<<lit1->global_var<<";"<<'\n';
td=99;
}
else
{
ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"accesses]"<<";"<<'\n';
ModelExecFile<<"T"<<lit->tid<<"->"<<"D"<<lit1->global_var<<";"<<'\n';
}
}
}




ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';


}


if(lit->order=="BEFORE" && lit->cont=="Reads from variable whose address is" and lit->node_type==0)


{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"reads]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"D"<<lit1->global_var<<";"<<'\n';

ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';

}


if(lit->order=="BEFORE" && lit->node_type==9)



ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="RUNS EXIT HANDLERS" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';




//if(lit->order=="BEFORE" && lit->cont=="Exits the Catch Block" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="Catches the Exception" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


if(lit->order=="BEFORE" && lit->cont=="Unregisters fork handlers" and lit->node_type==0)


ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



//if(lit->order=="BEFORE" && lit->cont=="Completed Exception handling" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';



//if(lit->order=="BEFORE" && lit->cont=="Detects Exception" and lit->node_type==0)


//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<"  "<<'\n';


//if(lit->order=="BEFORE" && lit->cont=="Throws Exception" and lit->node_type==0)



//ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit1->global_var <<"  "<<'\n';

if(lit->order=="BEFORE" && lit->cont=="Puts Memory bar" and lit->node_type==0)


{
//for(itb=blist.begin();itb!=blist.end();++itb)
//{ 
//if(lit1->mutex_var==*itb)
//{
//ModelExecFile<<"edge[label="<<"E"<<i<<"_"<<"barriers]"<<";"<<'\n';
//i++;
//ModelExecFile<<"T"<<lit->tid<<"->"<<"B"<<lit->global_var<<";"<<'\n';
//}
//}
ExecFile<<"Thread "<<lit->tid <<" "<<lit->cont <<" "<<lit->global_var<<'\n';
}


}

if(1==1)
{
ExecFile<<'\n'<<endl;
ModelExecFile<<"}"<<'\n'<<endl;
}
}





}
INT32 Usage()
{
    PIN_ERROR("This Pintool prints the IPs of every instruction executed\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if(PIN_Init(argc,argv))
    {
        return Usage();
    }

   // const string fileName = KnobOutputFile.Value();

   // if (!fileName.empty())
    //{
    //    out = new std::ofstream(fileName.c_str());
   // }
 InitLock(&lock);
 OutFile.open(KnobOutputFile.Value().c_str());

 
MutexFile.open(KnobMutexOutputFile.Value().c_str());
 
ExceptionFile.open(KnobExceptionOutputFile.Value().c_str());
ModelMutexFile.open(KnobModelMutexOutputFile.Value().c_str());
	ExecFile.open(KnobExecutionOutputFile.Value().c_str());
DataFile.open(KnobDataOutputFile.Value().c_str());
ModelDataFile.open(KnobModelDataOutputFile.Value().c_str());
DataseqFile.open(KnobDataSeqOutputFile.Value().c_str());
ModelMapFile.open(KnobModelMapOutputFile.Value().c_str());

ModelThreadFile.open(KnobModelThreadOutputFile.Value().c_str());
ThreadFile.open(KnobThreadOutputFile.Value().c_str());
ModelDataSeqFile.open(KnobModelDataSeqOutputFile.Value().c_str());


	ModelExecFile.open(KnobModelExecutionOutputFile.Value().c_str());
    // Obtain  a key for TLS storage.
   tls_key = PIN_CreateThreadDataKey(0);
    // Register Routine to be called to instrument rtn

	/*RegSkipNextR = PIN_ClaimToolRegister();
	if (!REG_valid(RegSkipNextR))
	{
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
	}
	
	RegSkipNextW = PIN_ClaimToolRegister();
	if (!REG_valid(RegSkipNextW))
	{
	std::cerr << "Not enough virtual registers" << std::endl;
	return 1;
	}*/

    PIN_AddDebugInterpreter(DebugInterpreter, 0);
 IMG_AddInstrumentFunction(Image, 0);
  INS_AddInstrumentFunction(Instruction, 0);
    RTN_AddInstrumentFunction(Routine, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, NULL);
   


   PIN_AddThreadStartFunction(ThreadStart, 0);
  PIN_AddThreadFiniFunction(ThreadFini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;

}



