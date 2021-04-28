/* 046267 Computer Architecture - Spring 2020 - HW #1 */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <vector>
#include "math.h"
using namespace std;

//defining global variables in a case of global FSM/history
int globalHistory = 0;
vector <int> globalFsm;

//the following method's aim is to extract k bits from specific position p in a given number
int bitExtracted(int number, int k, int p)
{
    return (((1 << k) -1) & (number >> (p-1)));
}

//defining a public class which will represent a row in the BTB table
class row{
public:
    row():valid_(0),tag_(0),target_(0),history_(0){}; //C'tor
    //  לשים לב להערה שכתבתי בטאבלט
    int valid_; // 0 = this instruction wasn't mapped to the BTB, 1=it was
    int tag_;
    int target_;
    int history_;
};

/*defining a public class which will represent the BTB table
since the class features and methods are public, we will be able
accessing and using them in external functions*/
class BTB *btb;
class BTB {
public:
    /*features*/
    unsigned btbSize_;
    unsigned historySize_;
    unsigned tagSize_;
    unsigned fsmState_;
    bool isGlobalHist_;
    bool isGlobalTable_;
    int Shared_;
    int locality_; //indicates one of 4 combinations of local/global history and FSM
    int flushes_; //counting the number of flushes in a case off wrong prediction
    int branches_; //counting the number of branch instructions
    vector <vector<int>> fsm;
    vector <row> rows;

    /*Methods*/
    /*Constructor - filling the btb fields and creating the table*/
    BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
        bool isGlobalHist, bool isGlobalTable, int Shared):btbSize_(btbSize), historySize_(historySize),
                                                           tagSize_(tagSize),fsmState_(fsmState), isGlobalHist_(isGlobalHist), isGlobalTable_(isGlobalTable),
                                                           Shared_(Shared)
    {
        flushes_ = 0;
        branches_ = 0;

        /*deciding one of 4 combinations of local/global history and FSM*/
        /* locality = 0 --> local history, local FSM */
        if((!isGlobalHist_) && (!isGlobalTable_))
            locality_ = 0;
            /* locality = 1 --> local history, global FSM */
        else if((!isGlobalHist_) && (isGlobalTable_))
            locality_ = 1;
            /* locality = 2 --> global history, global FSM */
        else if((isGlobalHist_) && (isGlobalTable_))
            locality_ = 2;
            /* locality = 3 --> global history, local FSM */
        else
            locality_ = 3;


        /*creating the BTB itself by vector of rows*/
        for (int i=0; i<btbSize_; i++)
            rows.push_back(row());


        /*initializing the FSM in a case it is global*/
        if ((locality_ == 1) || (locality_ == 2))
            for (int i = 0; i < pow(2, historySize_); i++)
                globalFsm.push_back(fsmState_);

            /*PAY ATTENTION : We don't need to initialize the FSM in a case it is local
            since we will do it in update function BUT because there is a problem from
             avoid doing this after that in predict and update function we finally do this*/

            /*initializing the FSM in a case it is local*/
        else
        {
            for (int i=0; i<btbSize_; i++)
            {
                vector <int> temp;
                fsm.push_back(temp);
                for (int j = 0; j < pow(2, historySize_); j++) {
                    fsm[i].push_back(fsmState_);
                }
            }
        }
    }//C'tor

    /*extractFsmState*/
    int* extractFsmState(uint32_t pc)
    {
        /*extracting the bits used for mapping the row in the BTB*/
        int mappedRow = bitExtracted((int)pc, (int)log2(btbSize_), 3);
        int* state = nullptr;
        // if(btb->rows[mappedRow].valid_==0)
        //    return state;
        /*extracting the current fsm state*/
        if(locality_ == 0)
            state = &btb->fsm[mappedRow][btb->rows[mappedRow].history_];
        else if(locality_ == 3)
            state = &btb->fsm[mappedRow][globalHistory];
        else // fsm is global
        {
            int histBits;
            int pcBits;
            //extracting pc bits according to kind of share
            switch (Shared_)
            {
                case 0:
                    pcBits = 0;
                    break;
                case 1: //lsb share
                    pcBits = bitExtracted((int)pc, (int)historySize_, 3);
                    break;
                case 2: //msb share
                    pcBits = bitExtracted((int)pc, (int)historySize_,17);
                    break;
            }
            //extracting relevant history bits
            if(locality_ == 1)
                histBits = btb->rows[mappedRow].history_;
            else // locality_ = 2
                histBits = globalHistory;
            state = &globalFsm[pcBits ^ histBits];
        }
        return state;
    } //extractFsmState


    /*fsmUpdate
     *giving a feedback to relevant fsm according to actual resolution
     */
    void fsmUpdate(bool taken, int* state)
    {
        // if(state == nullptr)
        //     return;
        if((*state == 0 && taken == 0) || (*state == 3 && taken == 1))
            return;
        else
        {
            if(taken == 1)
                (*state)++;
            else
                (*state)--;
        }
    }//fsmUpdate


    /*historyUpdate
     *
     **/
    void historyUpdate(bool taken, uint32_t pc) {
        int mappedRow = bitExtracted((int) pc, (int) log2(btbSize_), 3);
        //help bits arr all equal to 1 and its length is history length
        int help = (1 << historySize_) - 1;
        if (locality_ >= 2) //global history
        {
            if(taken)
                globalHistory = ((globalHistory << 1) + 1) & help;
            else
                globalHistory = (globalHistory << 1)  & help;
        }
        else {
            if (taken)
                (*btb).rows[mappedRow].history_ = ((btb->rows[mappedRow].history_ << 1) + 1) & help;
            else
                (*btb).rows[mappedRow].history_ = (btb->rows[mappedRow].history_ << 1) & help;
        }
    } //historyUpdate
};


