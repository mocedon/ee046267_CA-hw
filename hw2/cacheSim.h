#include <vector>
#include <math.h>

#ifndef HW2_CACHESIM_H
#define HW2_CACHESIM_H

#define ADDR_BITS 32u
#define EVICT -1
#define AVAILABLE 0
#define FOUND 1



using std::vector;
using std::string;


typedef unsigned int uint;


/*** Global Functions ***/

uint bitExt(uint num, uint k, uint p)
{
    return (((1u << k) -1u) & (num >> p));
}


/* Class Block */

class Block
{
private:
    // fields:
    bool valid_; // 0 = empty block
    bool dirty_; //0=not dirty regarding the lower level
    uint tag_;
    uint LRU_;
public:
    //methods:
    Block():valid_(0), tag_(0), dirty_(0), LRU_(0){}

    bool isValid() const {
        return valid_;
    }

    void setValid(bool valid_) {
        Block::valid_ = valid_;
    }

    bool isDirty() const {
        return dirty_;
    }

    void setDirty(bool dirty_) {
        Block::dirty_ = dirty_;
    }

    uint getTag() const {
        return tag_;
    }

    void setTag(uint tag_) {
        Block::tag_ = tag_;
    }

    uint getLRU() const {
        return LRU_;
    }

    void setLRU(uint LRU_) {
        Block::LRU_ = LRU_;
    }; //C'tor


};//class Block
/*
// *                             *****is the class really NECESSARY?
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
    vector<vector<Block>> cache;
    uint CSize_; // cache size           *****  LOG FORM  *****
    uint BSize_; // Block size           *****  LOG FORM  *****
    uint ways_; // =associativity         *****  LOG FORM  *****
    uint set_; //number of bits for mapping to the relevant line in cache      *****  LOG FORM ******

    //methods:
    Cache(uint CSize, uint ways, uint BSize);

    uint getSet(uint addr){
        return bitExt(addr, BSize_ + CSize_, BSize_);
    }

    uint getTag(uint addr){
        uint ones = 0xFFFFFFFF;
        return (addr >> (CSize_ + BSize_)) & ones;
    }

    uint getBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);
        uint i = 0u;
        for (auto& block : cache[set]){
            if ((block.getTag() == tag))
                return i;
            i++;
        }

    }

    bool isBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid())
                return true;
        }
        return false;
    }

    void updateLRU(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        uint curr;
        Block* b;
        for (auto& block : cache[set]) {
            if (block.getTag() == tag) {
                curr = block.getLRU();
                b = &block;
            }
        }
        for (auto& block : cache[set]) {
            if ((block.getLRU() < curr) && block.isValid())
                block.setLRU(block.getLRU() + 1);
        }
        b->setLRU(1);

    }

    void writeBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]) {
            if ((block.getTag() == tag) && block.isValid()) {
                block.setDirty(true);
            }
        }
        updateLRU(addr);
    }

    bool isDirty(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid() && block.isDirty())
                return true;
        }
        return false;
    }

    bool isSetFree(uint addr){
        uint set = getSet(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid()))
                return true;
        }
        return false;
    }

    void chooseVictim(uint addr){
        uint set = getSet(addr);
        uint maxLRU = (2u << ways_);
        for (auto& block : cache[set]){
            if (block.getLRU() == maxLRU)
                block.setValid(false);
        }
    }

    void newBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid())){
                block.setTag(tag);
                block.setValid(true);
                updateLRU(addr);
            }
        }
    }


};//class Cache


class System
{
public:
    //fields
    uint L1count_; //number of accessing to L1
    uint L2count_;
    uint memCount_;
    uint MemCyc_;
    uint BSize_;                    //      *****  LOG FORM  *****
    uint L1Cyc_;//latency for searching (+access) data in cache
    uint L2Cyc_;
    uint WrAlloc_;// 1=Write Allocate, 0=Write No Allocate
    vector<Cache> arr; //containing cache levels - L1 in arr[0], L2 in arr[1]

    //methods
    System(uint MemCyc, uint BSize, uint L1Size, uint L2Size, uint L1Assoc,uint L2Assoc,
           uint L1Cyc, uint L2Cyc, uint WrAlloc);


    void runCommand(char op, string cutAddr);

};//class System


#endif //HW2_CACHESIM_H
