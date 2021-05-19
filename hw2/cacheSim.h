#include <vector>
#include <math.h>

#ifndef HW2_CACHESIM_H
#define HW2_CACHESIM_H

#define ADDRESS_BITS_NUM 32
#define EVICT -1
#define AVAILABLE 0
#define FOUND 1

/* Global Functions */
int bitExtracted(unsigned number, unsigned k, unsigned p)




/* Class Block */

class Block
{
public:
    // fields:
    int valid_; // 0 = empty block
    int tag_;
    int dirty_; //0=not dirty regarding the lower level
    int LRU_;

    //methods:
    Block():valid_(0),tag_(0),dirty_(0),LRU_(0){}; //C'tor
};//class Block


/*
 *                             *****is the class really NECESSARY?
class Line
{
public:
    // fields:
    vector <Block> line;
    //int ID_; // mapped by set *****NECESSARY?

    //methods:
    Line(unsigned ways)
    {
        for (unsigned int i=0; i<pow(2, ways); i++)
            line.push_back(Block());
    }//C`tor Line
};
*/


class Cache
{
public:
    // fields:
    vector <Line> cache;
    unsigned CSize_; // cache size           *****  LOG FORM  *****
    unsigned ways_; //=associativity         *****  LOG FORM  *****
    unsigned setBitsNum_; //number of bits for mapping to the relevant line in cache      *****  REGULAR FORM ******
    bool isDirectMap_; //there is no eviction policy in that case


    //methods:
    Cache(unsigned CSize, unsigned ways):CSize_(CSize), ways_(ways)

    int findInCacheLevel(int level, unsigned addr, unsigned* way)

    int findVictim(unsigned level, unsigned addr)

    void lruUpdate(unsigned level, unsigned addr, unsigned accessedWay)


};//class Cache


class System
{
public:
    //fields
    unsigned L1count_; //number of accessing to L1
    unsigned L2count_;
    unsigned memCount_;
    unsigned MemCyc_;
    unsigned BSize_;                    //      *****  LOG FORM  *****
    unsigned L1Cyc_;//latency for searching (+access) data in cache
    unsigned L2Cyc_;
    unsigned WrAlloc_;// 1=Write Allocate, 0=Write No Allocate
    Cache arr[2]; //containing cache levels - L1 in arr[0], L2 in arr[1]

    //methods
    System(unsigned MemCyc, unsigned BSize, unsigned L1Size, unsigned L2Size, unsigned L1Assoc,unsigned L2Assoc,
           unsigned L1Cyc, unsigned L2Cyc, unsigned WrAlloc){};


    void runCommand(char op, string cutAddr)
    double L1MissRateCalc()
    double L2MissRateCalc()
    double avgAccTimeCalc(double L1MissRate, double L2MissRate)

};//class System


#endif //HW2_CACHESIM_H
