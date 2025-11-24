# HBM Implementation Summary - Clause Store Direct Access (#1)

## Overview
This implementation migrates the clause store from URAM to direct HBM access, as described in optimization #1 of the research paper's future work section.

## Changes Made

### 1. Removed URAM Clause Store Array
- **File**: `src/clause_store_handler.cpp`
- **Change**: Removed the `mClsStore` URAM array that was previously used to cache clause data
- **Impact**: Frees significant URAM resources that can be used for other purposes

### 2. Removed Initial Copy Operation
- **File**: `src/clause_store_handler.cpp`
- **Change**: Removed `copyCls()` function and `copy_cls_data()` function that copied data from HBM to URAM
- **Impact**: Reduces initialization time and eliminates redundant data copying

### 3. Modified Functions to Access HBM Directly

#### Modified Functions:
- `sendLoop()`: Now reads directly from HBM via `hbm_read_clause_element()`
- `sendData()`: Updated to pass HBM pointer instead of URAM array
- `sendData_dataflow()`: Updated to pass HBM pointer
- `saveData()`: Now writes directly to HBM via `hbm_write_clause_element()`
- `deleteClauses()`: Now reads directly from HBM
- `delete_wrapper()`: Updated to pass HBM pointer
- `deleteClauses_wrapper()`: Updated to pass HBM pointer

### 4. Created HBM Access Helper Functions
- **File**: `src/hbm_access_helpers.h` and `src/hbm_access_helpers.cpp`
- **Functions**:
  - `hbm_read_clause_element()`: Reads a single 128-bit element from HBM
  - `hbm_write_clause_element()`: Writes a single 128-bit element to HBM
  - `hbm_read_clause_burst()`: Burst read for future optimization
  - `hbm_write_clause_burst()`: Burst write for future optimization

## Benefits

1. **URAM Resource Savings**: 
   - Removed `mClsStore[_FPGA_MAX_LITERAL_ELEMENTS/4]` URAM array
   - This is a significant amount of URAM that can now be used for other purposes

2. **Reduced Initialization Time**:
   - No longer need to copy clause data from HBM to URAM at startup
   - Only metadata (`mCmd`) is copied, which is much smaller

3. **Scalability**:
   - Clause store size is no longer limited by URAM capacity
   - Can support larger clause stores limited only by HBM capacity

4. **Performance**:
   - Clause learning and minimization operations are not on the critical path
   - HBM latency is acceptable for these operations per the research paper

## Performance Considerations

- **Latency**: HBM access has higher latency than URAM (~40 cycles vs ~2 cycles)
- **Acceptability**: This is acceptable because clause operations are not on the critical path
- **Burst Optimization**: The helper functions include burst read/write functions for future optimization if needed

## Testing Recommendations

1. **Functional Testing**: Verify all clause operations work correctly:
   - Clause saving
   - Clause reading/sending
   - Clause deletion
   - Clause learning

2. **Performance Testing**: Compare against baseline:
   - Measure clause operation latency
   - Verify overall solver performance is not significantly impacted
   - Check HBM bandwidth utilization

3. **Resource Utilization**: Verify URAM savings:
   - Check synthesis reports for URAM usage reduction
   - Confirm freed resources can be used elsewhere

## Files Modified

1. `src/clause_store_handler.cpp` - Main implementation changes
2. `src/hbm_access_helpers.h` - New helper function declarations
3. `src/hbm_access_helpers.cpp` - New helper function implementations

## Compilation Notes

- No changes needed to `k2k.cfg` - HBM mapping remains the same (HBM[13])
- AXI interface pragmas remain in `clause_store_handler()` function
- All existing stream interfaces remain unchanged

