#include "pin.H"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <json/json.h>

// JSON file for storing test data
std::string jsonFile = "test_data.json";

// Stores function names to be instrumented
std::unordered_set<std::string> targetFunctions;

// Stores captured input-output pairs for each function
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> functionIOData;

// Stores non-deterministic functions
std::unordered_set<std::string> nonDeterministicFunctions;

// Function to load target functions from the .properties file
VOID LoadTargetFunctions(const std::string &filename)
{
    std::ifstream inFile(filename);
    std::string functionName;
    while (inFile >> functionName)
    {
        targetFunctions.insert(functionName);
    }
    inFile.close();
}

// Helper function to convert input arguments to a string
std::string ArgsToString(const CONTEXT *ctxt)
{
    std::ostringstream oss;
    oss << std::hex;

    for (int i = 0; i < 6; ++i)  // Assuming up to 6 arguments
    {
        ADDRINT arg;
        PIN_GetContextRegval(ctxt, REG_INST_G0 + i, reinterpret_cast<UINT8*>(&arg));
        oss << arg << ",";
    }

    return oss.str();
}

// This function is called before every target function executes
VOID FuncEntry(CONTEXT *ctxt, std::string *funcName)
{
    std::string input = ArgsToString(ctxt);

    // Check if the function has been called before with the same input
    if (functionIOData[*funcName].count(input))
    {
        // The function is deterministic if it has produced the same output for the same input
        std::string existingOutput = functionIOData[*funcName][input];
        std::string newOutput = "unknown"; // This will be captured after the function executes

        if (existingOutput != newOutput)
        {
            nonDeterministicFunctions.insert(*funcName);
        }
    }
}

// This function is called after every target function executes
VOID FuncExit(std::string *funcName, ADDRINT returnValue)
{
    std::ostringstream oss;
    oss << std::hex << returnValue;

    std::string output = oss.str();

    // Capture the input string for this function
    std::string input = ArgsToString(NULL);  // NULL since we're post-execution

    // Store the input-output pair if it's unique
    if (!functionIOData[*funcName].count(input))
    {
        functionIOData[*funcName][input] = output;
    }
}

// This routine is executed for each image loaded
VOID Image(IMG img, VOID *v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            std::string *funcName = new std::string(RTN_Name(rtn));

            if (targetFunctions.find(*funcName) != targetFunctions.end())
            {
                RTN_Open(rtn);

                RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)FuncEntry,
                               IARG_CONTEXT, IARG_PTR, funcName, IARG_END);

                RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)FuncExit,
                               IARG_PTR, funcName, IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

                RTN_Close(rtn);
            }
        }
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Load existing JSON data
    Json::Value root;
    std::ifstream jsonFileIn(jsonFile, std::ifstream::binary);
    if (jsonFileIn)
    {
        jsonFileIn >> root;
    }
    jsonFileIn.close();

    // Update the JSON with captured test data
    for (const auto &entry : functionIOData)
    {
        const std::string &funcName = entry.first;
        Json::Value &funcJson = root[funcName];

        for (const auto &ioPair : entry.second)
        {
            const std::string &input = ioPair.first;
            const std::string &output = ioPair.second;

            bool found = false;
            for (const auto &existingData : funcJson)
            {
                if (existingData["input"].asString() == input)
                {
                    found = true;
                    if (existingData["output"].asString() != output)
                    {
                        existingData["non_deterministic"] = true;
                    }
                    break;
                }
            }

            if (!found)
            {
                Json::Value newData;
                newData["input"] = input;
                newData["output"] = output;
                newData["non_deterministic"] = false;
                funcJson.append(newData);
            }
        }

        if (nonDeterministicFunctions.find(funcName) != nonDeterministicFunctions.end())
        {
            root[funcName]["non_deterministic"] = true;
        }
    }

    // Save the updated JSON data
    std::ofstream jsonFileOut(jsonFile, std::ofstream::binary);
    jsonFileOut << root;
    jsonFileOut.close();
}

// Main function
int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv))
    {
        std::cerr << "PIN Initialization Failed" << std::endl;
        return 1;
    }

    LoadTargetFunctions("functions.properties");

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();
    return 0;
}
