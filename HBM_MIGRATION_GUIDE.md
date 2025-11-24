# HBM Migration Guide for SAT-Accel - Clause Store Direct Access

This guide outlines the implementation strategy for migrating the clause store from URAM to direct HBM access, following the future work recommendations from the research paper.

## Overview

This migration focuses on **Clause Store HBM Direct Access**: Access clause store directly from HBM instead of copying to URAM.

## Current Architecture

### Memory Usage
- **Clause Store** (`mClsStore`): Currently copied from HBM[13] to URAM at initialization

### Critical Path Analysis
- **Clause Learning/Minimization**: Not on critical path, can tolerate HBM latency

## Implementation Strategy

### Clause Store HBM Direct Access

**Current Flow:**
```
HBM[13] → copy_cls_data() → mClsStore (URAM) → clause operations
```

**New Flow:**
```
HBM[13] → direct access via AXI → clause operations
```

**Changes Required:**
- Remove `mClsStore` URAM array from `clause_store_handler.cpp`
- Access `clauseStore` pointer directly from HBM via AXI interface
- Add burst read/write optimizations for clause operations
- Update `sendData` and `saveData` functions to use HBM directly

**Benefits:**
- Frees URAM for other uses
- Clause operations are not on critical path, so HBM latency is acceptable
- Reduces initialization time

## Implementation Steps

### Phase 1: Clause Store Migration ✅ COMPLETED
1. ✅ Modified `clause_store_handler.cpp` to access HBM directly
2. ✅ Removed `mClsStore` URAM array
3. ✅ Updated all clause store access functions
4. ⏳ Test clause learning and minimization operations (pending)

## Expected Benefits

1. **URAM Resource Savings**: 
   - Free URAM from clause store
   - Can be used for other purposes or to support larger designs

2. **Performance**:
   - Clause operations: Slight latency increase (acceptable, not on critical path)
   - Reduced initialization time (no copy operation)

3. **Scalability**:
   - Clause store size is no longer limited by URAM capacity
   - Can support larger clause stores limited only by HBM capacity

## Configuration Changes

### Memory Mapping (k2k.cfg)
- Keep existing HBM mappings (HBM[13] for clause store)
- Ensure HBM channels are properly allocated

### Compilation Flags
- No changes needed to compilation flags
- AXI interface pragmas remain in `clause_store_handler()` function

## Testing Strategy

1. **Functional Testing**: Verify correctness with existing test cases
   - Clause saving
   - Clause reading/sending
   - Clause deletion
   - Clause learning

2. **Performance Testing**: Compare against baseline implementation
   - Measure clause operation latency
   - Verify overall solver performance is not significantly impacted
   - Check HBM bandwidth utilization

3. **Resource Utilization**: Verify URAM savings
   - Check synthesis reports for URAM usage reduction
   - Confirm freed resources can be used elsewhere

## Notes

- HBM access has higher latency than URAM (~40 cycles vs ~2 cycles)
- This is acceptable because clause operations are not on the critical path
- Burst read/write functions are available in helper functions for future optimization if needed
