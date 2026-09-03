# AVRMon Roadmap

## v0.1 - Proof of Concept (Legacy)
- [x] Basic Serial REPL (38400 baud)
- [x] SRAM/SFR single-byte read (`R`) and write (`W`)
- [x] EEPROM single-byte read (`RE`) and write (`WE`)
- [x] Basic Intel HEX record parsing for RAM loading
- [x] Soft reset command (`RST`)

---

## v0.2 - Stable Core & Clean Scope (Current)
- [x] Refactored command interpreter with safe string parsing and semicolon chaining (`cmd1; cmd2`)
- [x] SRAM memory dump (`D <addr> [len]`)
- [x] Intel HEX parser focused strictly on RAM space
- [x] Fully translated code, comments, and CLI outputs to English
- [x] Scope reduction and deprecation:
  - **Removed Flash SPM operations (`WF`, `EF`, `DF`):** Dropped due to hardware SPM lock restrictions in stock ATmega328P bootloaders/fuses.
  - **Removed Flash dynamic jump (`G`):** Removed code execution in Flash to maintain code stability and portability without requiring custom ISP flashing.

---

## v0.3 - ESP32 Fork (ESPMon)
- [ ] Port codebase to ESP32 architecture (32-bit address space)
- [ ] Expand memory inspection commands to support 32-bit addresses (`uint32_t`)
- [ ] Implement RAM dynamic code execution (`G`) using ESP32 executable heap (`MALLOC_CAP_EXEC`)
- [ ] Non-Volatile Storage (NVS) read/write commands as EEPROM alternative
- [ ] Multi-core memory and peripheral inspection capabilities
