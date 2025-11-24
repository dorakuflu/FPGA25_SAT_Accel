# How to Check URAM Resource Utilization

## Overview

Hardware emulation (`hw_emu`) does **NOT** provide detailed resource utilization reports. You need to run synthesis for hardware (`hw`) to get resource reports.

## Quick Method: Run HLS Synthesis

The fastest way to get resource reports without full implementation:

```bash
# This runs synthesis (not full implementation) - takes 30min to 2 hours
./runCompile.sh hls
```

This will:
1. Synthesize all kernels for hardware
2. Copy synthesis reports to `FPGArpt/` directory
3. Show URAM usage in the reports

## Where to Find URAM Reports

After running `./runCompile.sh hls`, check:

### For clause_store_handler kernel:
```bash
# View the utilization report
cat FPGArpt/clause_store_handler_csynth.rpt | grep -i uram -A 5 -B 5

# Or open the full report
less FPGArpt/clause_store_handler_csynth.rpt
```

### Direct path to reports:
```bash
# Synthesis reports are also in:
_x/workload-clause_store_handler-hw/clause_store_handler/clause_store_handler/solution/syn/report/

# Key files:
# - clause_store_handler_csynth.rpt (utilization summary)
# - clause_store_handler_utilization.rpt (detailed utilization)
```

## What to Look For

### In the Utilization Report:

1. **URAM Usage**:
   - Look for "URAM" in the resource utilization section
   - Compare "Used" vs "Available"
   - Should see reduction from ~80% to lower percentage

2. **Memory Resources**:
   - Check for `mClsStore` array (should be gone)
   - Verify HBM interfaces are being used

### Example Search Commands:

```bash
# Find URAM usage
grep -i "uram" FPGArpt/clause_store_handler_csynth.rpt

# Find memory arrays
grep -i "memory\|array\|ram" FPGArpt/clause_store_handler_csynth.rpt | grep -i "clause"

# Find HBM/global memory usage
grep -i "global\|hbm\|m_axi" FPGArpt/clause_store_handler_csynth.rpt
```

## Comparing Before/After

If you have previous synthesis reports:

```bash
# Compare URAM usage
# Before (if you have old reports):
grep -i "uram" old_reports/clause_store_handler_csynth.rpt

# After (new reports):
grep -i "uram" FPGArpt/clause_store_handler_csynth.rpt
```

## Expected Results

### Before HBM Migration:
- URAM usage: ~80% (as you mentioned)
- `mClsStore` array in URAM
- Large URAM allocation for clause store

### After HBM Migration:
- URAM usage: Should be **significantly lower**
- `mClsStore` array removed
- HBM interfaces active for clause store
- More URAM available for other uses

## Alternative: Check During Build

You can also monitor during synthesis:

```bash
# Run synthesis and watch for URAM in logs
./runCompile.sh hls 2>&1 | tee build.log

# Search for URAM mentions
grep -i uram build.log
```

## Full Implementation (Optional)

For complete resource utilization (including placement/routing):

```bash
# This takes much longer (hours to days) but gives complete picture
./runCompile.sh hw
```

Then check implementation reports in:
```
_x/workload-hw/link/vivado/vpl/prj/prj.runs/impl_1/
```

## Quick Check Without Full Synthesis

If you want a quick estimate, you can check the HLS log during `hw_emu` build:

```bash
# Check HLS synthesis log (even for hw_emu, it does some synthesis)
grep -i uram _x/workload-clause_store_handler-hw_emu/clause_store_handler/clause_store_handler/solution/syn/report/*.rpt 2>/dev/null
```

Note: This may not be as accurate as full `hw` synthesis.

## Summary

**To check URAM usage:**
1. Run: `./runCompile.sh hls` (takes 30min-2hrs)
2. Check: `FPGArpt/clause_store_handler_csynth.rpt`
3. Look for: URAM utilization section
4. Compare: Before (~80%) vs After (should be lower)

**The key metric:** URAM usage should decrease because `mClsStore` is no longer in URAM.

