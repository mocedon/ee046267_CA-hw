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
    uint addr_;
    uint tag_;
    uint LRU_;
public:
    //methods:
    Block():valid_(0), dirty_(0),addr_(0), tag_(0), LRU_(0){}

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

    uint getAddr_() const {
        return addr_;
    }

    void setAddr_(uint addr_) {
        Block::addr_ = addr_;
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
    }


};//class Block


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
        return bitExt(addr, set_, BSize_);
    }

    uint getTag(uint addr){
        uint ones = 0xFFFFFFFF;
        return (addr >> (BSize_+set_)) & ones;
    }
/*
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
*/
    bool isBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid())
                return true;
        }
        return false;
    }

    void setInvalid(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid())
                block.setValid(false);
        }
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
        b->setLRU(1); // 1 means the most recently used in line

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

    void setDirty(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag))
                block.setDirty(true);
        }
    }

    bool isFree(uint addr){
        uint set = getSet(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid()))
                return true;
        }
        return false;
    }

    uint chooseVictim(uint addr){
        uint set = getSet(addr);
        uint maxLRU = (1u << ways_);
        for (auto& block : cache[set]){
            if (block.getLRU() == maxLRU)
                block.setValid(false);
                return block.getAddr_();
        }
    }

    void newBlock(uint addr){
        uint set = getSet(addr);
        uint tag = getTag(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid())){
                block.setAddr_(addr);
                block.setTag(tag);
                block.setValid(true);
                block.setDirty(false);
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
    uint L1Cyc_;//latency for searching (+access) data in cache
    uint L2Cyc_;
    uint WrAlloc_;// 1=Write Allocate, 0=Write No Allocate
    Cache L1; // L1 cache
    Cache L2; // L2 cache

    //methods
    System(uint MemCyc, uint BSize, uint L1Size, uint L2Size, uint L1Assoc,uint L2Assoc,
           uint L1Cyc, uint L2Cyc, uint WrAlloc);


    void runCommand(char op, uint addr);

    void calcStats(double& L1miss, double& L2miss, double& avgTime){
        L1miss =(L1count_ - L2count_) / L1count_;
        L2miss = (L2count_ - memCount_) / L2count_;
        avgTime = L1Cyc_ + (L2Cyc_ * L1miss) + (MemCyc_ * L1miss + L2miss);
    }

    void accessL1(){
        L1count_++;
    }

    void accessL2(){
        L2count_++;
    }

    void accessMem(){
        memCount_++;
    }

};//class System


#endif //HW2_CACHESIM_H
