# AVRMon

AVRMon is a minimalist, interactive machine monitor for AVR microcontrollers, with primary support for the ATmega328P (tested board: Arduino Uno).

Designed to run directly on the hardware as a bare-metal system, AVRMon provides a serial command-line interface for inspecting and manipulating the microcontroller's memory and hardware registers.

Inspired by the machine monitors of early microcomputing, AVRMon brings the simplicity of classic systems such as the Woz Monitor to the AVR platform. Instead of relying on an operating system, emulator, or high-level framework, it provides a direct interface between the user and the machine.

AVRMon is intentionally small. Rather than becoming a full debugger or development environment, it aims to be a compact tool for exploring how the microcontroller actually works at the hardware level.

## Features (v0.1)
- **R `<addr_hex>`**: Memory byte read (SRAM/IO).

- **W `<addr_hex>` `<val_hex>`**: Direct byte write.

- **D `<addr_hex>` `[len_hex]`**: Visual memory dump in HEX + ASCII format.

- **RST**: Hardware reset via Watchdog Timer (WDT).

## Project Structure
```text
AVRMon/
│
├── ARVMon.ino
├── config.h
├── wdt.h
│
├── docs/
│    └── roadmap.md 
└── src/
  	├── main.cpp
  	├── parser.cpp
  	└── commands.cpp
    └── avrmon.h

```
## How to Use
Connect the Arduino via Serial at 115200 baud (NL + CR) and send commands directly from the terminal.
