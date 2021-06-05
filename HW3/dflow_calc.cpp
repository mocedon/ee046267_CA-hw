/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <vector>
#include <cmath>

using namespace std;

class ProgElem
{
public:
    // fields
    unsigned int lat_;
    int src1D_;
    int src2D_;
    int entry_;
    int exit_;
    unsigned int dur_; //duration

    //method - C'tor
    ProgElem():lat_(0), src1D_(0), src2D_(0), entry_(0), exit_(1), dur_(0){}
};

class Ctx
{
public:
    unsigned int instNum_;
    vector<ProgElem> c;

    Ctx():instNum_(0){}
}*pCtx;

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts)
{
    //indices in this array stand for the destination registers,
    //each cell contains the recent instruction index whose dst register is the index of array
    int dstHelp[MAX_OPS] = {-1};

    /*
    //creating the data base
    ProgElem* ctx = new ProgElem[numOfInsts];

    if(ctx == nullptr)
        return PROG_CTX_NULL;
    */
    pCtx->instNum_ = numOfInsts;
    for (int i = 0; i < numOfInsts; ++i)
    {
        pCtx->c.push_back(ProgElem());
    }

    for (int i = 0; i < numOfInsts; ++i) {
        pCtx->c[i].lat_ = opsLatency[progTrace[i].opcode];


        pCtx->c[i].src1D_ = dstHelp[progTrace[i].src1Idx];
        pCtx->c[i].src2D_ = dstHelp[progTrace[i].src2Idx];


        dstHelp[progTrace[i].dstIdx] = i;


        if ((pCtx->c[i].src1D_ == -1) && (pCtx->c[i].src2D_ == -1))
            pCtx->c[i].entry_ = 1;


        if (pCtx->c[i].src1D_ >= 0)
            pCtx->c[pCtx->c[i].src1D_].exit_ = 0;

        if (pCtx->c[i].src2D_ >= 0)
            pCtx->c[pCtx->c[i].src2D_].exit_ = 0;




        if (pCtx->c[i].entry_ == 1)
            pCtx->c[i].dur_ = pCtx->c[i].lat_;
        else if (pCtx->c[i].src1D_ == -1)
            pCtx->c[i].dur_ = pCtx->c[pCtx->c[i].src2D_].dur_ + pCtx->c[i].lat_;
        else if (pCtx->c[i].src2D_ == -1)
            pCtx->c[i].dur_ = pCtx->c[pCtx->c[i].src1D_].dur_ + pCtx->c[i].lat_;
        else // both of current command's sources depend on former commands
            pCtx->c[i].dur_ = fmax(pCtx->c[pCtx->c[i].src1D_].dur_, pCtx->c[pCtx->c[i].src2D_].dur_) + pCtx->c[i].lat_;

    }
    return pCtx;
}


void freeProgCtx(ProgCtx ctx)
{
    return;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst)
{
    //ctx = pCtx;
    if ((theInst < 0) || (theInst >= pCtx->instNum_))
        return -1;

    return (int)(pCtx->c[theInst].dur_ - pCtx->c[theInst].lat_);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst)
{
    if ((theInst < 0) || (theInst >= pCtx->instNum_))
        return -1;

    *src1DepInst = pCtx->c[theInst].src1D_;
    *src2DepInst = pCtx->c[theInst].src2D_;
    return 0;
}

int getProgDepth(ProgCtx ctx)
{
    int max = 0;
    for (unsigned int i = 0; i < pCtx->instNum_; ++i)
    {
        if((pCtx->c[i].exit_==1) && (pCtx->c[i].dur_ > max))
            max = pCtx->c[i].dur_;
    }
    return max;
}


