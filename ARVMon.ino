#include <Arduino.h>
#include "config.h"
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

// Dedicated 256-byte user buffer placed safely in SRAM
uint8_t user_ram[256];

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
  Serial.println(F("Commands (separate multiple commands with ';'):"));
  Serial.println(F("  R <addr_hex>          -> Read byte from SRAM/SFR"));
  Serial.println(F("  W <addr_hex> <val_hex>-> Write byte to SRAM/SFR"));
  Serial.println(F("  D <addr_hex> [len_hex]-> Hexadecimal SRAM dump"));
  Serial.println(F("  RE <addr_hex>         -> Read byte from EEPROM"));
  Serial.println(F("  WE <addr_hex> <val_hex>-> Write byte to EEPROM"));
  Serial.println(F("  DF <addr_hex> [len]   -> Dump Flash program memory"));
  Serial.println(F("  G <addr_hex>          -> Jump/Execute code at Flash address"));
  Serial.println(F("  :10010000...          -> Direct Intel HEX parser (writes to RAM)"));
  Serial.println(F("  RST                   -> Soft reset MCU via Watchdog"));
  Serial.println(F("  ?                     -> Display help menu"));
  Serial.print(F("Safe User RAM Area: "));
  printHex16((uint16_t)user_ram);
  Serial.print(F(" - "));
  printHex16((uint16_t)user_ram + sizeof(user_ram) - 1);
  Serial.println();
}

void cmdGo(const char *arg1) {
  uint16_t addr = parseHex(arg1);
  
  // Prevent accidental jumps to null/reserved SFR space
  if (addr < 0x0100) {
    Serial.println(F("Err: Cannot jump to SFR/Register space (< 0x0100)"));
    return;
  }

  Serial.print(F("Jumping to "));
  printHex16(addr);
  Serial.println(F("..."));
  Serial.flush(); // Ensure text is sent before execution shift

  // AVR ICALL requires word addressing (address >> 1)
  void (*codeExec)() = (void (*)())(addr >> 1);
  codeExec();

  Serial.println(F("\nReturned from user code."));
}

void cmdDumpFlash(const char *arg1, const char *arg2, int parsedCount) {
  uint16_t addr = parseHex(arg1);
  uint16_t dumpLen = (parsedCount == 2) ? parseHex(arg2) : 0x10;

  if (dumpLen == 0) dumpLen = 0x10;
  if (dumpLen > 0x0100) dumpLen = 0x0100;

  for (uint16_t i = 0; i < dumpLen; i += 8) {
    printHex16(addr + i);
    Serial.print(F(" (FLASH): "));

    // Hex Bytes
    for (uint8_t j = 0; j < 8; j++) {
      if (i + j < dumpLen) {
        uint8_t b = pgm_read_byte_near(addr + i + j);
        printHex8(b);
        Serial.print(F(" "));
      } else {
        Serial.print(F("   "));
      }
    }

    Serial.print(F(" |"));

    // ASCII representations
    for (uint8_t j = 0; j < 8; j++) {
      if (i + j < dumpLen) {
        uint8_t b = pgm_read_byte_near(addr + i + j);
        Serial.print((b >= 32 && b <= 126) ? (char)b : '.');
      }
    }
    Serial.println(F("|"));
  }
}

// -------------------------------------------------------------
// INTEL HEX PARSER
// Parses records matching :llaaaatt[dd...]cc and writes to RAM
// -------------------------------------------------------------
void parseIntelHex(const char *line) {
  uint8_t len = strlen(line);
  if (len < 11) {
    Serial.println(F("HEX Err: Line too short"));
    return;
  }

  uint8_t byteCount  = parseHexByte(&line[1]);
  uint16_t address   = (parseHexByte(&line[3]) << 8) | parseHexByte(&line[5]);
  uint8_t recordType = parseHexByte(&line[7]);

  // Minimum length validation based on payload
  if (len < (11 + (byteCount * 2))) {
    Serial.println(F("HEX Err: Truncated record"));
    return;
  }

  // Two's complement checksum validation
  uint8_t checksum = byteCount + (address >> 8) + (address & 0xFF) + recordType;
  for (uint8_t i = 0; i < byteCount; i++) {
    checksum += parseHexByte(&line[9 + (i * 2)]);
  }
  uint8_t fileChecksum = parseHexByte(&line[9 + (byteCount * 2)]);
  if ((uint8_t)(checksum + fileChecksum) != 0x00) {
    Serial.println(F("HEX Err: Checksum mismatch"));
    return;
  }

  // Handle record types
  if (recordType == 0x00) { // Data Record
    volatile uint8_t *ptr = (volatile uint8_t *)address;
    for (uint8_t i = 0; i < byteCount; i++) {
      ptr[i] = parseHexByte(&line[9 + (i * 2)]);
    }
    Serial.print(F("HEX OK: Wrote "));
    Serial.print(byteCount);
    Serial.print(F(" bytes to "));
    printHex16(address);
    Serial.println();
  } 
  else if (recordType == 0x01) { // End of File Record
    Serial.println(F("HEX: End of File record."));
  } 
  else {
    Serial.print(F("HEX: Record type "));
    printHex8(recordType);
    Serial.println(F(" ignored."));
  }
}

