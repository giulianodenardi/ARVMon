## Roadmap

### v0.1 – Proof of Concept (Legacy)
- [x] Basic Serial REPL (38400 baud)
- [x] SRAM/SFR single-byte read (`R`) and write (`W`)
- [x] EEPROM single-byte read (`RE`) and write (`WE`)
- [x] Basic Intel HEX record parsing for RAM loading
- [x] Soft reset command (`RST`)

### v0.2 – Stable Core & Clean Scope
- [x] Refactored command interpreter with safe string parsing and semicolon chaining
- [x] SRAM memory dump (`D`)
- [x] Intel HEX parser focused on RAM
- [x] Fully translated code, comments, and CLI outputs to English
- [x] Scope reduction for stock bootloader compatibility (Flash SPM operations temporarily removed)

### v0.3 – Stock Bootloader Attempt (no MiniCore)
- [x] Last attempt to keep full functionality (including Flash write) while staying on the stock Arduino bootloader
- [x] Confirmed hardware SPM lock restrictions on stock Optiboot / official Arduino bootloaders
- [x] Decision to move to MiniCore for unrestricted Flash access

### v0.4 – MiniCore / Native SPM (Current)
- [x] Switched to MiniCore core + Flash library
- [x] Re-enabled Flash dump (`DF`) and Flash write (`WF`)
- [x] Clean integration with Optiboot/Urboot SPM API
- [x] Updated version string and help text

### Future ideas
- [ ] More compact command syntax / interactive mode improvements
- [ ] Optional support for other AVR devices (ATmega168, ATmega1284, etc.)
- [ ] Small utility scripts for bulk Intel HEX loading
