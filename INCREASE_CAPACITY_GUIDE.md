# Guide: Using Freed URAM to Increase Capacity

## Overview

With ~209 URAM instances freed from the clause store migration, you can now increase the design's capacity to support larger SAT instances. According to the paper, this should allow supporting **128k variables and 1024k clauses** (up from 32k and 131k).

## Current vs Target Capacity

| Parameter | Current | Target | Multiplier |
|-----------|---------|--------|------------|
| **Variables** | 32,768 (32k) | 131,072 (128k) | **4x** |
| **Clauses** | 131,072 (131k) | 1,048,576 (1024k) | **8x** |
| **Literal Elements** | 1,048,576 | Increase proportionally | ~4x |

## URAM Usage Analysis

### Current URAM Usage (394 instances)

| Component | URAM | Notes |
|-----------|------|-------|
| mLitStore (transposed) | 128 | 65536 words × 512 bits |
| mlmd (literal metadata) | 64 | 32768 literals |
| mClsStates | 32 | 131072 clauses / 8 partitions |
| freeLitPageAddresses | 16 | Page management |
| mPriorityQueue | 32 | 32768 literals × 2 |
| mPositioning | 8 | 32768 literals |
| mCmd (clause metadata) | 16 | 131072 clauses |
| Position arrays | 64 | Not removed yet |
| Other | 34 | Various |

### URAM Requirements for Capacity Increase

**To support 128k variables (4x increase):**

| Component | Current | New | URAM Increase |
|-----------|---------|-----|---------------|
| mLitStore | 128 | **512** | +384 |
| mlmd | 64 | **256** | +192 |
| mPriorityQueue | 32 | **128** | +96 |
| mPositioning | 8 | **32** | +24 |
| mAnswerStack | 0 (BRAM) | 0 | 0 |
| unitByCls | 0 (auto) | 0 | 0 |
| **Subtotal** | | | **+696 URAM** |

**To support 1024k clauses (8x increase):**

| Component | Current | New | URAM Increase |
|-----------|---------|-----|---------------|
| mClsStates | 32 | **256** | +224 |
| mCmd | 16 | **128** | +112 |
| freeClsID | 16 | **128** | +112 |
| freeClsPageAddresses | 16 | **128** | +112 |
| **Subtotal** | | | **+560 URAM** |

**Total Additional URAM Needed: ~1,256 instances**

## Available URAM

- **Freed from clause store**: ~209 URAM
- **Currently available**: 566 URAM (960 - 394)
- **Total available**: ~775 URAM

**⚠️ Challenge**: Need ~1,256 URAM but only have ~775 available

## Solution: Incremental Approach

Since we don't have enough URAM for the full increase yet, we can:

### Option 1: Increase Variables Only (More Feasible)

**Target: 64k variables (2x increase)**

| Component | Current | New | URAM Needed |
|-----------|---------|-----|-------------|
| mLitStore | 128 | 256 | +128 |
| mlmd | 64 | 128 | +64 |
| mPriorityQueue | 32 | 64 | +32 |
| mPositioning | 8 | 16 | +8 |
| **Total** | | | **+232 URAM** |

✅ **Feasible**: 232 < 775 available

### Option 2: Increase Clauses Only

**Target: 512k clauses (4x increase)**

| Component | Current | New | URAM Needed |
|-----------|---------|-----|-------------|
| mClsStates | 32 | 128 | +96 |
| mCmd | 16 | 64 | +48 |
| freeClsID | 16 | 64 | +48 |
| freeClsPageAddresses | 16 | 64 | +48 |
| **Total** | | | **+240 URAM** |

✅ **Feasible**: 240 < 775 available

### Option 3: Balanced Increase (Recommended)

**Target: 64k variables + 512k clauses**

- Variables: 2x increase → +232 URAM
- Clauses: 4x increase → +240 URAM
- **Total**: +472 URAM

✅ **Feasible**: 472 < 775 available

## Implementation Steps

### Step 1: Update Capacity Limits

Modify `src/fpga_solver.h`:

