# Getting Started with XIAO nRF54L15 Bootloader

This guide walks you through building, flashing, and using the Adafruit nRF52 Bootloader on the Seeed Studio XIAO nRF54L15 and XIAO nRF54L15 Sense boards.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Building the Bootloader](#building-the-bootloader)
3. [Initial Flashing](#initial-flashing)
4. [Entering Bootloader Mode](#entering-bootloader-mode)
5. [Creating Applications](#creating-applications)
6. [Performing DFU Updates](#performing-dfu-updates)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware

- **Seeed Studio XIAO nRF54L15** or **XIAO nRF54L15 Sense**
- USB-C cable (must support data transfer, not charge-only)
- Optional: J-Link programmer for initial flashing (if no bootloader is present)

### Software

#### 1. ARM GCC Toolchain

Download and install the ARM GCC toolchain:

```bash
# On Linux
sudo apt-get install gcc-arm-none-eabi

# On macOS
brew install gcc-arm-embedded

# Or download from ARM:
# https://developer.arm.com/downloads/-/gnu-rm
```

Verify installation:
```bash
arm-none-eabi-gcc --version
```

#### 2. Build Tools

**CMake (recommended):**
```bash
# Linux
sudo apt-get install cmake

# macOS
brew install cmake
```

**Or Make:**
```bash
# Usually pre-installed on Linux/macOS
make --version
```

#### 3. Nordic Tools

**nRF Command Line Tools** (includes nrfjprog, mergehex):
- Download from [Nordic Semiconductor](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools)
- Required for J-Link flashing and hex file manipulation

#### 4. SoftDevice S145

**Important:** The SoftDevice S145 v9.0.0 API has been published, but the binary is pending release from Nordic Semiconductor.

Once available:
1. Download SoftDevice S145 for nRF54L from Nordic
2. Extract to `lib/softdevice/s145_nrf54l_9.0.0/`
3. The directory should contain:
   - `s145_nrf54l_9.0.0_softdevice.hex`
   - API headers in `s145_nrf54l_9.0.0_API/`

## Building the Bootloader

### Clone the Repository

```bash
git clone https://github.com/adafruit/Adafruit_nRF52_Bootloader.git
cd Adafruit_nRF52_Bootloader
git submodule update --init --recursive
```

### Update nrfx for nRF54L15 Support

The nrfx submodule must support nRF54L15. Update if necessary:

```bash
cd lib/nrfx
git checkout v3.1.0  # Or later version with nRF54L15 support
cd ../..
git add lib/nrfx
```

### Build for XIAO nRF54L15

**Using CMake (Recommended):**

```bash
# For XIAO nRF54L15 (base version)
cmake -DBOARD=xiao_nrf54l15 -B_build
cmake --build _build

# For XIAO nRF54L15 Sense
cmake -DBOARD=xiao_nrf54l15_sense -B_build_sense
cmake --build _build_sense
```

**Using Make:**

```bash
# For XIAO nRF54L15
make BOARD=xiao_nrf54l15

# For XIAO nRF54L15 Sense
make BOARD=xiao_nrf54l15_sense
```

### Build Artifacts

After building, you'll find these files in `_build/` (or `_build-xiao_nrf54l15/` for Make):

- `xiao_nrf54l15_bootloader-<version>-nosd.hex` - Bootloader only
- `xiao_nrf54l15_bootloader-<version>-nosd.bin` - Bootloader binary
- `xiao_nrf54l15_bootloader-<version>.hex` - Bootloader + SoftDevice + MBR
- `xiao_nrf54l15_bootloader-<version>.zip` - DFU package

## Initial Flashing

### Method 1: Using J-Link (First Time Setup)

If you have a J-Link programmer:

```bash
# With CMake
cmake --build _build --target flash-jlink

# With Make
make BOARD=xiao_nrf54l15 flash
```

This will flash the combined bootloader with MBR and SoftDevice.

### Method 2: Using CMSIS-DAP (via XIAO's onboard SAMD11)

The XIAO nRF54L15 has an onboard SAMD11 debugger with CMSIS-DAP support.

**Option A: Using nrfjprog with CMSIS-DAP:**

```bash
nrfjprog --program _build/xiao_nrf54l15_bootloader-*-nosd.hex --chiperase --verify
nrfjprog --reset
```

**Option B: Using pyOCD:**

```bash
# Install pyOCD
pip install pyocd

# Flash bootloader
pyocd flash -t nrf54l15 _build/xiao_nrf54l15_bootloader-*-nosd.hex
pyocd reset
```

### Method 3: Using Existing UF2 Bootloader

If a UF2 bootloader is already installed (e.g., factory bootloader):

1. Enter bootloader mode (double-tap reset)
2. XIAO will appear as a USB drive
3. Copy the `.uf2` file to the drive:

```bash
# Build UF2 file
cmake --build _build --target bootloader_mbr.uf2

# Copy to mounted drive (adjust path as needed)
cp _build/xiao_nrf54l15_bootloader_mbr.uf2 /media/XIAO54L15/
```

## Entering Bootloader Mode

Once the bootloader is installed, you can enter it using two methods:

### Method 1: Double-Tap Reset (DFU Mode)

1. Quickly press the RESET button twice (within 500ms)
2. The LED will fade in/out to indicate bootloader mode
3. The XIAO will appear as a USB mass storage device named **XIAO54L15** (or **XIAO-SENSE**)

### Method 2: Hold Boot + Reset

1. Press and hold the BOOT button (D0)
2. Press and release the RESET button
3. Release the BOOT button
4. The bootloader will start

### Method 3: From Application Code

Your application can enter the bootloader programmatically:

```c
#include "nrf_power.h"

void enter_bootloader(void)
{
    // Set DFU flag
    NRF_POWER->GPREGRET = 0x57; // DFU magic number
    NVIC_SystemReset();
}
```

## Creating Applications

### Memory Layout

Your application must be compiled to fit within the application space:

```
Flash Memory Layout:
0x000000 - 0x001000: MBR (4 KB)
0x001000 - 0x025000: SoftDevice S145 (144 KB)
0x025000 - 0x16A000: Application Space (~1.32 MB)  <-- YOUR APP GOES HERE
0x174000 - 0x17E000: Bootloader (38 KB)
0x17E000 - 0x17F000: MBR Parameters (4 KB)
0x17F000 - 0x180000: Bootloader Settings (4 KB)

RAM Memory Layout:
0x20000000 - 0x20003000: SoftDevice RAM (12 KB)
0x20003000 - 0x20040000: Application RAM (244 KB)  <-- YOUR APP USES THIS
```

### Linker Script Configuration

Your application linker script must specify:

**Flash:**
- Application start: `0x025000`
- Application size: `0x145000` (1,316 KB)

**RAM:**
- RAM start: `0x20003000`
- RAM size: `0x3D000` (244 KB)

### Example Linker Script Snippet

```ld
MEMORY
{
  FLASH (rx) : ORIGIN = 0x025000, LENGTH = 0x145000
  RAM (rwx)  : ORIGIN = 0x20003000, LENGTH = 0x3D000
}
```

### SoftDevice S145 Configuration

In your application, initialize SoftDevice S145:

```c
#include "nrf_sdm.h"
#include "nrf_soc.h"

void softdevice_init(void)
{
    uint32_t ram_start = 0x20003000;

    // Enable SoftDevice
    nrf_sdh_enable_request();

    // Configure BLE stack using SoftDevice S145 API
    // (Refer to S145 API documentation for details)
}
```

### Creating DFU Packages from Your Application

To create a DFU package that can be uploaded via the bootloader:

```bash
# Using adafruit-nrfutil
pip install adafruit-nrfutil

# Create DFU package from your application hex file
adafruit-nrfutil dfu genpkg --dev-type 0xADA54115 \
                            --sd-req 0x3024 \
                            --application your_app.hex \
                            your_app_dfu.zip
```

**Parameters:**
- `--dev-type 0xADA54115`: UF2 family ID for nRF54L15
- `--sd-req 0x3024`: SoftDevice S145 v9.0.0 firmware ID
- `--application`: Your application hex file

## Performing DFU Updates

### Method 1: UF2 Drag-and-Drop (Easiest)

1. Convert your application to UF2 format:

```bash
# Using uf2conv.py (included in repository)
python3 lib/uf2/utils/uf2conv.py -c -f 0xADA54115 -b 0x025000 \
        your_app.hex -o your_app.uf2
```

2. Enter bootloader mode (double-tap reset)
3. Drag `your_app.uf2` to the XIAO USB drive
4. The bootloader will flash the application and reboot automatically

### Method 2: Serial DFU (via nrfutil)

For DFU over USB serial:

```bash
# Find the serial port
# Linux: /dev/ttyACM0
# macOS: /dev/tty.usbmodem*
# Windows: COM#

# Enter bootloader mode (double-tap reset)

# Perform DFU
adafruit-nrfutil dfu serial -pkg your_app_dfu.zip -p /dev/ttyACM0 -b 115200
```

### Method 3: Bluetooth LE DFU (OTA)

For wireless updates (requires application with BLE DFU support):

```bash
# Using nRF Connect app or nrfutil
adafruit-nrfutil dfu ble -pkg your_app_dfu.zip -a <device_address>
```

**Note:** Your application must implement the BLE DFU service for OTA updates.

## Advanced Topics

### Enabling Debug Output

The bootloader can output debug messages over USB CDC serial. To view:

```bash
# Linux/macOS
screen /dev/ttyACM0 115200

# Or using minicom
minicom -D /dev/ttyACM0 -b 115200

# Windows: Use PuTTY or TeraTerm
```

### Building with Debug Symbols

```bash
# CMake with debug build
cmake -DBOARD=xiao_nrf54l15 -DCMAKE_BUILD_TYPE=Debug -B_build_debug
cmake --build _build_debug

# Make with debug
make BOARD=xiao_nrf54l15 DEBUG=1
```

Debug builds provide:
- More verbose serial output
- Larger bootloader size (78 KB vs 38 KB)
- Starts at 0x16A000 instead of 0x174000

### Customizing Board Configuration

To modify LED behavior, button assignments, or USB identifiers:

1. Edit `src/boards/xiao_nrf54l15/board.h`:

```c
// Change LED pin
#define LED_PRIMARY_PIN       _PINNUM(2, 0)

// Change button pins
#define BUTTON_1              _PINNUM(1, 4)

// Change USB VID/PID
#define USB_DESC_VID           0x2886
#define USB_DESC_UF2_PID       0x0068
```

2. Rebuild:
```bash
cmake --build _build --target clean
cmake --build _build
```

### Protecting the Bootloader

To enable flash protection (prevents accidental overwrite):

```bash
# After flashing bootloader
nrfjprog --memwr 0x10001200 --val 0x00000000
nrfjprog --reset
```

This writes to UICR to protect bootloader region.

## Troubleshooting

### Bootloader Not Entering

**Symptom:** Double-tap reset doesn't work

**Solutions:**
1. Check timing - must double-tap within 500ms
2. Try holding BOOT button while pressing RESET
3. Verify USB cable supports data (not charge-only)
4. Check if bootloader is actually flashed:
   ```bash
   nrfjprog --memrd 0x174000 --n 16
   # Should show non-0xFF values
   ```

### USB Device Not Recognized

**Symptom:** Computer doesn't detect XIAO in bootloader mode

**Solutions:**
1. **Windows:** Install Seeed XIAO drivers
2. Check Device Manager (Windows) or `lsusb` (Linux) for VID:PID 0x2886:0x0068
3. Try different USB port
4. Verify USB cable quality

### Application Doesn't Start After Flashing

**Symptom:** Bootloader works, but application doesn't run

**Solutions:**
1. Verify application is compiled for correct address (0x025000)
2. Check application size fits in available space (max 1.32 MB)
3. Ensure SoftDevice S145 is included
4. Check bootloader settings:
   ```bash
   nrfjprog --memrd 0x17F000 --n 256
   ```
5. Verify application has valid vector table at 0x025000

### DFU Update Fails

**Symptom:** DFU package upload fails or corrupts

**Solutions:**
1. Verify DFU package parameters:
   - Device type: 0xADA54115
   - SoftDevice requirement: 0x3024
2. Check application size (must fit in 0x025000-0x16A000)
3. Ensure enough space in flash
4. Try serial DFU instead of UF2 if drag-and-drop fails
5. Erase settings and retry:
   ```bash
   nrfjprog --erasepage 0x17F000
   ```

### SoftDevice Not Available

**Symptom:** Build fails because S145 binary is missing

**Solutions:**
1. Check `lib/softdevice/s145_nrf54l_9.0.0/` exists
2. SoftDevice S145 v9.0.0 binary is pending release from Nordic
3. Monitor [Nordic Downloads](https://www.nordicsemi.com/Products/Development-software/s145/download) for release
4. Can build without SoftDevice for testing:
   ```bash
   cmake -DBOARD=xiao_nrf54l15 -DNO_SOFTDEVICE=1 -B_build
   ```

### LED Doesn't Blink

**Symptom:** No LED indication in bootloader mode

**Solutions:**
1. Verify LED is on P2.0 (check schematic)
2. LED is active-low on XIAO nRF54L15
3. Check LED isn't damaged
4. Modify `board.h` if LED pin differs:
   ```c
   #define LED_PRIMARY_PIN       _PINNUM(2, 0)
   #define LED_STATE_ON          0  // 0 for active-low
   ```

### Serial Output Not Working

**Symptom:** No debug messages over USB CDC

**Solutions:**
1. Ensure debug build is used (not release)
2. Open serial terminal before entering bootloader
3. Check baud rate (should be 115200)
4. On Linux, may need to add user to dialout group:
   ```bash
   sudo usermod -a -G dialout $USER
   # Then logout/login
   ```

### J-Link Connection Issues

**Symptom:** nrfjprog can't connect to XIAO

**Solutions:**
1. Install latest J-Link software
2. Try specifying probe:
   ```bash
   nrfjprog --jlink CMSIS-DAP --program bootloader.hex
   ```
3. Check SWD connections if using external J-Link
4. Reset XIAO before programming
5. Use slower SWD speed:
   ```bash
   nrfjprog --clockspeed 1000 --program bootloader.hex
   ```

## Testing Your Bootloader

### Quick Test Sequence

1. **Flash bootloader:**
   ```bash
   cmake --build _build --target flash-jlink
   ```

2. **Double-tap reset:**
   - LED should fade in/out
   - USB drive "XIAO54L15" appears

3. **Create test UF2:**
   ```bash
   # Use provided blinky example or create simple hex
   python3 lib/uf2/utils/uf2conv.py -c -f 0xADA54115 \
           -b 0x025000 test_app.hex -o test.uf2
   ```

4. **Upload test UF2:**
   - Drag to USB drive
   - XIAO reboots automatically

5. **Verify application runs:**
   - Check serial output
   - Observe expected behavior

## Resources

### Documentation

- [XIAO nRF54L15 Boards](./xiao_nrf54l15_boards.md) - Hardware specifications and pinout
- [nRF54L15 Support](./nrf54l15_support.md) - Memory layout and architecture details
- [S145 Validation](./s145_validation.md) - SoftDevice specifications

### Nordic Resources

- [nRF54L15 Product Page](https://www.nordicsemi.com/Products/nRF54L15)
- [SoftDevice S145 Documentation](https://docs.nordicsemi.com/bundle/s145_9.0.0/page/topics.html)
- [nRF54L15 Development Kit](https://www.nordicsemi.com/Products/Development-hardware/nRF54L15-DK)

### Seeed Studio Resources

- [XIAO nRF54L15 Wiki](https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/)
- [Product Page](https://www.seeedstudio.com/XIAO-nRF54L15-p-6493.html)
- [Forum Support](https://forum.seeedstudio.com/)

### Tools

- [adafruit-nrfutil](https://github.com/adafruit/Adafruit_nRF52_nrfutil) - DFU package creation
- [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools)
- [UF2 Utils](https://github.com/microsoft/uf2) - UF2 file conversion

## FAQ

### Q: Do I need SoftDevice S145 to use the bootloader?

A: The bootloader itself works without SoftDevice, but most applications will need S145 for Bluetooth LE functionality. The bootloader reserves space for S145 in the memory layout.

### Q: Can I use this with Zephyr or nRF Connect SDK?

A: Yes! Configure your Zephyr application to use:
- Flash start: 0x025000
- RAM start: 0x20003000
- Enable DFU support in your application

### Q: What's the difference between UF2 and DFU zip files?

A:
- **UF2**: Drag-and-drop format, easiest for users, works in bootloader mode
- **DFU zip**: Used for serial/BLE updates, can include SoftDevice updates, supports signatures

### Q: Can I update the SoftDevice via DFU?

A: Yes, create a DFU package with SoftDevice + application:
```bash
adafruit-nrfutil dfu genpkg --dev-type 0xADA54115 \
                            --sd-req 0x3024 \
                            --softdevice s145_nrf54l_9.0.0.hex \
                            --application your_app.hex \
                            package.zip
```

### Q: How do I factory reset the bootloader?

A: Erase all flash and reflash:
```bash
nrfjprog --eraseall
nrfjprog --program bootloader_with_sd.hex
nrfjprog --reset
```

### Q: Can I use this bootloader on other nRF54L boards?

A: Yes! Create a new board configuration in `src/boards/<your_board>/` following the XIAO examples. Update pins, USB IDs, and rebuild.

## Next Steps

1. **Build and flash** the bootloader using this guide
2. **Create a simple application** (LED blink) to test DFU
3. **Implement BLE functionality** using SoftDevice S145 API
4. **Add OTA DFU support** to your application for wireless updates
5. **Explore advanced features** like bond sharing and buttonless DFU

For issues or questions:
- Check the [Troubleshooting](#troubleshooting) section
- Review [GitHub Issues](https://github.com/adafruit/Adafruit_nRF52_Bootloader/issues)
- Post on [Adafruit Forums](https://forums.adafruit.com/)

## License

This bootloader is based on the Adafruit nRF52 Bootloader and is released under the MIT License.
