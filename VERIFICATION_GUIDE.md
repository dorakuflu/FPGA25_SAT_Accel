# Verification Guide for HBM Clause Store Migration

## Current Status: ✅ Tests Passing

Your tests are passing, which is excellent! Now let's verify everything is working correctly and set proper expectations.

## Performance Expectations

### ⚠️ Important: Do NOT Expect Performance Increase Yet

**For Implementation #1 (Clause Store HBM Direct Access):**
- **Expected**: Slight performance decrease or similar performance
- **Reason**: HBM has higher latency (~40 cycles) than URAM (~2 cycles)
- **Acceptable**: Yes, because clause operations are NOT on the critical path
- **Benefit**: Frees URAM resources (the main goal)

### What You Should See:

1. **Correctness**: ✅ Same SAT/UNSAT results (you're seeing this)
2. **Performance**: Similar or slightly slower (acceptable)
3. **Resource Savings**: URAM usage should decrease (check synthesis reports)

## Verification Steps

### 1. Correctness Verification ✅

**Already Passing:**
- Tests show "Test passed" 
- SAT/UNSAT results match expected values
- No runtime errors

**Additional Checks:**
```bash
# Run a few more test cases to ensure consistency
./testcases_sim.sh hw_emu
```

### 2. Performance Comparison

**Compare Cycle Counters:**

Look at the `CYCLE COUNTERS` output:
```
CYCLE COUNTERS (COPY, PQ-FIND, BRANCH, LEARN, LEARN_MIN, SAVE, RESIZE, BACKTRACK, DELETE)
```

**Key Metrics to Compare:**

1. **SAVE counter [5]**: Clause saving operations
   - May increase slightly due to HBM latency
   - Should still be acceptable

2. **LEARN counter [3]**: Clause learning
   - May increase slightly
   - Not on critical path

3. **Overall execution time**: 
   - Should be similar or slightly higher
   - The paper states this is acceptable

**How to Compare:**
```bash
# Run baseline (if you have it) and new version
# Compare the cycle counters and execution times
```

### 3. Resource Utilization Verification

**⚠️ Important:** Hardware emulation (`hw_emu`) does NOT provide detailed resource utilization. You need to run synthesis.

**Quick Method - HLS Synthesis:**

```bash
# Run synthesis (takes 30min-2hrs, but faster than full implementation)
./runCompile.sh hls
```

This generates synthesis reports in `FPGArpt/` directory.

**Check URAM Usage:**

```bash
# View URAM utilization
grep -i uram FPGArpt/clause_store_handler_csynth.rpt

# Or view full report
less FPGArpt/clause_store_handler_csynth.rpt
```

**Expected Results:**
- **Before**: ~80% URAM usage (as you mentioned)
- **After**: Should be significantly lower (mClsStore removed)

**Full Implementation (Optional):**

After building for hardware (`./runCompile.sh hw`), check:

1. **URAM Usage**:
   - Should see reduction in URAM usage
   - Look for `clause_store_handler` kernel URAM usage
   - Compare before/after if possible

2. **HBM Usage**:
   - Verify HBM[13] is being used for clause store
   - Check bandwidth utilization

**Location of Reports:**
```
_x/workload-clause_store_handler-hw/clause_store_handler/solution/syn/report/
```

### 4. Functional Verification

**Verify All Clause Operations Work:**

From your test output, check:
- ✅ Clause learning: `TRIP COUNT FOR LEARNING` shows activity
- ✅ Clause operations: No errors in execution
- ✅ Results match expected SAT/UNSAT

**Test with Different Instance Sizes:**
```bash
# Test with larger instances to verify scalability
./test.hw_emu.out workload-hw_emu.xclbin ../configuration.json \
  ../../SAT_test_cases/sat/larger_instance.dimacs result.txt 1
```

## Expected Results Summary

### ✅ What Should Work:
1. All tests pass (SAT/UNSAT correct)
2. No runtime errors
3. URAM resources freed
4. Can handle larger clause stores

### ⚠️ What to Expect:
1. **Performance**: Similar or slightly slower (acceptable)
2. **SAVE cycles**: May increase slightly
3. **LEARN cycles**: May increase slightly

### ❌ What NOT to Expect:
1. **Performance improvement** - Not the goal of #1
2. **Faster clause operations** - HBM is slower than URAM
3. **Overall speedup** - That comes from using freed URAM elsewhere

## When Will You See Performance Benefits?

Performance benefits will come from:

1. **Using Freed URAM**:
   - Can now use URAM for other optimizations
   - Support larger variable/clause counts
   - Implement other features

2. **Future Optimizations** (not implemented yet):
   - #2: Remove position arrays (frees 384 URAM)
   - #3: Cache for transposed array
   - These would enable 128k variables, 1024k clauses

3. **Scalability**:
   - Can now handle larger instances
   - Not limited by URAM capacity

## Monitoring During Execution

**Watch for:**
- Execution time (should be similar)
- Cycle counters (SAVE, LEARN may increase)
- Memory access patterns
- No crashes or hangs

**Red Flags:**
- Significant performance degradation (>20%)
- Incorrect SAT/UNSAT results
- Runtime errors
- Hangs or timeouts

## Next Steps

1. ✅ **Correctness**: Verified (tests passing)
2. ⏳ **Performance**: Compare cycle counters with baseline
3. ⏳ **Resources**: Check synthesis reports for URAM savings
4. ⏳ **Scalability**: Test with larger instances

## Summary

**Your implementation is working correctly!** The tests passing confirms:
- Functionality is correct
- No breaking changes
- System is stable

**Performance expectations:**
- Similar or slightly slower is expected and acceptable
- The benefit is resource savings, not speed
- Future optimizations will use the freed resources

You're on the right track! The HBM migration is working as intended.

