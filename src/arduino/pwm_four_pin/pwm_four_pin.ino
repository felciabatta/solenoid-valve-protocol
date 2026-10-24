#include <string.h>
#include "types.h"
#include "valve.h"


// DECLARE GLOBAL VARIABLES

constexpr size_t BUFFER_LENGTH = 25;
char data_buffer[BUFFER_LENGTH];  // Buffer to store incoming serial data
pin_size_t PIN_VALVES[4] = { 2, 3, 4, 5 };  // Pin id for each valve

Valve valves[VALVEID_MAX];

void setup() {
  // INITIALISE SERIAL
  Serial.begin(9600);
  Serial.println("##### Arduino Ready #####\n");

  // INITIALISE LED
  pinMode(LED_BUILTIN, OUTPUT);

  // INITIALISE VALVES
  initValves(valves, N_VALVES, PIN_VALVES);
  Serial.println("[SETUP] Valves initialised.\n");

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  updateCommand(data_buffer, valves, N_VALVES, BUFFER_LENGTH);
  updateStatus(valves, N_VALVES);
  updateActuator(valves, N_VALVES);
}
