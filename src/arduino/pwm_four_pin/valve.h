#ifndef _VALVE_H
#define _VALVE_H

constexpr int MAX_VALVES = 4;
constexpr int N_VALVES = 4;

struct Valve {
  int pin;
  bool is_on = false;
  int pwm = 50;
  unsigned long period = 10000;
  unsigned long lastswitch;
};

void addValve(Valve valves[], unsigned int i_valve, int pin, bool is_on, int pwm, unsigned long period) {
  if (i_valve >= MAX_VALVES) return;

  Valve &v = valves[i_valve];

  pinMode(pin, OUTPUT);
  delay(10);
  digitalWrite(pin, LOW);

  v.pin = pin;
  v.is_on = is_on;
  v.pwm = pwm;
  v.period = period;
  v.lastswitch = millis();
}

void initValves(Valve valves[], unsigned int n_valves, unsigned int pins[], int pwm = 50, unsigned long period = 10000) {
  for (int i = 0; i < n_valves; i++) {
    addValve(valves, i, pins[i], false, pwm, period);
  }
}

int findValve(Valve valves[], unsigned int n_valves, int pin) {
  for (int i = 0; i < n_valves; i++) {
    if (valves[i].pin == pin) {
      return i;
    }
  }
  return -1;
}

void updateState(Valve valves[], unsigned int n_valves) {
  // PWM Logic
  unsigned long now = millis();
  long elapsed;

  for (int i = 0; i < n_valves; i++) {
    if (valves[i].is_on == 1) {
      elapsed = now - valves[i].lastswitch;

      if (elapsed > valves[i].period) {
        valves[i].lastswitch = now;
      } else if (elapsed < (valves[i].period * valves[i].pwm) / 100) {
        digitalWrite(valves[i].pin, HIGH);
      } else {
        digitalWrite(valves[i].pin, LOW);
      }
    } else {
      valves[i].lastswitch = now;
    }
  }
}

#endif