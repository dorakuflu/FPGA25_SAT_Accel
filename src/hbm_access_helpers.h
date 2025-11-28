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
    unsigned int addr);


// Write single clause element to HBM
void hbm_write_clause_element(
    ap_uint<128>* hbm_clauseStore,
    ap_uint<128> mClsStore[_FPGA_MAX_LITERAL_ELEMENTS/4],
    ClsCacheData &clsCacheData,
    unsigned int addr,
    ap_uint<128> data);


#endif // HBM_ACCESS_HELPERS_H
