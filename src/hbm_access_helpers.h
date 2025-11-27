#ifndef HBM_ACCESS_HELPERS_H
#define HBM_ACCESS_HELPERS_H

#include "fpga_solver.h"
#include "data_structures.h"
#include <ap_int.h>
#include <ap_utils.h>

struct ClsCacheData {
    unsigned int cache_tag[_FPGA_MAX_LITERAL_ELEMENTS/4];

    ap_uint<2> cache_bits[_FPGA_MAX_LITERAL_ELEMENTS/4];

    //round robin eviction ptr
    unsigned int repl_idx = 0;

};

// Helper functions for efficient HBM access

// Read single clause element from HBM
ap_uint<128> hbm_read_clause_element(
    ap_uint<128>* hbm_clauseStore,
    ap_uint<128> mClsStore[_FPGA_MAX_LITERAL_ELEMENTS/4],
    ClsCacheData &clsCacheData,
    unsigned int addr)
{
    #pragma HLS inline off

    const int CACHE_SIZE = _FPGA_MAX_LITERAL_ELEMENTS/4;

    bool hit = false;
    unsigned int found_addr = 0;

    LOOKUP_LOOP:
    for (int i = 0; i < CACHE_SIZE; i++) {
	// parallelizes the lookup
        #pragma HLS unroll
        if (clsCacheData.cache_bits[i][0] && clsCacheData.cache_tag[i] == addr) {
	    found_addr = i;
            hit = true;
        }
    }

    if (hit) {
        return mClsStore[found_addr];
    }

    ap_uint<128> fetched = reg(reg(hbm_clauseStore[addr]));

    // writeback dirty
    if(clsCacheData.cache_bits[clsCacheData.repl_idx][1]){
	    hbm_clauseStore[addr] = mClsStore[clsCacheData.repl_idx];
    }

    mClsStore[clsCacheData.repl_idx] = fetched;
    clsCacheData.cache_tag[clsCacheData.repl_idx]   = addr;
    clsCacheData.cache_bits[clsCacheData.repl_idx][0] = 1;
    clsCacheData.cache_bits[clsCacheData.repl_idx][1] = 0;

    // round robin style eviction ptr
    clsCacheData.repl_idx = (clsCacheData.repl_idx + 1);
    if (clsCacheData.repl_idx == CACHE_SIZE) clsCacheData.repl_idx = 0;


    return fetched;
}


// Write single clause element to HBM
void hbm_write_clause_element(
    ap_uint<128>* hbm_clauseStore,
    ap_uint<128> mClsStore[_FPGA_MAX_LITERAL_ELEMENTS/4],
    ClsCacheData &clsCacheData,
    unsigned int addr,
    ap_uint<128> data) {

    #pragma HLS inline off

    const int CACHE_SIZE = _FPGA_MAX_LITERAL_ELEMENTS/4;

    bool hit = false;
    unsigned int found_addr = 0;

    LOOKUP_LOOP:
    for (int i = 0; i < CACHE_SIZE; i++) {
	// parallelizes the lookup
        #pragma HLS unroll
        if (clsCacheData.cache_bits[i][0] && clsCacheData.cache_tag[i] == addr) {
	    found_addr = i;
            hit = true;
        }
    }

    if (hit) {
        // Cache hit: update cache, mark dirty
        mClsStore[found_addr] = data;
        clsCacheData.cache_bits[found_addr][1] = 1;
        return;
    }

    if (clsCacheData.cache_bits[clsCacheData.repl_idx][1]) {
        // writeback dirty line to HBM
        hbm_clauseStore[clsCacheData.cache_tag[clsCacheData.repl_idx]] = mClsStore[clsCacheData.repl_idx];
    }

    // replace data in cache
    mClsStore[clsCacheData.repl_idx] = data;
    clsCacheData.cache_tag[clsCacheData.repl_idx] = addr;
    clsCacheData.cache_bits[clsCacheData.repl_idx][0] = 1; // valid
    clsCacheData.cache_bits[clsCacheData.repl_idx][1] = 1; // dirty

    // round robin style eviction ptr
    clsCacheData.repl_idx = (clsCacheData.repl_idx + 1);
    if (clsCacheData.repl_idx == CACHE_SIZE) clsCacheData.repl_idx = 0;
}


#endif // HBM_ACCESS_HELPERS_H
