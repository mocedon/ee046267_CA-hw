/* 046267 Computer Architecture - Spring 2021 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include "cacheSim.h"


using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;





/*** System Methods ***/

System::System(uint MemCyc, uint BSize, uint L1Size, uint L2Size, uint L1Assoc,uint L2Assoc,
               uint L1Cyc, uint L2Cyc, uint WrAlloc):MemCyc_(MemCyc),BSize_(BSize),L1Cyc_(L1Cyc),L2Cyc_(L2Cyc),
                                                                 WrAlloc_(WrAlloc), L1count_(0), L2count_(0), memCount_(0)
{

    arr.push_back(Cache(L1Size, L1Assoc, BSize));
    arr.push_back(Cache(L2Size, L2Assoc, BSize));
}//System C`tor

void System::runCommand(char op, string cutAddr)
{

}




/*** Cache Methods ***/

/*C'tor Cache
 *
 * */
Cache::Cache(uint CSize, uint ways, uint BSize):CSize_(CSize), ways_(ways), BSize_(BSize)
{
    set_ = CSize - BSize - ways; //this calculation is according to logarithm rules

    vector<Block> line((2u << ways), Block());
    for (uint i = 0; i < (2u << set_); i++)
    {
        cache.push_back(line);
    }
}






int main(int argc, char **argv){
    
    if (argc < 19) {
        cerr << "Not enough arguments" << endl;
        return 0;
    }

    // Get input arguments

    // File
    // Assuming it is the first argument
    char* fileString = argv[1];
    ifstream file(fileString); //input file stream
    string line;
    if (!file || !file.good()) {
        // File doesn't exist or some other error
        cerr << "File not found" << endl;
        return 0;
    }

    uint MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
            L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

    for (int i = 2; i < 19; i += 2) {
        string s(argv[i]);
        if (s == "--mem-cyc") {
            MemCyc = atoi(argv[i + 1]);
        } else if (s == "--bsize") {
            BSize = atoi(argv[i + 1]);
        } else if (s == "--l1-size") {
            L1Size = atoi(argv[i + 1]);
        } else if (s == "--l2-size") {
            L2Size = atoi(argv[i + 1]);
        } else if (s == "--l1-cyc") {
            L1Cyc = atoi(argv[i + 1]);
        } else if (s == "--l2-cyc") {
            L2Cyc = atoi(argv[i + 1]);
        } else if (s == "--l1-assoc") {
            L1Assoc = atoi(argv[i + 1]);
        } else if (s == "--l2-assoc") {
            L2Assoc = atoi(argv[i + 1]);
        } else if (s == "--wr-alloc") {
            WrAlloc = atoi(argv[i + 1]);
        } else {
            cerr << "Error in arguments" << endl;
            return 0;
        }
    }
    System sys(MemCyc, BSize, L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc, WrAlloc);
    while (getline(file, line)) {

        stringstream ss(line);
        string address;
        char operation = 0; // read (R) or write (W)
        if (!(ss >> operation >> address)) {
            // Operation appears in an Invalid format
            cout << "Command Format error" << endl;
            return 0;
        }

        string cutAddress = address.substr(2); // Removing the "0x" part of the address

        unsigned long int num = 0;
        num = strtoul(cutAddress.c_str(), NULL, 16);
// *******************   HERE WE NEED TO EXECUTE THE OPERATION     **********************

    }

    double L1MissRate;
    double L2MissRate;
    double avgAccTime;



    printf("L1miss=%.03f ", L1MissRate);
    printf("L2miss=%.03f ", L2MissRate);
    printf("AccTimeAvg=%.03f\n", avgAccTime);

    return 0;
}
