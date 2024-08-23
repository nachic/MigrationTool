#include "pin.H"
#include <fstream>
#include <iostream>
#include <map>
#include <set>

std::ofstream OutFile;
std::map<std::string, int> FunctionCallCounts;
std::map<std::string, std::set<std::string>> BasicBlocks;
std::map<std::string, int> Edges;
std::map<std::string, int> Nodes;
std::map<std::string, int> CyclomaticComplexities;

VOID FunctionEntry(THREADID threadid, CONTEXT *ctxt, ADDRINT addr, std::string *name) {
    FunctionCallCounts[*name]++;
}

VOID BasicBlockEntry(std::string *name, ADDRINT bbAddr, ADDRINT fallThroughAddr) {
    std::string bbName = *name + ":" + std::to_string(bbAddr);
    BasicBlocks[*name].insert(bbName);
    Edges[*name]++;
    Nodes[*name] = BasicBlocks[*name].size();
}

VOID Image(IMG img, VOID *v) {
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            std::string name = RTN_Name(rtn);
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)FunctionEntry,
                           IARG_THREAD_ID, IARG_CONTEXT,
                           IARG_ADDRINT, RTN_Address(rtn),
                           IARG_PTR, new std::string(name),
                           IARG_END);

            for (BBL bbl = RTN_BblHead(rtn); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
                BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)BasicBlockEntry,
                               IARG_PTR, new std::string(name),
                               IARG_ADDRINT, BBL_Address(bbl),
                               IARG_ADDRINT, BBL_FallThrough(bbl),
                               IARG_END);
            }

            RTN_Close(rtn);
        }
    }
}

VOID Fini(INT32 code, VOID *v) {
    OutFile << "digraph ComplexityGraph {" << std::endl;
    for (const auto &entry : FunctionCallCounts) {
        const std::string &name = entry.first;
        int cyclomaticComplexity = Edges[name] - Nodes[name] + 2;
        CyclomaticComplexities[name] = cyclomaticComplexity;

        OutFile << "\"" << name << "\" [label=\"" << name << "\\n"
                << "Calls: " << entry.second << "\\n"
                << "Cyclomatic Complexity: " << cyclomaticComplexity << "\\n\"];" << std::endl;
    }
    OutFile << "}" << std::endl;
    OutFile.close();
}

int main(int argc, char *argv[]) {
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) {
        std::cerr << "Error starting PIN" << std::endl;
        return 1;
    }

    OutFile.open("complexity_graph.dot");

    IMG_AddInstrumentFunction(Image, nullptr);
    PIN_AddFiniFunction(Fini, nullptr);

    PIN_StartProgram();
    return 0;
}
