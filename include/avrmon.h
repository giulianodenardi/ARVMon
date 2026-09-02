#ifndef AVRMON_H
#define AVRMON_H

#include <Arduino.h>
#include <avr/wdt.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

// Prototypes of system functions and commands
void mcuReset();
void printHelp();
uint16_t parseHex(const char *str);
void printHex8(uint8_t val);
void printHex16(uint16_t val);

// Commands
void cmdRead(const char *arg1);
void cmdWrite(const char *arg1, const char *arg2);
void cmdDump(const char *arg1, const char *arg2, int parsedCount);

// Parser
void executeSingleCommand(char *line);
void processCommandLine(char *line);

#endif
