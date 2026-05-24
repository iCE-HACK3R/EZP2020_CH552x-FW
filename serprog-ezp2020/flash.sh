#!/usr/bin/env bash
# flash.sh - Flash ezp2020-serprog.bin to CH552 via wchisp
#
# Usage:
#   ./flash.sh                              # flash ezp2020-serprog.bin in current dir
#   ./flash.sh path/to/ezp2020-serprog.bin  # flash a specific binary
#
# To enter ISP (bootloader) mode on EZP2020 V2.0:
#   1. Unplug USB.
#   2. Bridge the two square P2 pads on the PCB with tweezers.
#   3. Plug in USB while holding the bridge.
#   4. Release — device should appear as VID=4348 PID=55E0.
#
# wchisp is bundled in ../Tools/flash/
# A standalone copy can also be installed from:
#   https://github.com/ch32-rs/wchisp/releases

set -euo pipefail

BIN="${1:-ezp2020-serprog.bin}"

if [[ ! -f "$BIN" ]]; then
    echo "ERROR: binary not found: $BIN"
    echo "Run 'make' first to build it."
    exit 1
fi

# Locate wchisp: prefer the bundled copy, then PATH
if [[ -x "../Tools/flash/wchisp" ]]; then
    WCHISP=../Tools/flash/wchisp
elif [[ -x "../Tools/flash/wchisp.exe" ]]; then
    WCHISP=../Tools/flash/wchisp.exe
elif command -v wchisp &>/dev/null; then
    WCHISP=wchisp
else
    echo "ERROR: wchisp not found."
    echo "Use the bundled Tools/flash/wchisp or install it from:"
    echo "  https://github.com/ch32-rs/wchisp/releases"
    exit 1
fi

echo "Checking for CH552 in ISP mode..."
"$WCHISP" info

echo ""
echo "Flashing $BIN ..."
"$WCHISP" flash "$BIN"

echo ""
echo "Done. Unplug and replug USB to start the serprog firmware."
echo "The device should enumerate as /dev/ttyACM0 (or similar)."
echo ""
echo "Test with:"
echo "  flashrom -p serprog:dev=/dev/ttyACM0:4000000"
