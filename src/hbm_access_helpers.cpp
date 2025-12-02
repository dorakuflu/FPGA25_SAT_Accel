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

    //printf("Reading addr: %h, with cache access %h", addr, index);

    ap_uint<128> get;

    if (clsCacheData.cache_bits[index][0] && clsCacheData.cache_tag[index] == addr) {
	//printf("Data read: %h", mClsStore[index]);
        get = mClsStore[index];
    }
    else{
        ap_uint<128> fetched = reg(reg(hbm_clauseStore[addr]));
    
        // writeback dirty
        if(clsCacheData.cache_bits[index][1]){
	    //printf("Wrote back addr %h from line %h", clsCacheData.cache_tag[index], index);
	    //printf("Had value of %h", mClsStore[index]);
            hbm_clauseStore[clsCacheData.cache_tag[index]] = mClsStore[index];
        }
    
        mClsStore[index] = fetched;
        clsCacheData.cache_tag[index]   = addr;
        clsCacheData.cache_bits[index][0] = 1; // valid
        clsCacheData.cache_bits[index][1] = 0; // dirty
    
        //printf("Data read: %h", fetched);
        get = fetched;
    }
    return get;
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
    //printf("Writing addr: %h, with cache access %h", addr, index);
    //printf("Data written: %h", data);

    if (clsCacheData.cache_bits[index][0] && clsCacheData.cache_tag[index] == addr) {
        // Cache hit: update cache, mark dirty
        mClsStore[index] = data;
        clsCacheData.cache_bits[index][1] = 1;
    }
    else{
        if (clsCacheData.cache_bits[index][1]) {
            // writeback dirty line to HBM
	    //printf("Wrote back addr %h from line %h", clsCacheData.cache_tag[index], index);
	    //printf("Had value of %h", mClsStore[index]);
            hbm_clauseStore[clsCacheData.cache_tag[index]] = mClsStore[index];
        }
    
        // replace data in cache
        mClsStore[index] = data;
        clsCacheData.cache_tag[index] = addr;
        clsCacheData.cache_bits[index][0] = 1; // valid
        clsCacheData.cache_bits[index][1] = 1; // dirty
    }
}

