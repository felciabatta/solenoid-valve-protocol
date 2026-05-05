#include <string.h>
#include "valve.h"

// DECLARE GLOBAL VARIABLES
int pin_l;
char received_data[25];  // Buffer to store incoming serial data
int PIN_VALVES[4] = {2, 3, 4, 5};

Valve valves[MAX_VALVES];

void setup() {
  // INITIALISE SERIAL
  Serial.begin(9600);
  Serial.println("Arduino Ready...");

  // INITIALISE LED
  pinMode(LED_BUILTIN, OUTPUT);

  // INITIALISE VALVES
  initValves(valves, N_VALVES, PIN_VALVES);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // Check if serial data is available
  if (Serial.available() > 0) {
    int bytes_read = Serial.readBytesUntil(';', received_data, sizeof(received_data) - 1);
    received_data[bytes_read] = '\0';  // Ensure null termination

    // Look for "PWM" in the received string
    if (strncmp(received_data, "PWM", 3) == 0) {
      pin_l = atoi(received_data + 3);  // Extract pin number

      // ENSURE VALID PIN
      if (pin_l < 2 || pin_l > 5) {
        Serial.println("Invalid pin! Must be 2, 3, 4 or 5.");
        return;
      }

      int i = findValve(valves, N_VALVES, pin_l);
      if (i >= 0) {
        valves[i].pin = pin_l;
        valves[i].is_on = atoi(received_data + 7);
        valves[i].pwm = atoi(received_data + 9);
        valves[i].period = atoi(received_data + 13);
        Serial.print(" PWM: ");
        Serial.print(valves[i].pwm);
      }
    }
  }
  updateState(valves, N_VALVES);
}
