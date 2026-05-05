#ifndef _VALVE_H
#define _VALVE_H

struct Valve {
  int pin;
  bool is_on = false;
  int pwm = 50;
  unsigned long period = 10000;
  unsigned long lastswitch;
};

Valve *valves = nullptr;
unsigned int n_valves = 0;

void addValve(int pin, bool is_on, int pwm, unsigned long period) {
  n_valves++;

  // (RE)ALLOCATE MEMORY
  valves = (Valve *)realloc(valves, n_valves * sizeof(Valve));
  // if (valves != 0) {
  //   valves = (Valve *)realloc(valves, n_valves * sizeof(Valve));
  // } else {
  //   valves = (Valve *)malloc(n_valves * sizeof(Valve));
  // }

  valves[n_valves - 1].pin = pin;
  valves[n_valves - 1].is_on = is_on;
  valves[n_valves - 1].pwm = pwm;
  valves[n_valves - 1].period = period;
  valves[n_valves - 1].lastswitch = millis();
}

int findValve(int pin) {
  for (int i = 0; i < n_valves; i++) {
    if (valves[i].pin == pin) {
      return i;
    }
  }
  return -1;
}

void updateState() {
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