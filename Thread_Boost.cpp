

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include "pin.H"

ofstream outFile;


typedef struct RtnCount
{
    string _name;
    string _image;
    UINT64 _rtnCount;
    struct RtnCount * _next;
} RTN_COUNT;


RTN_COUNT * RtnList = 0;

// This function is called before every instruction is executed

VOID docount(UINT64 * counter)
{
    (*counter)++;
}
    
const char * StripPath(const char * path)
{
    const char * file = strrchr(path,'/');
    if (file)
        return file+1;
    else
        return path;
}  

// Pin calls this function every time a new rtn is executed
VOID Routine(RTN rtn, VOID *v)
{
    
    // Allocate a counter for this routine
    RTN_COUNT * rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name = RTN_Name(rtn);

     if((RTN_Name(rtn)=="_ZN5boost5mutex4lockEv" && strcmp(StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()),"libboost_thread.so.1.56.0")!=0)||(rc->_name=="_ZN5boost5mutex6unlockEv" && strcmp(StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str()),"libboost_thread.so.1.56.0")!=0)||rc->_name=="_ZN5boost6thread4joinEv"){
          std::cout<<"Lock Detected"<<endl;
     

    rc->_image = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    rc->_rtnCount = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList = rc;
            
    RTN_Open(rtn);
            
    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_rtnCount), IARG_END);
    
    RTN_Close(rtn); 
}
}

// This function is called when the application exits
// It prints the name and count for each procedure

VOID Fini(INT32 code, VOID *v)
{
    outFile << setw(30) << "Procedure" << setw(30)<< "Image" << setw(13) <<"Calls"<< endl;

    for (RTN_COUNT * rc = RtnList; rc; rc = rc->_next)
    {
      //  if (rc->_icount > 0)
        
	
	    outFile << setw(30) << rc->_name << setw(30)<< rc->_image << setw(13) << rc->_rtnCount << endl;
        
    }  
                  
      
}


int main(int argc, char * argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    outFile.open("proc_name_detect.out");

    // Initialize pin
    PIN_Init(argc, argv);
 
    // Register Routine to be called to instrument rtn
    RTN_AddInstrumentFunction(Routine, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
