#include "avrmon.h"

void mcuReset() {
  Serial.println(F("Resetting MCU... Bye!"));
  Serial.flush();
  wdt_enable(WDTO_15MS);
  while (1) {}
}

void printHelp() {
  Serial.println(F("--- AVRMon Help ---"));
  Serial.println(F("R <addr>        : Read byte"));
  Serial.println(F("W <addr> <val>  : Write byte"));
  Serial.println(F("D <addr> [len]  : Dump memory"));
  Serial.println(F("RST             : Reset MCU"));
  Serial.println(F("?               : Show help"));
}

uint16_t parseHex(const char *str) {
  return (uint16_t)strtol(str, NULL, 16);
}

void printHex8(uint8_t val) {
  if (val < 0x10) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void printHex16(uint16_t val) {
  if (val < 0x1000) Serial.print(F("0"));
  if (val < 0x0100) Serial.print(F("0"));
  if (val < 0x0010) Serial.print(F("0"));
  Serial.print(val, HEX);
}

void cmdRead(const char *arg1) {
  uint16_t addr = parseHex(arg1);
  volatile uint8_t *ptr = (volatile uint8_t *)addr;
  printHex16(addr);
  Serial.print(F(" = "));
  printHex8(*ptr);
  Serial.println();
}

void cmdWrite(const char *arg1, const char *arg2) {
  uint16_t addr = parseHex(arg1);
  uint8_t val = (uint8_t)parseHex(arg2);
  volatile uint8_t *ptr = (volatile uint8_t *)addr;
  *ptr = val;
  Serial.print(F("OK! Wrote "));
  printHex8(val);
  Serial.print(F(" to "));
  printHex16(addr);
  Serial.println();
}

void cmdDump(const char *arg1, const char *arg2, int parsedCount) {
  uint16_t addr = parseHex(arg1);
  uint16_t dumpLen = (parsedCount == 2) ? parseHex(arg2) : 0x10;

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
        if (b >= 32 && b <= 126) {
          Serial.print((char)b);
        } else {
          Serial.print(F("."));
        }
      }
    }
    Serial.println(F("|"));
  }
}
