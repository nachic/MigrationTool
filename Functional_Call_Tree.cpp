#include "pin.H"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

std::map<std::string, UINT64> functionCount;
std::vector<std::string> callSequence;
std::ofstream outFile;

// This function will be called before each function is executed
void recordFunctionCall(ADDRINT address, const std::string &name) {
    // Increment the execution count for this function
    functionCount[name]++;
    callSequence.push_back(name);
}

// This function is called for each function found
void imageLoad(IMG img, VOID *v) {
    // Iterate over all functions in the loaded image
    for (RTN rtn = RTN_Before(img); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
        // Get function name
        const std::string name = RTN_Name(rtn);
        if (!name.empty()) {
            // Instrument the function to record its execution
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)recordFunctionCall, 
                           IARG_ADDRINT, RTN_Address(rtn),
                           IARG_ADDRINT, name.c_str(),
                           IARG_END);
            RTN_Close(rtn);
        }
    }
}

// This function is called when the program exits
void fini(INT32 code, VOID *v) {
    // Open the JSON output file
    outFile.open("control_graph.json");
    
    // Write the JSON structure
    outFile << "{\n";
    outFile << "  \"functions\": {\n";
    
    // Output the function counts
    for (const auto &func : functionCount) {
        outFile << "    \"" << func.first << "\": " << func.second << ",\n";
    }
    
    // Remove the last comma
    if (!functionCount.empty()) {
        outFile.seekp(-2, std::ios_base::end); // Move the pointer to remove the last comma
        outFile << "\n";
    }
    
    outFile << "  },\n";
    outFile << "  \"call_sequence\": [\n";
    
    // Output the call sequence
    for (const auto &call : callSequence) {
        outFile << "    \"" << call << "\",\n";
    }
    
    // Remove the last comma
    if (!callSequence.empty()) {
        outFile.seekp(-2, std::ios_base::end); // Move the pointer to remove the last comma
        outFile << "\n";
    }
    
    outFile << "  ]\n";
    outFile << "}\n";

    outFile.close();
}

int main(int argc, char *argv[]) {
    // Initialize the PIN library
    if (PIN_Init(argc, argv)) {
        return 1;
    }

    // Open the output file for writing
    outFile.open("control_graph.json");

    // Register image load callback
    IMG_AddInstrumentFunction(imageLoad, 0);
    
    // Register function to be called at program exit
    PIN_AddFiniFunction(fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
