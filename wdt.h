#ifndef WDT_H
#define WDT_H

#include <avr/wdt.h>

void mcuReset() {
  Serial.println(F("Resetting... Bye!"));
  Serial.flush(); // Garante que a mensagem Serial seja enviada antes de travar
  
  wdt_enable(WDTO_15MS); // Ativa o Watchdog para estourar em 15 milissegundos
  while (1) {}           // Loop infinito até o Watchdog reiniciar a placa
}

#endif