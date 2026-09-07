# AVRMon

**AVRMon** is a minimalist, interactive machine monitor for AVR microcontrollers, with primary support for the **ATmega328P** (tested on Arduino Uno).

It runs as a bare-metal firmware and exposes a simple serial command-line interface for inspecting and manipulating the microcontroller’s memory spaces and hardware registers.

Inspired by classic machine monitors of early microcomputing (such as the Woz Monitor), AVRMon gives you a direct interface to the hardware without an operating system, emulator, or high-level framework.

AVRMon is intentionally small. It is not a full debugger or development environment — it is a compact tool for exploring how the microcontroller actually works at the register and memory level.

**Current version: v0.4 (MiniCore / Optiboot SPM)**

---

## Features (v0.4)

| Command | Description |
|---------|-------------|
| `R <addr_hex>` | Read one byte from SRAM / SFR |
| `W <addr_hex> <val_hex>` | Write one byte to SRAM / SFR |
| `D <addr_hex> [len_hex]` | Dump SRAM block (HEX + ASCII) |
| `DF <addr_hex> [len_hex]` | Dump Flash (PROGMEM) block |
| `WF <addr_hex> <val_hex>` | Write one byte to Flash (via MiniCore Flash / SPM) |
| `RE <addr_hex>` | Read one byte from EEPROM |
| `WE <addr_hex> <val_hex>` | Write one byte to EEPROM |
| `:1001...` | Intel HEX record → load data into RAM |
| `RST` | Soft reset via Watchdog Timer |
| `?` | Show help |

- Commands can be chained with `;` (example: `R 100; W 200 AA; D 100 20`).
- Default baud rate: **38400**.
- Prompt: `@ `

---

## Project Structure

```text
ARVMon/
├── ARVMon.ino          ← main firmware
├── config.h
├── wdt.h
├── docs/
│   └── roadmap.md
├── LICENSE
└── README.md
```

---

## Prerequisites

### 1. Hardware
- Arduino Uno (or any ATmega328P board)
- USB cable
- **Another Arduino** (or a dedicated ISP programmer) to burn the bootloader

### 2. MiniCore (required)

AVRMon v0.4 uses the MiniCore core and its Flash library so that the firmware can write to its own Flash memory via SPM. The stock Arduino bootloader does **not** allow this.

Install MiniCore:

1. Open **Arduino IDE** → **File → Preferences**.
2. In **Additional Boards Manager URLs** add:

   ```
   https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json
   ```

3. Go to **Tools → Board → Boards Manager…**, search for **MiniCore**, and install it.

**Burn the bootloader** (one-time operation):

1. Connect an ISP programmer (or use a second Arduino as “Arduino as ISP”).
2. Select:
   - Board: **MiniCore → ATmega328**
   - Clock: **16 MHz external** (or matching your crystal)
   - Bootloader: **Yes (UART0)**
   - Variant: **328P** (or the correct one for your chip)
   - Programmer: your ISP / “Arduino as ISP”
3. Click **Tools → Burn Bootloader**.

After this step the board will use the MiniCore/Urboot (or Optiboot-compatible) bootloader and the Flash library will work.

> More details: https://github.com/MCUdude/MiniCore

### 3. Software on the host (Linux recommended)

```bash
sudo apt update
sudo apt install picocom git -y
```

Add yourself to the `dialout` group (needed for serial access):

```bash
sudo usermod -aG dialout $USER
# log out and log back in (or run: newgrp dialout)
```

### 4. Convenient serial alias

Add this function to your `~/.bashrc` (or `~/.zshrc`):

```bash
function serial {
    local port
    case "$2" in
        ACM|acm) port="/dev/ttyACM0" ;;
        USB|usb) port="/dev/ttyUSB0" ;;
        *)       port="/dev/ttyUSB0" ;;
    esac
    picocom -b "$1" "$port" --omap crlf --imap lfcrlf
}
```

Then reload the shell:

```bash
source ~/.bashrc
```

Usage examples:

```bash
serial 38400          # → /dev/ttyUSB0 at 38400
serial 38400 ACM      # → /dev/ttyACM0 at 38400
serial 115200 usb     # → /dev/ttyUSB0 at 115200
```

---

## Installation & Upload

1. Clone the repository and **rename the folder** (important for Arduino IDE):

   ```bash
   git clone https://github.com/giulianodenardi/ARVMon.git
   mv ARVMon ARVMon          # or: mv ARVMon-main ARVMon
   cd ARVMon
   ```

   > Arduino IDE uses the folder name as the sketch name. Keeping it as `ARVMon` avoids conflicts.

2. Open the sketch in Arduino IDE:

   - **File → Open** → select `ARVMon.ino`

3. Select the board:

   - **Tools → Board → MiniCore → ATmega328**
   - Baud rate: "Default"
   - BOD: "BOD 2.7V"
   - Bootloader: "Yes (UARTO)"
   - Clock: "External 16 MHz"
   - EEPROM: "EEPROM retained"
   - Compiler LTO: "LTO enabled"
   - Variant: "328"
   - Port: your serial port (`/dev/ttyACM0` or `/dev/ttyUSB0`)

4. Click **Upload**.

---

## How to Use

1. Connect the board via USB.
2. Open a serial terminal at **38400 baud** (NL+CR or just use the alias):

   ```bash
   serial 38400 ACM     # or serial 38400 USB
   ```

3. You should see the help banner and the `@ ` prompt.
4. Type commands and press Enter. Examples:

   ```text
   ?                          # help
   R 100                      # read byte at 0x0100
   W 200 AA                   # write 0xAA to 0x0200
   D 100 20                   # dump 0x20 bytes from 0x0100
   DF 0 40                    # dump 64 bytes of Flash from address 0
   RE 10                      # read EEPROM address 0x10
   WE 10 55                   # write 0x55 to EEPROM 0x10
   WF 100 42                  # write 0x42 to Flash address 0x0100 (MiniCore only)
   RST                        # soft reset
   R 100; W 101 FF; D 100 10  # chained commands
   ```

5. Exit picocom with `Ctrl+A` then `Ctrl+X`.

---

## License

MIT License – see [LICENSE](LICENSE).

---

## Credits

- Inspired by classic machine monitors (Woz Monitor and similar early systems).
- Flash write support made possible by [MCUdude/MiniCore](https://github.com/MCUdude/MiniCore) and its Flash library.
```

