#include "avrmon.h"

void executeSingleCommand(char *line) {
  while (*line == ' ') line++;

  int len = strlen(line);
  while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
    line[--len] = '\0';
  }
  if (len == 0) return;

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
      cmdRead(arg1);
    } else {
      Serial.println(F("Err: Usage -> R <addr_hex>"));
    }
  }
  else if (cmd == 'W' || cmd == 'w') {
    if (sscanf(line, "%*c %15s %15s", arg1, arg2) == 2) {
      cmdWrite(arg1, arg2);
    } else {
      Serial.println(F("Err: Usage -> W <addr_hex> <val_hex>"));
    }
  }
  else if (cmd == 'D' || cmd == 'd') {
    int parsed = sscanf(line, "%*c %15s %15s", arg1, arg2);
    if (parsed >= 1) {
      cmdDump(arg1, arg2, parsed);
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
  char *cmd = strtok(line, ";");
  while (cmd != NULL) {
    executeSingleCommand(cmd);
    cmd = strtok(NULL, ";");
  }
}
