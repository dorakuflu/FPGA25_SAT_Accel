#ifndef HBM_ACCESS_HELPERS_H
#define HBM_ACCESS_HELPERS_H

#include "fpga_solver.h"
#include "data_structures.h"
#include <ap_int.h>
#include <ap_utils.h>

// Helper functions for efficient HBM access

// Read single clause element from HBM
inline ap_uint<128> hbm_read_clause_element(
    ap_uint<128>* hbm_clauseStore,
    unsigned int addr) {
    #pragma HLS inline
    
    // Direct read from HBM - AXI interface is declared in parent function
    // Use reg() to ensure proper pipelining
    return reg(reg(hbm_clauseStore[addr]));
}

// Write single clause element to HBM
inline void hbm_write_clause_element(
    ap_uint<128>* hbm_clauseStore,
    unsigned int addr,
    ap_uint<128> data) {
    #pragma HLS inline
    
    // Direct write to HBM - AXI interface is declared in parent function
    hbm_clauseStore[addr] = data;
}

#endif // HBM_ACCESS_HELPERS_H
