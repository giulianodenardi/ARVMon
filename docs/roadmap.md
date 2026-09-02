# AVRMon Roadmap

## Phase 1: Core Foundation (v0.1) - [Current]
- [x] Individual byte read and write (`R`, `W`).
- [x] Formatted SRAM memory dump (`D`).
- [x] MCU reset via Watchdog (`RST`).
- [x] Refactoring of the monolithic structure into C/C++ modules (`include/`, `src/`).

## Phase 2: Memory & Execution
- [ ] Support for code execution from a memory address (`G <addr>`).

- [ ] Flash/EEPROM memory read/write.

- [ ] Direct Intel HEX file parsing via serial.

## Phase 3: Hardware Interface & Tooling
- [ ] I/O register read/write via aliases (e.g., `PINB`, `PORTD`).

- [ ] Python CLI interface for test automation on the host.
