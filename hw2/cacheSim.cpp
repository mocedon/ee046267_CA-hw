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

/*** Global Functions ***/

int bitExtracted(unsigned number, unsigned k, unsigned p)
{
    return (((1 << k) -1) & (number >> (p-1)));
}



/*** Cache Methods ***/

/*C'tor Cache
 *
 * */
Cache::Cache(unsigned CSize, unsigned ways):CSize_(CSize), ways_(ways)
{
    setBitsNum_ = CSize_ - (sys.Bsize_ + ways_); //this calculation is according to logarithm rules

    for (unsigned int i=0; i<pow(2, setBitNum_); i++)
    {
        vector <Block()> line;
        cache.push_back(line);
        {
            for (unsigned int i=0; i<pow(2, ways_); i++)
                line.push_back(Block());
        }
    }

    isDirectMap_ = (ways_==0)? true:false;
}



/* return value :  -1= not found and whole line is occupied so we need to evict
 *                  0= not found but there is an available place in the line, assign to added pointer
 *                  the way of chosen block to occupy
 *                  1= found in cache, assign to added pointer the way of found block
 */
int Cache::findInCacheLevel(unsigned level, unsigned addr, unsigned* way)
{
    unsigned lev = level - 1; // level 1 is located in index 0
    unsigned tagPos = sys.arr[lev].setBitsNum_ + pow(2,sys.BSize_);
    unsigned tag = bitExtracted(addr,ADDRESS_BITS_NUM - tagPos, tagPos);
    unsigned set = bitExtracted(addr, sys.arr[lev].setBitsNum_, pow(2,sys.BSize_));

    for (int i = 0; i < pow(2,sys.arr[lev].ways_); ++i)
    {
        if(sys.arr[lev][set][i].valid_ == 0) //there is a place in the line
        {
            *way = i;
            return AVAILABLE;
        }
        //here we know that the current block is valid
        if(sys.arr[lev][set][i].tag_ == tag)
        {
            *way = i;
            return FOUND;
        }
    }
    //here we know that there is no place in the whole line so we need to evict
    return EVICT;
}


/*
 * return value : way of chosen block to be evicted
 * NOTE : this function is called only in case that "findInCacheLevel" result was EVICT
 * */
unsigned Cache::findVictim(unsigned level, unsigned addr)
{
    unsigned lev = level - 1; // level 1 is located in index 0
    //special case - direct mapping so there is no need to find victim
    if(sys.arr[lev].isDirectMap)
        return 0; //first and only way of single block in line
    unsigned set = bitExtracted(addr, sys.arr[lev].setBitsNum_, pow(2,sys.BSize_));
    for (int i = 0; i < pow(2,sys.arr[lev].ways_); ++i)
    {
        if(sys.arr[lev][set][i].LRU_ == 0)
            return i;
    }
}


/*
 * according to the algorithm learned in lectures
 * NOTE : this function would be called only in case that cache level is NOT direct mapped
 */
void Cache::lruUpdate(int level, unsigned addr, unsigned accessedWay)
{
    unsigned lev = level - 1; // level 1 is located in index 0
    unsigned set = bitExtracted(addr, sys.arr[lev].setBitsNum_, pow(2,sys.BSize_));

    //updating LRU of accessed block
    oldLRU = sys.arr[lev][set][accessedWay].LRU_;
    sys.arr[lev][set][accessedWay].LRU_ = pow(2,sys.arr[lev].ways_) - 1;

    //updating LRU of other blocks in line
    for (int i = 0; i < pow(2,sys.arr[lev].ways_); ++i)
    {
        if(i!=accessedWay)&&(sys.arr[lev][set][i].valid)&&(sys.arr[lev][set][i].LRU > oldLRU)
        sys.arr[lev][set][i].LRU--;
    }
}




/*** System Methods ***/

System::System(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,unsigned L2Assoc,
               unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc):MemCyc_(MemCyc),BSize_(BSize),L1Cyc_(L1Cyc),L2Cyc_(L2Cyc),
                                                                 WrAlloc_(WrAlloc), L1count_(0), L2count_(0), memCount_(0)
{
    arr[0] = Cache(L1Size, L1Assoc);
    arr[1] = Cache(L2Size, L2Assoc);
}//System C`tor

void System::runCommand(char op, string cutAddr)
{

}

double System::L1MissRateCalc()
{
    return(1-(sys.L1count_ - sys.L1count_)/sys.L1count_);
}

double System::L2MissRateCalc()
{
    return(1-(sys.L2count_ - sys.memCount_)/sys.L2count_);
}

double System::avgAccTimeCalc(double L1MissRate, double L2MissRate)
{
    return(sys.L1Cyc_ + L1MissRate * (sys.L2Cyc_ + L2MissRate * sys.MemCyc_));
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

    unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
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

    double L1MissRate = L1MissRateCalc();
    double L2MissRate = L2MissRateCalc();
    double avgAccTime = avgAccTimeCalc(L1MissRate, L2MissRate);

    printf("L1miss=%.03f ", L1MissRate);
    printf("L2miss=%.03f ", L2MissRate);
    printf("AccTimeAvg=%.03f\n", avgAccTime);

    return 0;
}
