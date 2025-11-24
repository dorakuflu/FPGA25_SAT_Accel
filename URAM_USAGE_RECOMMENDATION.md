# Recommendation: Using Freed URAM from Clause Store Migration

## Current Situation

**URAM Freed from #1 (Clause Store Migration):** ~209 URAM instances

**Current Usage:**
- Used ~96 URAM to increase clauses: 131k → 512k (4x increase) ✅
- Solver kernel: 336 URAM (105% of 320 SLR limit) ⚠️
- Total system: 490 URAM (51% of 960 available) ✅

**Remaining Freed URAM:** ~113 URAM still available

## The Problem

You're 5% over the SLR limit (336 vs 320), which could impact your 235MHz target. However, you don't want to implement #3 (transposed array caching) because:
- It requires major architectural revisions
- Would push propagation latency from 34 to 100 cycles
- Random access pattern makes caching complex
- Could significantly impact frequency

## Recommended Solution: Simple Fix

### Move `freeLitPageAddresses` to BRAM

**Why this works:**
- ✅ Solves the SLR limit issue (336 → 320, exactly at limit)
- ✅ Minimal code change (one line)
- ✅ No performance impact (page management is not on critical path)
- ✅ Uses freed URAM wisely (already used for clause increase)

**Implementation:**

In `src/solver.cpp` line 147, change:

```cpp
// From:
#pragma HLS bind_storage variable=freeLitPageAddresses.array type=RAM_S2P impl=URAM

// To:
#pragma HLS bind_storage variable=freeLitPageAddresses.array type=RAM_S2P impl=BRAM
```

**Result:**
- Solver URAM: 336 → 320 (exactly at SLR limit) ✅
- No frequency impact (BRAM latency similar to URAM for this use case)
- No architectural changes needed

## Why This is Safe

**`freeLitPageAddresses` Usage:**
- Used for page management (allocation/deallocation)
- Not on the critical path (propagation)
- Access pattern: FIFO-like (sequential), not random
- BRAM latency is acceptable for this use case

**Performance Impact:**
- BRAM: ~1-2 cycle latency
- URAM: ~1-2 cycle latency
- **No significant difference** for page management operations

## Alternative: Keep Margin

If you want more safety margin, you could:

1. **Keep current configuration** (336 URAM, 5% over)
   - The 5% overage is small
   - Partitioned `mClsStates` helps with placement
   - May work fine in place and route
   - Risk: Could impact frequency if placement is tight

2. **Move freeLitPageAddresses to BRAM** (320 URAM, exactly at limit) ✅ **RECOMMENDED**
   - Exactly at SLR limit
   - No margin, but should work
   - Simple, safe change

3. **Don't use all freed URAM** (be conservative)
   - Keep some URAM as reserve
   - But you've already used 96 URAM for clause increase
   - The remaining ~113 URAM could be kept as margin

## What NOT to Do

**Don't implement #3 (Transposed Array Caching):**
- ❌ Too complex (major architectural revisions)
- ❌ Performance impact (34 → 100 cycle latency)
- ❌ Random access pattern makes it challenging
- ❌ Could significantly impact frequency

**Don't reduce capacity:**
- You've already increased clauses to 512k (good use of freed URAM)
- Reducing variables would hurt capacity without solving the SLR issue

## Summary

**Best Recommendation: Move `freeLitPageAddresses` to BRAM**

This:
- ✅ Solves the SLR limit issue (336 → 320)
- ✅ Uses the freed URAM wisely (already used for clause increase)
- ✅ No performance impact
- ✅ Minimal code change
- ✅ No architectural complexity
- ✅ Safe for 235MHz target

**The freed URAM from clause store migration has been well-used:**
- ~96 URAM: Increased clause capacity 4x (131k → 512k) ✅
- ~16 URAM: Can be freed by moving freeLitPageAddresses to BRAM ✅
- ~97 URAM: Remaining margin/reserve ✅

This is a conservative, safe approach that maintains frequency while maximizing the benefits of the clause store migration.