```cpp
// Current:
#define _FPGA_MAX_LITERALS (8192*4)      // 32,768
#define _FPGA_MAX_CLAUSES 131072         // 131k

// Option 1: 64k variables, 512k clauses
#define _FPGA_MAX_LITERALS (8192*8)      // 65,536 (64k)
#define _FPGA_MAX_CLAUSES 524288         // 524k

// Option 2: Full target (requires more URAM or removing position arrays)
#define _FPGA_MAX_LITERALS (8192*16)     // 131,072 (128k)
#define _FPGA_MAX_CLAUSES 1048576        // 1,048,576 (1024k)
```

### Step 2: Update Literal Elements

Increase `_FPGA_MAX_LITERAL_ELEMENTS` proportionally:

```cpp
// Current:
#define _FPGA_MAX_LITERAL_ELEMENTS (256*4096)  // 1,048,576

// For 2x variables:
#define _FPGA_MAX_LITERAL_ELEMENTS (512*4096)  // 2,097,152

// For 4x variables:
#define _FPGA_MAX_LITERAL_ELEMENTS (1024*4096)  // 4,194,304
```

### Step 3: Recompile and Verify

```bash
# Rebuild with new limits
./runCompile.sh hls

# Check URAM usage in reports
grep -i uram FPGArpt/*_csynth.rpt | grep -i total
```

## Expected Benefits

### Capacity Increase

| Scenario | Variables | Clauses | Instances Supported |
|----------|-----------|---------|---------------------|
| **Current** | 32k | 131k | 205/401 (51%) |
| **64k + 512k** | 64k | 512k | ~240/401 (60%) |
| **128k + 1024k** | 128k | 1024k | 270/401 (67%) |

### Performance Impact

- **More instances solvable**: Can handle larger SAT competition instances
- **No performance degradation**: Same per-instance performance
- **Better scalability**: Support for more complex problems

## Constraints and Considerations

### 1. URAM Availability

- **Current**: 566 URAM available
- **For 64k+512k**: Need 472 URAM ✅
- **For 128k+1024k**: Need 1,256 URAM ❌ (need to remove position arrays first)

### 2. Place and Route Challenges

The paper mentions:
> "place and route becomes challenging as the transposed array crosses multiple SLRs, impacting the frequency"

**Considerations:**
- Larger arrays may cross SLR boundaries
- May need to adjust partitioning
- Frequency may decrease slightly

### 3. Memory Bandwidth

- Larger arrays = more memory bandwidth needed
- HBM bandwidth should be sufficient
- Monitor bandwidth utilization

## Recommended Approach

### Phase 1: Moderate Increase (Now)

1. **Increase to 64k variables + 512k clauses**
   - Uses ~472 URAM (within available)
   - 2x variable capacity, 4x clause capacity
   - Low risk, immediate benefit

### Phase 2: Full Increase (After Removing Position Arrays)

1. **Remove position arrays** (frees 64 URAM)
2. **Increase to 128k variables + 1024k clauses**
   - Total available: ~775 + 64 = 839 URAM
   - Still need ~1,256 URAM
   - May need to optimize other structures

### Phase 3: Optimize Further

1. **Review other URAM usage**
2. **Consider moving more to HBM** (if acceptable)
3. **Optimize array sizes** for better fit

## Quick Start: Increase to 64k + 512k

This is the safest immediate improvement:

1. Edit `src/fpga_solver.h`:
   ```cpp
   #define _FPGA_MAX_LITERALS (8192*8)      // 65,536
   #define _FPGA_MAX_CLAUSES 524288         // 524k
   #define _FPGA_MAX_LITERAL_ELEMENTS (512*4096)  // 2,097,152
   ```

2. Rebuild:
   ```bash
   ./runCompile.sh hls
   ```

3. Verify URAM usage stays within limits

4. Test with larger instances

## Summary

**Immediate Action:**
- ✅ Increase to **64k variables + 512k clauses** (feasible with current URAM)
- Uses ~472 of 775 available URAM
- Provides 2x variable and 4x clause capacity

**Future Action:**
- Remove position arrays (frees 64 more URAM)
- Then consider 128k + 1024k (may need further optimization)

The freed URAM from clause store migration enables significant capacity increases!

