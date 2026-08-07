#ifndef _VALVE_H
#define _VALVE_H

constexpr int MAX_VALVES = 4;
constexpr int N_VALVES = 4;

struct Valve {
  int pin;
  bool is_active = false;
  int pwm = 50;
  unsigned long period = 10000;
  unsigned long pulse_window;
  unsigned long lastswitch;
};

void addValve(Valve valves[], unsigned int i_valve, int pin, bool is_active, int pwm, unsigned long period) {
  if (i_valve >= MAX_VALVES) return;

  Valve &v = valves[i_valve];

  pinMode(pin, OUTPUT);
  delay(10);
  digitalWrite(pin, LOW);

  v.pin = pin;
  v.is_active = is_active;
  v.pwm = pwm;
  v.period = period;
  v.pulse_window = (v.period * v.pwm) / 100.0;
  v.lastswitch = millis();
}

void initValves(Valve valves[], unsigned int n_valves, unsigned int pins[], int pwm = 50, unsigned long period = 10000) {
  for (int i = 0; i < n_valves; i++) {
    addValve(valves, i, pins[i], false, pwm, period);
  }
}

int findValve(Valve valves[], unsigned int n_valves, int pin) {
  for (int i = 0; i < n_valves; i++) {
    if (valves[i].pin == pin) { return i; }
  }
  return -1;
}

void printValveStatus(Valve v, int i_valve) {
  Serial.print("[Setting Valve ");
  Serial.print(i_valve + 1);
  Serial.println("]");
  Serial.println("—————————————————");

  Serial.print("-    PWM: ");
  Serial.print(v.pwm);
  Serial.println(" %");

  Serial.print("- Period: ");
  Serial.print(v.period);
  Serial.println(" ms");

  Serial.print("- Active: ");
  Serial.println(v.is_active ? "true" : "false");
  Serial.println("—————————————————");
  Serial.println();
}

void updateState(char data_received[], Valve valves[], unsigned int n_valves, unsigned int buffer_length) {
  if (Serial.available() <= 0) { return; }

  int bytes_read = Serial.readBytesUntil(';', data_received, buffer_length - 1);
  data_received[bytes_read] = '\0';  // Ensure null termination
  Serial.print("[Received Data] \"");
  Serial.print(data_received);
  Serial.println("\"");

  // Look for "PWM" in the received string
  if (strncmp(data_received, "PWM", 3) == 0) {
    int pin_received = atoi(data_received + 3);  // Extract pin number

    // ENSURE VALID PIN
    if (pin_received < 2 || pin_received > 5) {
      Serial.print("Invalid pin = ");
      Serial.print(pin_received);
      Serial.println("! Must be 2, 3, 4 or 5.");
      Serial.println();
      return;
    }

    int i_valve = findValve(valves, n_valves, pin_received);
    if (i_valve < 0) { return; }
    Valve &v = valves[i_valve];
    v.is_active = atoi(data_received + 7);
    v.pwm = atoi(data_received + 9);
    v.period = atoi(data_received + 13);
    v.pulse_window = (v.period * v.pwm) / 100.0;

    printValveStatus(v, i_valve);
  }
}

/*
  PWM Logic
*/
void updateActuator(Valve valves[], unsigned int n_valves) {
  unsigned long now = millis();

  for (int i = 0; i < n_valves; i++) {
    Valve &v = valves[i];
    unsigned long elapsed = now - v.lastswitch;
    bool exceeded_period = v.is_active && elapsed > v.period;
    bool within_pulse_window = v.is_active && elapsed < v.pulse_window;
    if (exceeded_period || !v.is_active) {
      v.lastswitch = now;
    } else if (within_pulse_window) {
      digitalWrite(v.pin, HIGH);  // far-side inlet is open (non-normal)
    } else {
      digitalWrite(v.pin, LOW);  // pin-side inlet is open (normal)
    }
  }
}

#endif