int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
            bool isGlobalHist, bool isGlobalTable, int Shared)
{
    btb = new(nothrow) BTB(btbSize,historySize,tagSize,fsmState,isGlobalHist,
                           isGlobalTable,Shared);
    if(btb == nullptr)
        return -1;
    return 0;
}//BP_init


bool BP_predict(uint32_t pc, uint32_t *dst)
{
    int inputTag = bitExtracted((int)pc, (int)btb->tagSize_, 3+(int)log2(btb->btbSize_));
    int* state = btb->extractFsmState(pc);
    int mappedRow = bitExtracted((int)pc, (int)log2(btb->btbSize_), 3);
    //the given branch is in the table and it's predicted to be taken
    if((btb->rows[mappedRow].valid_)&&(btb->rows[mappedRow].tag_ == inputTag)&&(*state > 1))
    {
        *dst = btb->rows[mappedRow].target_;
        return true;
    }
        /* other 3 options : 1. current branch is in the table but not taken
        *                    2. there is a branch in the mapped row but its not the same
        *                       branch(different tag)
        *                    3. the mapped row is empty (valid=0);
        */
    else
    {
        *dst = pc + 4;
        return false;
    }
}//BP_predict

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
    // PAY ATTENTION : assuming target isn't changed for specific branch through the program

    btb->branches_++;

    /*cases for flush: 1.predicted resolution and target differs from actual resolution and target
     *                 2.predicted resolution equals to actual resolution but destinations
     *                 are different. happens when mapped row and tag aren't enough to
     *                 distinguish between different branches
     */
    int* state = btb->extractFsmState(pc); //predicted resolution
    //int predicted = (*state>1)? 1:0;
    //if((predicted || taken)&&(targetPc != pred_dst))
    bool my_predict = (targetPc == pred_dst);
    if (my_predict != taken)
        btb->flushes_++;

    /* the update itself */
    int mappedRow = bitExtracted((int)pc, (int)log2(btb->btbSize_), 3);
    int inputTag = bitExtracted((int)pc, (int)btb->tagSize_, 3+(int)log2(btb->btbSize_));
    /*if there row is empty or there us a different branch in that row - we need to
     * insert a new row so there are additional updates
     */
    if((!btb->rows[mappedRow].valid_)||(btb->rows[mappedRow].tag_ != inputTag))
    {
        if (!btb->rows[mappedRow].valid_)
            btb->rows[mappedRow].valid_=1;

        if(!btb->isGlobalHist_)//local history
            btb->rows[mappedRow].history_ = 0;
        if(!btb->isGlobalTable_)//local FSM
            for (int j = 0; j < pow(2, btb->historySize_); j++)
                btb->fsm[mappedRow].push_back(btb->fsmState_);

        btb->rows[mappedRow].target_ =  (int)targetPc;
        btb->rows[mappedRow].tag_ =  (int)inputTag;
    }

    /* now we make the updates that relevant to any case*/
    btb->fsmUpdate(taken,btb->extractFsmState(pc));
    btb->historyUpdate(taken,pc);
}//BP_update


void BP_GetStats(SIM_stats *curStats)
{
    curStats->br_num = btb->branches_;
    curStats->flush_num = btb->flushes_;
    /* 30 in the following calculation stands for the target bits */
    switch (btb->locality_)
    {
        case 0:
            curStats->size = btb->btbSize_ * ((int)btb->tagSize_ + 30  + (int)btb->historySize_ + (int)pow(2, btb->historySize_ + 1));
            break;
        case 1:
            curStats->size = btb->btbSize_ * ((int) btb->tagSize_ + 30 + (int) btb->historySize_)
                             + (int) pow(2, btb->historySize_ + 1);
            break;
        case 2:
            curStats->size = btb->btbSize_ * ( btb->tagSize_ + 30) + (int) btb->historySize_
                             + (int) pow(2, btb->historySize_ + 1);
            break;
        case 3:
            curStats->size = btb->historySize_ + btb->btbSize_ *
                                                 (btb->tagSize_ + 30 +(int)pow(2, btb->historySize_ + 1));
            break;
    }//switch
    delete(btb); //frees the dynamic memory allocated in the Ctor
    return;
}

