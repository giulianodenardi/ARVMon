#include <Arduino.h>
#include "config.h"
#include <avr/wdt.h>
#define WDT_H

// Dedicated 256-byte user buffer placed safely in SRAM
uint8_t user_ram[256];

uint16_t parseHex(const char *str) {
  return (uint16_t) strtol(str, NULL, 16);
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
  Serial.println(F("Commands (separate multiple commands with ';'):"));
  Serial.println(F("  R <addr_hex>          -> Read byte from memory/SFR"));
  Serial.println(F("  W <addr_hex> <val_hex>-> Write byte to memory/SFR"));
  Serial.println(F("  D <addr_hex> <len_hex>-> Hexadecimal memory dump"));
  Serial.println(F("  RST                   -> Soft reset MCU via Watchdog"));
  Serial.println(F("  ?                     -> Display help menu"));
  Serial.print(F("Safe User RAM Area: "));
  printHex16((uint16_t)user_ram);
  Serial.print(F(" - "));
  printHex16((uint16_t)user_ram + sizeof(user_ram) - 1);
  Serial.println();
}

void mcuReset() {
  Serial.println(F("Resetting MCU... Bye!"));
  Serial.flush();
  
  wdt_enable(WDTO_15MS);
  while (1) {}
}

void executeSingleCommand(char *line) {
  // Trim inicial (remove espaços do começo)
  while (*line == ' ') line++;
  
  // Trim final (remove espaços, \r, \n do fim)
  int len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }
  if (len == 0) return;

  // COMMAND: RST (Explicit Software Reset)
  if (strcasecmp(line, "RST") == 0) {
    mcuReset();
    return;
  }

  char cmd = line[0];
  char arg1[16] = {0};
  char arg2[16] = {0};

  if (cmd == '?') {
    printHelp();
  }
  else if (cmd == 'R' || cmd == 'r') {
    if (sscanf(line, "%*c %15s", arg1) == 1) {
      uint16_t addr = parseHex(arg1);
      volatile uint8_t *ptr = (volatile uint8_t *)addr;
      printHex16(addr);
      Serial.print(F(" = "));
      printHex8(*ptr);
      Serial.println();
    } else {
      Serial.println(F("Err: Usage -> R <addr_hex>"));
    }
  }
  else if (cmd == 'W' || cmd == 'w') {
    if (sscanf(line, "%*c %15s %15s", arg1, arg2) == 2) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = (uint8_t)parseHex(arg2);
      volatile uint8_t *ptr = (volatile uint8_t *)addr;
      *ptr = val;
      Serial.print(F("OK! Wrote "));
      printHex8(val);
      Serial.print(F(" to "));
      printHex16(addr);
      Serial.println();
    } else {
      Serial.println(F("Err: Usage -> W <addr_hex> <val_hex>"));
    }
  }
  else if (cmd == 'D' || cmd == 'd') {
    int parsed = sscanf(line, "%*c %15s %15s", arg1, arg2);
    if (parsed >= 1) {
      uint16_t addr = parseHex(arg1);
      uint16_t dumpLen = (parsed == 2) ? parseHex(arg2) : 0x10;

      if (dumpLen == 0) dumpLen = 0x10;
      if (dumpLen > 0x0100) dumpLen = 0x0100;

      for (uint16_t i = 0; i < dumpLen; i += 8) {
        printHex16(addr + i);
        Serial.print(F(": "));

        // Print Hex bytes
        for (uint8_t j = 0; j < 8; j++) {
          if (i + j < dumpLen) {
            volatile uint8_t *ptr = (volatile uint8_t *)(addr + i + j);
            printHex8(*ptr);
            Serial.print(F(" "));
          } else {
            Serial.print(F("   ")); // Padding pra alinhar se faltar bytes
          }
        }

        Serial.print(F(" |"));

        // Print ASCII characters
        for (uint8_t j = 0; j < 8; j++) {
          if (i + j < dumpLen) {
            uint8_t b = *(volatile uint8_t *)(addr + i + j);
            if (b >= 32 && b <= 126) {
              Serial.print((char)b);
            } else {
              Serial.print(F("."));
            }
          }
        }
        Serial.println(F("|"));
      }
    } else {
      Serial.println(F("Err: Usage -> D <addr_hex> [len_hex]"));
    }
  }
  else {
    Serial.print(F("NOPE: "));
    Serial.println(line);
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
  static char buffer[128]; // Expanded buffer size for long concatenated chains
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