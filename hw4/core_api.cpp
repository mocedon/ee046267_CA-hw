/* 046267 Computer Architecture - Spring 2021 - HW #4 */

#include "core_api.h"
#include "sim_api.h"

#include <stdio.h>
#include <vector>


class MT {
public:
	MT() : thread(0), clck(0), inst(0), thread_num(SIM_GetThreadsNum()),
	thread_inst(thread_num, 0), thread_halt(thread_num, 0), thread_ctxt(thread_num){

		for (int i = 0; i < thread_num; i++){
			for (int j = 0; j < REGS_COUNT; j++){
				thread_ctxt[i].reg[j] = 0;
			}
		}
	}

	void kill_MT() {
	}

	void thread_fix() {
	    thread = -1; // offset the +1 on first round
	}
    bool thread_active(){
        for (int i = 0; i < thread_num; i++){
            if (thread_inst[i] > -1){
                return true;
            }
        }
        return false;
    }

    bool pick_thread(int start){
        for (int i = start; i < (thread_num + 1); i++){
            int j = (thread + i) % thread_num;
            if ((thread_inst[j] > -1) && (thread_halt[j] == 0)) {
                thread = j;
                return true;
            }
        }
        return false; //All threads are idle
    }

    void set_reg(int dst, int val){
	    thread_ctxt[thread].reg[dst] = val;
	}

	int get_reg(int reg) {
	    return thread_ctxt[thread].reg[reg];
	}

    void update_halt() {
        clck++;
        for (int i = 0; i < thread_num; i++){
            if (thread_halt[i] > 0){
                thread_halt[i]--;
            }
        }
    }

    void op(bool sw_penalty) {
	    inst++;
	    Instruction curr_inst;
	    SIM_MemInstRead((uint32_t)thread_inst[thread], &curr_inst, thread);
	    thread_inst[thread]++;
	    cmd_opcode opc = curr_inst.opcode;
	    int dst_reg = curr_inst.dst_index;
	    int src1 = curr_inst.src1_index;
	    int src2 = curr_inst.src2_index_imm;
	    bool imm = curr_inst.isSrc2Imm;

	    int addr;
	    int data;
	    switch(opc) {
	        case CMD_NOP:
	            break;
	        case CMD_ADD:
	            set_reg(dst_reg, get_reg(src1) + get_reg(src2));
	            break;
            case CMD_ADDI:
                set_reg(dst_reg, get_reg(src1) + src2);
                break;
	        case CMD_SUB:
                set_reg(dst_reg, get_reg(src1) - get_reg(src2));
	            break;
	        case CMD_SUBI:
                set_reg(dst_reg, get_reg(src1) - src2);
	            break;
	        case CMD_LOAD:
	            addr = get_reg(src1) + (imm ? src2 : get_reg(src2));
	            SIM_MemDataRead((uint32_t)addr, &data);
	            set_reg(dst_reg, data);
	            thread_halt[thread] = SIM_GetLoadLat() + 1 ; // including this clock cycle
	            if (sw_penalty){
	                for (int i = 0; i < SIM_GetSwitchCycles(); i++){
	                    update_halt();
	                }
	            }
	            break;
	        case CMD_STORE:
                addr = get_reg(dst_reg) + (imm ? src2 : get_reg(src2));
                SIM_MemDataWrite((uint32_t)addr, get_reg(src1));
                thread_halt[thread] = SIM_GetStoreLat() + 1; // including this clock cycle
                if (sw_penalty){
                    for (int i = 0; i < SIM_GetSwitchCycles(); i++){
                        update_halt();
                    }
                }
	            break;
	        case CMD_HALT:
	            thread_inst[thread] = -1;
                if (sw_penalty){
                    for (int i = 0; i < SIM_GetSwitchCycles(); i++){
                        update_halt();
                    }
                }
	            break;
	    }

	}

    tcontext get_context(int id) {
	    return thread_ctxt[id];
	}

    double get_CPI() {
	    return ((double)clck) / ((double)inst);
	}



private:

	int thread;
	int thread_num;
	std::vector<int> thread_inst;
	std::vector<int> thread_halt;

	std::vector<tcontext> thread_ctxt;
	int clck;
	int inst;
};

MT block;
MT grain;

void CORE_BlockedMT() {
	block = MT();

	while (block.thread_active()){
		if (block.pick_thread(0)){
			block.op(true);
		}
		block.update_halt();
	}
}

void CORE_FinegrainedMT() {
    grain = MT();
    grain.thread_fix();
    while (grain.thread_active()){
        if (grain.pick_thread(1)){
            grain.op(false);
        }
        grain.update_halt();
    }
}

double CORE_BlockedMT_CPI(){
	double ret = block.get_CPI();
	block.kill_MT();
    return ret;
}

double CORE_FinegrainedMT_CPI(){
    double ret = grain.get_CPI();
    grain.kill_MT();
    return ret;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
	tcontext ctxt = block.get_context(threadid);
	for (int i = 0; i < REGS_COUNT; i++){
	    context[threadid].reg[i] = ctxt.reg[i];
	}

}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
	tcontext ctxt = block.get_context(threadid);
    for (int i = 0; i < REGS_COUNT; i++){
        context[threadid].reg[i] = ctxt.reg[i];
    }
}
