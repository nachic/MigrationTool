#include "pin.H"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <set>

// Output file for the .dot graph
std::ofstream outFile;

// Stores function execution counts
std::unordered_map<std::string, int> funcExecCount;

// Stores the edges of the control flow graph (function calls)
std::set<std::pair<std::string, std::string>> callEdges;

// Keeps track of the previous function in the call sequence
std::string prevFunc = "";

// This function is called before every function is executed
VOID FuncEntry(VOID *ip, std::string *funcName)
{
    // Increment the execution count of the function
    funcExecCount[*funcName]++;

    // Record the function call edge in the sequence if it's not the first function
    if (!prevFunc.empty())
    {
        callEdges.insert(std::make_pair(prevFunc, *funcName));
    }

    // Update previous function
    prevFunc = *funcName;
}

// This routine is executed for each image loaded
VOID Image(IMG img, VOID *v)
{
    // Instrument each routine (function)
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Name of the function
            std::string *funcName = new std::string(RTN_Name(rtn));

            // Instrument the function to call FuncEntry before execution
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)FuncEntry,
                           IARG_INST_PTR,
                           IARG_PTR, funcName,
                           IARG_END);
            RTN_Close(rtn);
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write the DOT graph to file
    outFile << "digraph ControlFlowGraph {\n";

    // Write the nodes with their execution frequency
    for (const auto &entry : funcExecCount)
    {
        outFile << "    \"" << entry.first << "\" [label=\"" << entry.first << "\\nExec Count: " << entry.second << "\"];\n";
    }

    // Write the edges
    for (const auto &edge : callEdges)
    {
        outFile << "    \"" << edge.first << "\" -> \"" << edge.second << "\";\n";
    }

    outFile << "}\n";
    outFile.close();
}

// Main function
int main(int argc, char *argv[])
{
    // Initialize PIN
    if (PIN_Init(argc, argv))
    {
        std::cerr << "PIN Initialization Failed" << std::endl;
        return 1;
    }

    // Open output file (append mode)
    outFile.open("control_flow_graph.dot", std::ios_base::app);

    // Register Image to be called to instrument functions
    IMG_AddInstrumentFunction(Image, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
