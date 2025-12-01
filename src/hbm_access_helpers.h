#ifndef HBM_ACCESS_HELPERS_H
#define HBM_ACCESS_HELPERS_H

#include "fpga_solver.h"
#include "data_structures.h"
#include <ap_int.h>
#include <ap_utils.h>

constexpr int CACHE_SIZE = _HOST_MAX_CLAUSE_ELEMENTS;
constexpr int INDEX_BITS = __builtin_ctz(CACHE_SIZE);

struct ClsCacheData {
    unsigned int cache_tag[_HOST_MAX_CLAUSE_ELEMENTS ];

    ap_uint<2> cache_bits[_HOST_MAX_CLAUSE_ELEMENTS ];

    //round robin eviction ptr
    unsigned int repl_idx = 0;

};

// Helper functions for efficient HBM access

// Read single clause element from HBM
ap_uint<128> hbm_read_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr);


// Write single clause element to HBM
void hbm_write_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr,
    ap_uint<128> data);


#endif // HBM_ACCESS_HELPERS_H
