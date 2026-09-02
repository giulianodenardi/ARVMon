#include "avrmon.h"

static char inputBuffer[64];
static uint8_t bufferIdx = 0;

void setup() {
  MCUSR = 0;
  wdt_disable();

  Serial.begin(115200);
  while (!Serial) {}

  Serial.println();
  Serial.println(F("AVRMon v0.1 ready. Type ? for help."));
  Serial.print(F("> "));
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\r' || c == '\n') {
      if (bufferIdx > 0) {
        inputBuffer[bufferIdx] = '\0';
        Serial.println();
        processCommandLine(inputBuffer);
        bufferIdx = 0;
        Serial.print(F("> "));
      }
    } else if (c == '\b' || c == 127) {
      if (bufferIdx > 0) {
        bufferIdx--;
        Serial.print(F("\b \b"));
      }
    } else if (bufferIdx < sizeof(inputBuffer) - 1) {
      inputBuffer[bufferIdx++] = c;
      Serial.print(c);
    }
  }
}
