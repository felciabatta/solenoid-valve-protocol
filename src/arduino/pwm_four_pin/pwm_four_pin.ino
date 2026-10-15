#include <string.h>
#include "valve.h"

// DECLARE GLOBAL VARIABLES
constexpr unsigned int BUFFER_LENGTH = 25;
char data_received[BUFFER_LENGTH];  // Buffer to store incoming serial data
unsigned int PIN_VALVES[4] = { 2, 3, 4, 5 };

Valve valves[MAX_VALVES];

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
  updateState(data_received, valves, N_VALVES, BUFFER_LENGTH);
  updateActuator(valves, N_VALVES);
}
