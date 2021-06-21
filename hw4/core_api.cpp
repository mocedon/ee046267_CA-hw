/* 046267 Computer Architecture - Spring 2021 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>
#include <vector>

/* Class engulfing the whole multi-thread operation
 * thread      - Represents the current working thread, each clock has only 1 active thread at most, initialized to 0
 * clck        - Represents the clock counter
 * inst        - Represents the instruction counter
 * sw          - Represents thread switch event (Block) used only in blccking MT
 * thread_num  - Represents the number of threads (active and non-active)
 * thread_inst - Represents the current instruction on each thread, starts at 0, -1 non-active
 * thread_halt - Represents the halt timer for each thread, for load/Store OP
 * thread_ctxt - Represents the context of each thread separately */
class MT {
public:
    // Constructor initializes everything to 0
	MT() : thread(0), clck(0), inst(0), sw(false){
        thread_num = SIM_GetThreadsNum();
	    thread_inst = std::vector<int>(thread_num, 0);
        thread_halt = std::vector<int>(thread_num, 0);
        thread_ctxt = std::vector<tcontext>(thread_num);
		for (int i = 0; i < thread_num; i++){
			for (int j = 0; j < REGS_COUNT; j++){
				thread_ctxt[i].reg[j] = 0;
			}
		}
	}

    // Offsets the +1 on first round for fine grained MT
	void thread_fix() {
	    thread = -1;
	}

	// Goes over all threads checking if they are done or still active (might be idle)
    bool thread_active(){
        for (int i = 0; i < thread_num; i++){
            if (thread_inst[i] > -1){ // Instruction -1 is representing tread reached OP - halt
                return true;
            }
        }
        return false;
    }

    // Picks the next thread that isn't idle
    bool pick_thread(int start){
        // start can be either 0 or 1
        // start = 0 : blocked MT first search the current thread
        // start = 1 : grained MT first search the next thread
        for (int i = start; i < (thread_num + start); i++){
            int j = (thread + i) % thread_num; // Constraints j to range 0 - thread_num
            if ((thread_inst[j] > -1) && (thread_halt[j] == 0)) { // Thread is active and not on idle
                if (sw && (thread != j)){ // Used in blocking TM only
                    // In case that a switch event is reached and a new thread in picked we need to change context
                    for (int i = 0; i < SIM_GetSwitchCycles(); i++){
                        update_halt(); // For switch_cycles run the clock forward
                    }
                    sw = false; // Terminate the switch event
                }
                thread = j; // Current thread will be j
                return true;
            }
        }
        return false; // All threads are idle
    }

    // Set register in current thread to a curtain value
    void set_reg(int dst, int val){
	    thread_ctxt[thread].reg[dst] = val;
	}

	// Get the value of a curtain register on current thread
	int get_reg(int reg) {
	    return thread_ctxt[thread].reg[reg];
	}

    // Moves clock forward updating idle threads
	void update_halt() {
        clck++;
        for (int i = 0; i < thread_num; i++){
            if (thread_halt[i] > 0){ // Thread is idle
                thread_halt[i]--;
            }
        }
    }

    // Computes an operation
    void op(bool sw_penalty) {
        // Blocked MT : sw_penalty = true
        // Grained MT : sw_penalty = false
	    inst++; // increase instruction count

	    // Get the instruction for current thread
	    Instruction curr_inst;
	    SIM_MemInstRead((uint32_t)thread_inst[thread], &curr_inst, thread);
	    thread_inst[thread]++;
	    cmd_opcode opc = curr_inst.opcode;
	    int dst_reg = curr_inst.dst_index;
	    int src1 = curr_inst.src1_index;
	    int src2 = curr_inst.src2_index_imm;
	    bool imm = curr_inst.isSrc2Imm;

	    int addr; // Saves the address for load/store op
	    int data; // Saves the value for load/store op

	    switch(opc) {
	        case CMD_NOP: // Nop - do nothing
	            break;
	        case CMD_ADD: // Add r[dst] <- r[src1] + r[src2]
	            set_reg(dst_reg, get_reg(src1) + get_reg(src2));
	            break;
            case CMD_ADDI: // Add r[dst] <- r[src1] + src2
                set_reg(dst_reg, get_reg(src1) + src2);
                break;
	        case CMD_SUB: // Subtract r[dst] <- r[src1] - r[src2]
                set_reg(dst_reg, get_reg(src1) - get_reg(src2));
	            break;
	        case CMD_SUBI: // Subtract r[dst] <- r[src1] - src2
                set_reg(dst_reg, get_reg(src1) - src2);
	            break;
	        case CMD_LOAD:
	            addr = get_reg(src1) + (imm ? src2 : get_reg(src2)); // In case of immediate add src2
	            SIM_MemDataRead((uint32_t)addr, &data);
	            set_reg(dst_reg, data);
	            thread_halt[thread] = SIM_GetLoadLat() + 1 ; // including this clock cycle
	            sw = sw_penalty; // Switch event for blocked MT
	            break;
	        case CMD_STORE:
                addr = get_reg(dst_reg) + (imm ? src2 : get_reg(src2)); // In case of immediate add src2
                SIM_MemDataWrite((uint32_t)addr, get_reg(src1));
                thread_halt[thread] = SIM_GetStoreLat() + 1; // including this clock cycle
                sw = sw_penalty; // Switch event for blocked MT
	            break;
	        case CMD_HALT:
	            thread_inst[thread] = -1; // Switch thread to non-active
                sw = sw_penalty; // Switch event for blocked MT
	            break;
	    }
	}

    // Get the context for a specific thread
	tcontext get_context(int id) {
	    return thread_ctxt[id];
	}

	// Calculate the CPI for the MT
    double get_CPI() {
	    return ((double)clck) / ((double)inst);
	}

private:

	int thread;
    int clck;
    int inst;
	int thread_num;
    bool sw;

    std::vector<int> thread_inst;
	std::vector<int> thread_halt;

	std::vector<tcontext> thread_ctxt;


};

// Global MT instances
MT *block = nullptr;
MT *grain = nullptr;


void CORE_BlockedMT() {
	block = new MT(); // Dynamic allocation
	while (block->thread_active()){ // All threads are active
		if (block->pick_thread(0)){
		    // There is a thread that is not idle
			block->op(true);
		}
		block->update_halt();
	}
}

void CORE_FinegrainedMT() {
    grain = new MT(); //Dynamic allocation
    grain->thread_fix(); // Super good enough
    while (grain->thread_active()){ // All threads are active
        if (grain->pick_thread(1)){
            // There is a thread that is not idle
            grain->op(false);
        }
        grain->update_halt();
    }
}

double CORE_BlockedMT_CPI(){
	double ret = block->get_CPI();
	free(block); // Free allocation
    return ret;
}

double CORE_FinegrainedMT_CPI(){
    double ret = grain->get_CPI();
    free(grain); // Free allocation
    return ret;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
	tcontext ctxt = block->get_context(threadid);
	for (int i = 0; i < REGS_COUNT; i++){
	    context[threadid].reg[i] = ctxt.reg[i];
	}

}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
	tcontext ctxt = block->get_context(threadid);
    for (int i = 0; i < REGS_COUNT; i++){
        context[threadid].reg[i] = ctxt.reg[i];
    }
}
