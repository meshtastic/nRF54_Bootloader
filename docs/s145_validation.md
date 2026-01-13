# SoftDevice S145 9.0.0 Validation Report

## Overview

This document validates the nRF54L15 bootloader implementation against the published SoftDevice S145 9.0.0 API documentation.

**Status**: API published, binary not yet available
**Date**: January 2026
**Documentation**: https://docs.nordicsemi.com/bundle/s145_9.0.0/page/topics.html

## Findings and Required Updates

### 1. Version Number ❌ **NEEDS UPDATE**

**Current Implementation:**
- `SD_VERSION = 1.0.0` (CMakeLists.txt line 55, Makefile line 27)

**Correct Version:**
- `SD_VERSION = 9.0.0`

**Reasoning:**
- SoftDevice S115 for nRF54L uses version 9.0.0 (v9.0.0-3.prototype)
- S145 follows the same versioning scheme for nRF54L series
- This differs from nRF52 SoftDevice versions (6.x, 7.x)

**Action Required:**
Update both CMakeLists.txt and Makefile to use `SD_VERSION = 9.0.0` for nRF54L15.

---

### 2. RAM Requirements ⚠️ **VERIFY WHEN BINARY AVAILABLE**

**Current Implementation:**
- RAM base address: `0x20004000` (16 KB reserved for SoftDevice)
- Comment: "SoftDevice S145 estimated to use ~16KB RAM"

**Documentation Data:**
- **S115 v9.0.0-3.prototype**: 10.8 kB total RAM
  - Base: 6.7 kB (4.9 kB minimum + 1.8 kB worst-case call stack)
  - Configurable: ~4.1 kB (application compile-time configuration)

- **S145 Characteristics**:
  - Central and multirole (vs S115 peripheral-only)
  - Up to 5 concurrent connections (vs S115 2 connections)
  - Includes Coded PHY and Extended Advertising

**Analysis:**
- 16 KB (0x4000) allocation is reasonable for S145 given:
  - S115 uses ~11 KB for 2 peripheral connections
  - S145 supports 5 connections in all roles
  - Central role requires additional RAM for scanning/connection management
  - Extended advertising and Coded PHY add overhead

**Recommendation:**
- Keep current 16 KB allocation
- Verify against actual binary when released
- May need adjustment to 20 KB (0x5000) if 5-connection multirole requires it

---

### 3. Flash Size ⚠️ **NEEDS VERIFICATION**

**Current Implementation:**
- SoftDevice space: `0x001000 - 0x02D000` (~180 KB, 0x2C000 bytes)

**Available Data:**
- No official S145 flash size published yet
- S115 flash size not found in available documentation
- nRF52 comparison:
  - S132 (peripheral): ~112 KB
  - S140 (central/multirole): ~136-152 KB depending on version

**Analysis:**
- 180 KB estimate is reasonable given:
  - S145 includes central role functionality
  - Extended advertising and Coded PHY support
  - nRF54L15 RRAM vs Flash may have different size requirements

**Recommendation:**
- Keep current 180 KB allocation as conservative estimate
- **CRITICAL**: Update once official binary is released
- Application start address (0x02D000) should be verified

---

### 4. Directory Structure ✅ **CORRECT**

**Current Implementation:**
```
lib/softdevice/s145_nrf54l_9.0.0/
└── s145_nrf54l_9.0.0_API/include/
```

**Expected Structure:**
Following nRF52 pattern, the structure should be:
```
lib/softdevice/s145_nrf54l_9.0.0/
├── s145_nrf54l_9.0.0_softdevice.hex  (when binary released)
└── s145_nrf54l_9.0.0_API/
    └── include/
        └── [header files]
```

**Action Required:**
- Update build system to expect version 9.0.0
- Path will be: `lib/softdevice/s145_nrf54l_9.0.0/`

---

### 5. Bootloader Memory Layout ✅ **ACCEPTABLE**

**Current Implementation:**
```
0x174000 - 0x17D800: Bootloader (38 KB release)
0x17D800 - 0x17E000: Bootloader Config (2 KB)
0x17E000 - 0x17F000: MBR Parameters (4 KB)
0x17F000 - 0x180000: Bootloader Settings (4 KB)
```

**Analysis:**
- Layout follows nRF52 bootloader pattern
- 38 KB bootloader size is adequate
- Last 12 KB reserved for configuration/settings
- No conflicts with SoftDevice S145 requirements

**Status:** No changes needed

---

### 6. Device Configuration ✅ **CORRECT**

**Current Implementation:**
- Device revision: 54115
- UF2 Family ID: 0xADA54115
- MCU defines: `NRF54L15_XXAA`, `S145`

**Status:** Correct and following established patterns

---

## Summary of Required Changes

### Immediate Updates Needed:

1. **Update SD_VERSION**: Change from 1.0.0 to 9.0.0
   - File: `CMakeLists.txt` (line 55)
   - File: `Makefile` (line 27)
   - File: `docs/nrf54l15_support.md` (multiple references)

2. **Update Documentation**: Correct version references
   - Change "S145 v1.0.0" to "S145 v9.0.0"
   - Add note about version numbering difference from nRF52

### Verification Needed When Binary Released:

1. **Flash Size**: Verify 180 KB allocation is sufficient
   - May need to adjust application start address
   - Update linker script if needed

2. **RAM Size**: Verify 16 KB allocation is sufficient
   - Test with 5 concurrent connections
   - Adjust if needed (possibly to 20 KB)

3. **Memory Addresses**: Confirm:
   - SoftDevice ends at 0x02D000
   - No conflicts with bootloader regions
   - RAM layout is optimal

---

## Version Numbering Clarification

### nRF52 Series SoftDevices:
- S132: v6.1.1, v7.x
- S140: v6.1.1, v7.2.0, v7.3.0

### nRF54L Series SoftDevices:
- S115: v9.0.0-3.prototype
- S145: v9.0.0 (API published)

**Key Insight**: Nordic restarted version numbering at 9.0.0 for nRF54L SoftDevices, distinguishing them from nRF52 versions.

---

## Testing Checklist

Once SoftDevice S145 9.0.0 binary is available:

- [ ] Verify flash size matches 180 KB allocation
- [ ] Test bootloader with actual S145 binary
- [ ] Verify RAM allocation with multirole configurations
- [ ] Test DFU with S145 present
- [ ] Validate 5 concurrent connections don't exceed RAM
- [ ] Test Coded PHY functionality
- [ ] Test Extended Advertising
- [ ] Verify bootloader entry modes work correctly
- [ ] Test UF2 firmware updates
- [ ] Validate bond information exchange (NOINIT region)

---

## References

- **S145 API Documentation**: https://docs.nordicsemi.com/bundle/s145_9.0.0/page/topics.html
- **S115 Memory Requirements**: ~10.8 kB RAM (v9.0.0-3.prototype)
- **nRF Connect SDK Bare Metal**: https://www.nordicsemi.com/Products/Development-software/nRF-Connect-SDK/Bare-Metal-option-for-nRF54L-Series
- **Nordic Blog Post**: https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/a-technical-dive-into-the-nrf-connect-sdk-bare-metal-option

---

## Conclusion

**Current Status**: Implementation is fundamentally sound with one critical correction needed.

**Priority Action**: Update SD_VERSION from 1.0.0 to 9.0.0 across all configuration files.

**Secondary Actions**: Add documentation notes about verification requirements once the S145 binary is released.

The memory layout (16 KB RAM, 180 KB flash for SoftDevice) is reasonable based on S115 data and S145 characteristics, but should be verified against the actual binary when available.
