/*
 * AVRMon - AVR Serial Monitor & Inspection Engine
 * Version: v0.4 (Optiboot SPM Native API)
 */

#include <Arduino.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

// MiniCore's native library for unrestricted access in Flash.
#include <Flash.h>

#define AVRMON_VERSION "v0.4-MiniCore"
#define AVRMON_BAUD_RATE 38400


uint8_t user_ram[256];
uint8_t ram_buffer[SPM_PAGESIZE];

#define NUMBER_OF_PAGES 8   // quantas páginas você quer poder escrever
const uint8_t flash_space[SPM_PAGESIZE * NUMBER_OF_PAGES] 
  __attribute__((aligned(SPM_PAGESIZE))) PROGMEM = {};

Flash flash(flash_space, sizeof(flash_space), ram_buffer, sizeof(ram_buffer));

void printHex16(uint16_t val);
void printHex8(uint8_t val);

uint16_t parseHex(const char *str) {
  return (uint16_t) strtol(str, NULL, 16);
}

uint8_t parseHexByte(const char *str) {
  char buf[3] = {str[0], str[1], '\0'};
  return (uint8_t) strtol(buf, NULL, 16);
}

void printHex16(uint16_t val) {
  Serial.print(F("0x"));
  if (val < 0x1000) Serial.print(F("0"));
  if (val < 0x0100) Serial.print(F("0"));
  if (val < 0x0010) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void printHex8(uint8_t val) {
  Serial.print(F("0x"));
  if (val < 0x10) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void printHelp() {
  Serial.println(F("\n=== AVRMon " AVRMON_VERSION " ==="));
  Serial.println(F("Commands:"));
  Serial.println(F("  R <addr_hex>          -> Read SRAM/SFR byte"));
  Serial.println(F("  W <addr_hex> <val_hex>-> Write SRAM/SFR byte"));
  Serial.println(F("  D <addr_hex> [len_hex]-> Dump SRAM memory block"));
  Serial.println(F("  DF <addr_hex> [len_h] -> Dump Flash (PROGMEM) block"));
  Serial.println(F("  WF <addr_hex> <val_h> -> Write Flash byte (Optiboot SPM)"));
  Serial.println(F("  RE <addr_hex>         -> Read EEPROM byte"));
  Serial.println(F("  WE <addr_hex> <val_h> -> Write EEPROM byte"));
  Serial.println(F("  :100100...            -> Intel HEX Record (RAM loader)"));
  Serial.println(F("  RST                   -> Soft Reset MCU"));
}


// Write function to Flash using the actual Flash.h syntax of MiniCore
void writeFlashByte(uint16_t addr, uint8_t val) {
  // Calculates the page number within the allocated space 
  // (only works if addr is within the flash_space you allocated)
  uint16_t pageNumber = (addr - (uint16_t)flash_space) / SPM_PAGESIZE;
  
  // 1. Reads the entire page into the buffer.
  flash.fetch_page(pageNumber);
  
  // 2. Modify the desired byte
  uint16_t offset = addr % SPM_PAGESIZE;
  flash[offset] = val;
  
  // 3. Save the page back
  flash.write_page(pageNumber);
}

void parseIntelHex(const char *line) {
  uint8_t len = strlen(line);
  if (len < 11) return;

  uint8_t byteCount  = parseHexByte(&line[1]);
  uint16_t address   = (parseHexByte(&line[3]) << 8) | parseHexByte(&line[5]);
  uint8_t recordType = parseHexByte(&line[7]);

  if (recordType == 0x00) {
    volatile uint8_t *ptr = (volatile uint8_t *)address;
    for (uint8_t i = 0; i < byteCount; i++) {
      ptr[i] = parseHexByte(&line[9 + (i * 2)]);
    }
    Serial.print(F("HEX OK: Wrote "));
    Serial.print(byteCount);
    Serial.print(F(" bytes to RAM "));
    printHex16(address);
    Serial.println();
  }
}

void mcuReset() {
  Serial.println(F("Resetting MCU..."));
  Serial.flush();
  wdt_enable(WDTO_15MS);
  while (1) {}
}

void executeSingleCommand(char *line) {
  while (*line == ' ' || *line == '\t') line++;
  int len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }

  if (len == 0) return;

  if (line[0] == ':') {
    parseIntelHex(line);
    return;
  }

  if (strcasecmp(line, "RST") == 0) {
    mcuReset();
    return;
  }

  char arg1[16] = {0};
  char arg2[16] = {0};

  if (line[0] == '?') {
    printHelp();
  }
  else if (strncasecmp(line, "DF", 2) == 0) {
    int parsed = sscanf(line, "%*s %15s %15s", arg1, arg2);
    if (parsed >= 1) {
      uint16_t addr = parseHex(arg1);
      uint16_t dumpLen = (parsed == 2) ? parseHex(arg2) : 0x10;
      if (dumpLen == 0) dumpLen = 0x10;
      if (dumpLen > 0x0100) dumpLen = 0x0100;

      for (uint16_t i = 0; i < dumpLen; i += 8) {
        printHex16(addr + i);
        Serial.print(F(": "));
        for (uint8_t j = 0; j < 8; j++) {
          if (i + j < dumpLen) {
            uint8_t b = pgm_read_byte((const void *)(addr + i + j));
            printHex8(b);
            Serial.print(F(" "));
          } else {
            Serial.print(F("     ")); 
          }
        }
        Serial.println();
      }
    }
  }
  else if (strncasecmp(line, "WF", 2) == 0) {
    if (sscanf(line, "%*s %15s %15s", arg1, arg2) == 2) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = (uint8_t)parseHex(arg2);

      writeFlashByte(addr, val);

      Serial.print(F("OK! Flash Wrote "));
      printHex8(val);
      Serial.print(F(" to Flash "));
      printHex16(addr);
      Serial.println();
    }
  }
  else if (strncasecmp(line, "R ", 2) == 0 || strcasecmp(line, "R") == 0) {
    if (sscanf(line, "%*s %15s", arg1) == 1) {
      uint16_t addr = parseHex(arg1);
      volatile uint8_t *ptr = (volatile uint8_t *)addr;
      printHex16(addr);
      Serial.print(F(" = "));
      printHex8(*ptr);
      Serial.println();
    }
  }
  else if (strncasecmp(line, "W ", 2) == 0 || strcasecmp(line, "W") == 0) {
    if (sscanf(line, "%*s %15s %15s", arg1, arg2) == 2) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = (uint8_t)parseHex(arg2);
      volatile uint8_t *ptr = (volatile uint8_t *)addr;
      *ptr = val;
      Serial.print(F("OK! Wrote "));
      printHex8(val);
      Serial.print(F(" to RAM "));
      printHex16(addr);
      Serial.println();
    }
  }
}

void processCommandLine(char *line) {
  char *token = strtok(line, ";");
  while (token != NULL) {
    executeSingleCommand(token);
    token = strtok(NULL, ";");
  }
}

void setup() {
  Serial.begin(AVRMON_BAUD_RATE);
  printHelp();
  Serial.print(F("\n@ "));
}

void loop() {
  static char buffer[128];
  static uint8_t bufIdx = 0;

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (bufIdx > 0) {
        buffer[bufIdx] = '\0';
        processCommandLine(buffer);
        bufIdx = 0;
        Serial.print(F("\n@ "));
      }
    } else if (bufIdx < sizeof(buffer) - 1) {
      buffer[bufIdx++] = c;
    }
  }
}
