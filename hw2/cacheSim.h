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
/*
 * Extract k bit starting from position p
 */
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
    uint addr_; // Whole address of block (including offset)
    uint tag_;
    uint LRU_; //0 is most recently used,
public:
    //methods:


    Block(uint LRU):valid_(0), dirty_(0), tag_(0), addr_(0), LRU_(LRU){}  //C'tor

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
    };


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

    uint getSet(uint addr){ // Extract set value from address
        return bitExt(addr, set_, BSize_);
    }

    uint getTag(uint addr){ // Extract tag value from address
        return bitExt(addr, ADDR_BITS - set_ - BSize_, BSize_ + set_);
    }

    bool isBlock(uint addr){ // Search for a tag in a cache line according to set
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){ // Iterate over all blocks in the set
            if ((block.getTag() == tag) && block.isValid())
                return true;
        }
        return false;
    }

    void setInvalid(uint addr){ // Change block in address to invalid
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid()) {
                block.setValid(false);
                block.setLRU(1u << ways_); // Does not play a part
            }
        }
    }

    void updateLRU(uint addr){ // Change LRU for all blocks in set
        uint set = getSet(addr);
        uint tag = getTag(addr);

        uint curr; // Current block's LRU value
        Block* b;
        for (auto& block : cache[set]) {
            if (block.getTag() == tag && block.isValid()) {
                curr = block.getLRU();
                b = &block; // Save it to update last
            }
        }
        for (auto& block : cache[set]) {
            if ((block.getLRU() < curr) && block.isValid()) {
                // LRU value is smaller then current
                block.setLRU(block.getLRU() + 1);
            }
        }
        b->setLRU(0); // 0 is most recently used

    }

    void writeBlock(uint addr){ // Change block in address to dirty
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]) {
            if ((block.getTag() == tag) && block.isValid()) {
                block.setDirty(true);
            }
        }
        updateLRU(addr);
    }

    bool isDirty(uint addr){ // Check block in address if it si dirty
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid() && block.isDirty())
                return true;
        }
        return false;
    }

    void setDirty(uint addr){ // Change block in address to dirty
        uint set = getSet(addr);
        uint tag = getTag(addr);

        for (auto& block : cache[set]){
            if ((block.getTag() == tag) && block.isValid()) {
                block.setDirty(true);
                return;
            }
        }
    }

    bool isFree(uint addr){ // Check if set has block that is invalid
        uint set = getSet(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid()))
                return true;
        }
        return false;
    }

    uint chooseVictim(uint addr){ // Search for least recently used block in set
        uint set = getSet(addr);
        uint maxLRU = (1u << ways_) - 1;
        for (auto& block : cache[set]){
            if ((block.getLRU() == maxLRU) && block.isValid()) {
                return block.getAddr_();
            }
        }
    }

    void newBlock(uint addr){ // sets and new block for address
        uint set = getSet(addr);
        uint tag = getTag(addr);
        for (auto& block : cache[set]){
            if (!(block.isValid())){
                block.setAddr_(addr);
                block.setTag(tag);
                block.setValid(true);
                block.setDirty(false);
                updateLRU(addr);
                return;
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
        if (L1count_)
            L1miss = (double)(L2count_) / L1count_; // Each access to L2 means a miss in L1
        if (L2count_)
            L2miss = (double)(memCount_) / L2count_; // Each access to mem means a miss in L2

        avgTime = L1Cyc_ + ((double)L2Cyc_ * L1miss) + ((double)MemCyc_ * L1miss * L2miss);
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

    bool blockAlloc(char op){ // Block should be allocated to read or write allocate

        return ((op == 'r') || ((WrAlloc_ == 1) && (op == 'w')));
    }

};//class System


#endif //HW2_CACHESIM_H
