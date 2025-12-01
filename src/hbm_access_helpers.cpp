#include "hbm_access_helpers.h"

// Helper functions for efficient HBM access
void cache_find(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr,
    unsigned int &found_addr,
    bool &hit)
{
    #pragma HLS inline off

    LOOKUP_LOOP:
    for (int i = 0; i < CACHE_SIZE; i++) {
	// parallelizes the lookup
        // #pragma HLS unroll
        if (clsCacheData.cache_bits[i][0] && clsCacheData.cache_tag[i] == addr) {
	    found_addr = i;
            hit = true;
	    break;
        }
    }
}

// Read single clause element from HBM
ap_uint<128> hbm_read_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr)
{
    // fully associative search
    // cache_find(hbm_clauseStore, mClsStore, clsCacheData, addr, found_addr, hit);

    // direct mapped search
    unsigned int index = addr % CACHE_SIZE;

    if (clsCacheData.cache_bits[index][0] && clsCacheData.cache_tag[index] == addr) {
        return mClsStore[index];
    }

    ap_uint<128> fetched = reg(reg(hbm_clauseStore[addr]));

    // writeback dirty
    if(clsCacheData.cache_bits[index][1]){
        hbm_clauseStore[clsCacheData.cache_tag[index]] = mClsStore[index];
    }

    mClsStore[index] = fetched;
    clsCacheData.cache_tag[index]   = addr;
    clsCacheData.cache_bits[index][0] = 1;
    clsCacheData.cache_bits[index][1] = 0;

    return fetched;
}


// Write single clause element to HBM
void hbm_write_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr,
    ap_uint<128> data)
{
    bool hit = false;

    // fully associative
    // cache_find(hbm_clauseStore, mClsStore, clsCacheData, addr, found_addr, hit);

    // direct mapped search
    unsigned int index = addr % CACHE_SIZE;

    if (clsCacheData.cache_bits[index][0] && clsCacheData.cache_tag[index] == addr) {
        // Cache hit: update cache, mark dirty
        mClsStore[index] = data;
        clsCacheData.cache_bits[index][1] = 1;
        return;
    }

    if (clsCacheData.cache_bits[index][1]) {
        // writeback dirty line to HBM
        hbm_clauseStore[clsCacheData.cache_tag[index]] = mClsStore[index];
    }

    // replace data in cache
    mClsStore[index] = data;
    clsCacheData.cache_tag[index] = addr;
    clsCacheData.cache_bits[index][0] = 1; // valid
    clsCacheData.cache_bits[index][1] = 1; // dirty
}

