#include "hbm_access_helpers.h"

// Configuration for set-associative cache
#define NUM_WAYS 4  // Number of ways (associativity)
#define NUM_SETS (CACHE_SIZE / NUM_WAYS)  // Number of sets

// Read single clause element from HBM with set-associative cache
ap_uint<128> hbm_read_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr)
{
    // Extract set index from address
    unsigned int set_index = addr % NUM_SETS;
    unsigned int base_index = set_index * NUM_WAYS;
    
    // Search within the set for a hit
    bool hit = false;
    unsigned int hit_way = 0;
    
    SEARCH_WAYS:
    for (unsigned int way = 0; way < NUM_WAYS; way++) {
        #pragma HLS unroll
        unsigned int cache_index = base_index + way;
        if (clsCacheData.cache_bits[cache_index][0] && 
            clsCacheData.cache_tag[cache_index] == addr) {
            hit = true;
            hit_way = way;
            break;
        }
    }
    
    ap_uint<128> get;
    
    if (hit) {
        // Cache hit
        unsigned int cache_index = base_index + hit_way;
        get = mClsStore[cache_index];
    }
    else {
        // Cache miss - fetch from HBM
        ap_uint<128> fetched = reg(reg(hbm_clauseStore[addr]));
        
        // Find replacement candidate (simple: first invalid or LRU)
        unsigned int replace_way = 0;
        
        FIND_REPLACEMENT:
        for (unsigned int way = 0; way < NUM_WAYS; way++) {
            #pragma HLS unroll
            unsigned int cache_index = base_index + way;
            if (!clsCacheData.cache_bits[cache_index][0]) {
                replace_way = way;
                break;
            }
        }
        // If all valid, replace_way stays 0 (could add LRU counter here)
        
        unsigned int victim_index = base_index + replace_way;
        
        // Writeback dirty line if necessary
        if (clsCacheData.cache_bits[victim_index][1]) {
            hbm_clauseStore[clsCacheData.cache_tag[victim_index]] = mClsStore[victim_index];
        }
        
        // Install new line in cache
        mClsStore[victim_index] = fetched;
        clsCacheData.cache_tag[victim_index] = addr;
        clsCacheData.cache_bits[victim_index][0] = 1; // valid
        clsCacheData.cache_bits[victim_index][1] = 0; // dirty
        
        get = fetched;
    }
    
    return get;
}

// Write single clause element to HBM with set-associative cache
void hbm_write_clause_element(
    ap_uint<128> *hbm_clauseStore,
    ap_uint<128> *mClsStore,
    ClsCacheData &clsCacheData,
    unsigned int addr,
    ap_uint<128> data)
{
    // Extract set index from address
    unsigned int set_index = addr % NUM_SETS;
    unsigned int base_index = set_index * NUM_WAYS;
    
    // Search within the set for a hit
    bool hit = false;
    unsigned int hit_way = 0;
    
    SEARCH_WAYS_WRITE:
    for (unsigned int way = 0; way < NUM_WAYS; way++) {
        #pragma HLS unroll
        unsigned int cache_index = base_index + way;
        if (clsCacheData.cache_bits[cache_index][0] && 
            clsCacheData.cache_tag[cache_index] == addr) {
            hit = true;
            hit_way = way;
            break;
        }
    }
    
    if (hit) {
        // Cache hit: update cache, mark dirty
        unsigned int cache_index = base_index + hit_way;
        mClsStore[cache_index] = data;
        clsCacheData.cache_bits[cache_index][1] = 1;
    }
    else {
        // Cache miss - need to allocate line
        
        // Find replacement candidate (simple: first invalid or LRU)
        unsigned int replace_way = 0;
        
        FIND_REPLACEMENT_WRITE:
        for (unsigned int way = 0; way < NUM_WAYS; way++) {
            #pragma HLS unroll
            unsigned int cache_index = base_index + way;
            if (!clsCacheData.cache_bits[cache_index][0]) {
                replace_way = way;
                break;
            }
        }
        // If all valid, replace_way stays 0 (could add LRU counter here)
        
        unsigned int victim_index = base_index + replace_way;
        
        // Writeback dirty line if necessary
        if (clsCacheData.cache_bits[victim_index][1]) {
            hbm_clauseStore[clsCacheData.cache_tag[victim_index]] = mClsStore[victim_index];
        }
        
        // Install new line in cache and mark dirty
        mClsStore[victim_index] = data;
        clsCacheData.cache_tag[victim_index] = addr;
        clsCacheData.cache_bits[victim_index][0] = 1; // valid
        clsCacheData.cache_bits[victim_index][1] = 1; // dirty
    }
}