void mcuReset() {
  Serial.println(F("Resetting MCU... Bye!"));
  Serial.flush();
  
  wdt_enable(WDTO_15MS);
  while (1) {}
}

void executeSingleCommand(char *line) {
  // 1. Remove espaços e tabulações do início
  while (*line == ' ' || *line == '\t') line++;

  // 2. Remove espaços, \r e \n do final
  int len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }

  // Se a linha ficou vazia, sai fora
  if (len == 0) return;

  // Processa Intel HEX direto
  if (line[0] == ':') {
    parseIntelHex(line);
    return;
  }

  // COMMAND: RST (Software Reset)
  if (strcasecmp(line, "RST") == 0) {
    mcuReset();
    return;
  }

  // Pega a letra do comando e prepara os buffers
  char cmd = line[0];
  char arg1[16] = {0};
  char arg2[16] = {0};

  if (cmd == '?') {
    printHelp();
  }
  // COMANDOS DE DUAS LETRAS (DF, RE, WE)
  else if (strncasecmp(line, "DF", 2) == 0) {
    int parsed = sscanf(line, "%*s %15s %15s", arg1, arg2);
    if (parsed >= 1) {
      cmdDumpFlash(arg1, arg2, parsed);
    } else {
      Serial.println(F("Err: Usage -> DF <addr_hex> [len_hex]"));
    }
  }
  else if (strncasecmp(line, "RE", 2) == 0) {
    if (sscanf(line, "%*s %15s", arg1) == 1) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = eeprom_read_byte((const uint8_t *)addr);
      Serial.print(F("EEPROM "));
      printHex16(addr);
      Serial.print(F(" = "));
      printHex8(val);
      Serial.println();
    } else {
      Serial.println(F("Err: Usage -> RE <addr_hex>"));
    }
  }
  else if (strncasecmp(line, "WE", 2) == 0) {
    if (sscanf(line, "%*s %15s %15s", arg1, arg2) == 2) {
      uint16_t addr = parseHex(arg1);
      uint8_t val = (uint8_t)parseHex(arg2);
      eeprom_update_byte((uint8_t *)addr, val);
      Serial.print(F("OK! EEPROM Wrote "));
      printHex8(val);
      Serial.print(F(" to "));
      printHex16(addr);
      Serial.println();
    } else {
      Serial.println(F("Err: Usage -> WE <addr_hex> <val_hex>"));
    }
  }
  // COMANDOS DE UMA LETRA (R, W, D, G)
  else if (cmd == 'R' || cmd == 'r') {
    if (sscanf(line, "%*s %15s", arg1) == 1) {
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
    // Usamos %*s para pular o comando 'W' inteiro e qualquer espaço após ele!
    if (sscanf(line, "%*s %15s %15s", arg1, arg2) == 2) {
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
            volatile uint8_t *ptr = (volatile uint8_t *)(addr + i + j);
            printHex8(*ptr);
            Serial.print(F(" "));
          } else {
            Serial.print(F("   "));
          }
        }

        Serial.print(F(" |"));

        for (uint8_t j = 0; j < 8; j++) {
          if (i + j < dumpLen) {
            uint8_t b = *(volatile uint8_t *)(addr + i + j);
            Serial.print((b >= 32 && b <= 126) ? (char)b : '.');
          }
        }
        Serial.println(F("|"));
      }

    } else {
      Serial.println(F("Err: Usage -> D <addr_hex> [len_hex]"));
    }
  }
  else if (cmd == 'G' || cmd == 'g') {
    if (sscanf(line, "%*s %15s", arg1) == 1) {
      cmdGo(arg1);
    } else {
      Serial.println(F("Err: Usage -> G <addr_hex>"));
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
