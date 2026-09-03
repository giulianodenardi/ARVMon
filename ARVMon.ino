/*
 * AVRMon - AVR Serial Monitor & Inspection Engine
 * Version: v0.2 (RAM/SFR & EEPROM Scope)
 * Target: ATmega328P (Arduino Uno)
 */

#include <Arduino.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#define AVRMON_VERSION "v0.2"
#define AVRMON_BAUD_RATE 38400

uint8_t user_ram[256];

// Function Prototypes
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
  Serial.println(F("  RE <addr_hex>         -> Read EEPROM byte"));
  Serial.println(F("  WE <addr_hex> <val_h> -> Write EEPROM byte"));
  Serial.println(F("  :100100...            -> Intel HEX Record (RAM loader)"));
  Serial.println(F("  RST                   -> Soft Reset MCU"));
  
  Serial.print(F("Safe User RAM Area:    "));
  printHex16((uint16_t)user_ram);
  Serial.print(F(" - "));
  printHex16((uint16_t)user_ram + sizeof(user_ram) - 1);
  Serial.println();
}

void parseIntelHex(const char *line) {
  uint8_t len = strlen(line);
  if (len < 11) return;

  uint8_t byteCount  = parseHexByte(&line[1]);
  uint16_t address   = (parseHexByte(&line[3]) << 8) | parseHexByte(&line[5]);
  uint8_t recordType = parseHexByte(&line[7]);

  if (recordType == 0x00) { // Data Record
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
  while (1) {} // Wait for watchdog timer to trigger reset
}

void executeSingleCommand(char *line) {
  // Trim leading whitespace
  while (*line == ' ' || *line == '\t') line++;

  // Trim trailing whitespace and newlines
  int len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }

  if (len == 0) return;

  // Intel HEX Record handler
  if (line[0] == ':') {
    parseIntelHex(line);
    return;
  }

  // System Reset
  if (strcasecmp(line, "RST") == 0) {
    mcuReset();
    return;
  }

  char arg1[16] = {0};
  char arg2[16] = {0};

  // Help Menu
  if (line[0] == '?') {
    printHelp();
  }
  // EEPROM Read Byte
  else if (strncasecmp(line, "RE", 2) == 0) {
    if (sscanf(line, "%*s %15s", arg1) == 1) {
      uint16_t addr = parseHex(arg1);
      printHex16(addr);
      Serial.print(F(" = "));
      printHex8(eeprom_read_byte((const uint8_t *)addr));
      Serial.println();
    }
  }
  // EEPROM Write Byte
  else if (strncasecmp(line, "WE", 2) == 0) {
    if (sscanf(line, "%*s %15s %15s", arg1, arg2) == 2) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = (uint8_t)parseHex(arg2);
      eeprom_update_byte((uint8_t *)addr, val);
      Serial.print(F("OK! EEPROM Wrote "));
      printHex8(val);
      Serial.println();
    }
  }
  // SRAM/SFR Write Byte
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
  // SRAM/SFR Read Byte
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
  // SRAM Memory Dump
  else if (strncasecmp(line, "D ", 2) == 0 || strcasecmp(line, "D") == 0) {
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
            printHex8(*(volatile uint8_t *)(addr + i + j));
            Serial.print(F(" "));
          }
        }
        Serial.println();
      }
    }
  }
  // Unknown Command Handler
  else {
    Serial.print(F("NOPE: "));
    Serial.println(line);
  }
}

void processCommandLine(char *line) {
  // Split commands separated by semicolons
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
