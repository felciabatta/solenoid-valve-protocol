#ifndef _VALVE_H
#define _VALVE_H

#include "types.h"

constexpr size_t N_VALVES = 4;
constexpr ValveID VALVEID_MIN = 1;
constexpr ValveID VALVEID_MAX = N_VALVES;
constexpr pin_size_t VALVEPIN_MIN = 2;
constexpr pin_size_t VALVEPIN_MAX = 5;
constexpr DutyCycle DUTY_OPEN = 100;
constexpr DutyCycle DUTY_CLOSED = 0;
constexpr DutyCycle DUTY_DEFAULT = 0;
constexpr TimeMillis PERIOD_DEFAULT = 1000;
constexpr char NULL_CHR = '\0';

struct Valve {
  ValveID id;
  pin_size_t pin;
  OnOff enable = false;
  DutyCycle duty_cycle = DUTY_DEFAULT;
  TimeMillis period = PERIOD_DEFAULT;
  TimeMillis pulse_window;
  TimeMillis period_start;
  PinStatus status = LOW;
};

void initValve(Valve valves[], ValveID id_valve, pin_size_t pin, DutyCycle duty_cycle, TimeMillis period) {
  if (id_valve < VALVEID_MIN || id_valve > VALVEID_MAX) return;

  Valve &v = valves[id_valve - VALVEID_MIN];

  pinMode(pin, OUTPUT);
  delay(10);
  digitalWrite(pin, LOW);

  v.id = id_valve;
  v.pin = pin;
  v.enable = false;
  v.duty_cycle = duty_cycle;
  v.period = period;
  v.pulse_window = (v.period * v.duty_cycle) / 100.0;
  v.period_start = millis();
}

void initValves(Valve valves[], size_t n_valves, pin_size_t pins[], DutyCycle duty_cycle = DUTY_DEFAULT, TimeMillis period = PERIOD_DEFAULT) {
  for (size_t idx = 0; idx < n_valves; idx++) {
    initValve(valves, idx, pins[idx], duty_cycle, period);
  }
}

int valvePin2Index(Valve valves[], size_t n_valves, pin_size_t pin) {
  for (size_t idx = 0; idx < n_valves; idx++) {
    if (valves[idx].pin == pin) { return idx; }
  }
  return -1;
}

int valveID2Index(Valve valves[], size_t n_valves, ValveID id) {
  for (size_t idx = 0; idx < n_valves; idx++) {
    if (valves[idx].id == id) { return idx; }
  }
  return -1;
}

int buffer2ValveIndex(Valve valves[], size_t n_valves, char data_buffer[]) {
  // EXTRACT ID AND IDX
  pin_size_t id_received = atoi(data_buffer + 3);
  int idx_valve = valveID2Index(valves, n_valves, id_received);

  // VALIDATE ID
  if (idx_valve < 0) {
    Serial.print("Invalid ID = ");
    Serial.print(id_received);
    Serial.println("! Must be 1, 2, 3 or 4.");
    Serial.println();
  }

  return idx_valve;
}

void printValveStatus(Valve v) {
  Serial.print("[Setting Valve ");
  Serial.print(v.id);
  Serial.println("]");
  Serial.println("—————————————————");

  Serial.print("-    PWM: ");
  Serial.print(v.duty_cycle);
  Serial.println(" %");

  Serial.print("- Period: ");
  Serial.print(v.period);
  Serial.println(" ms");

  Serial.print("- Active: ");
  Serial.println(v.enable ? "true" : "false");
  Serial.println("—————————————————");
  Serial.println();
}

void updateBuffer(char data_buffer[], size_t buffer_length) {
  int bytes_read = Serial.readBytesUntil(';', data_buffer, buffer_length - 1);
  data_buffer[bytes_read] = NULL_CHR;  // Ensure null termination
  Serial.print("[Received Data] \"");
  Serial.print(data_buffer);
  Serial.println("\"");
}

// parseCommand

void updateCommand(Valve &v, OnOff enable, DutyCycle D, TimeMillis T) {
  v.enable = enable;
  v.duty_cycle = D;
  v.period = T;
  // window where valve is powered on
  v.pulse_window = (v.period * v.duty_cycle) / 100.0;
  printValveStatus(v);
}

void updateCommand(char data_buffer[], Valve valves[], size_t n_valves) {
  // Look for "PWM" in buffer
  if (strncmp(data_buffer, "PWM", 3) != 0) { return; }

  int idx_valve = buffer2ValveIndex(valves, n_valves, data_buffer);
  if (idx_valve < 0) { return; }

  OnOff enable = atoi(data_buffer + 7);
  DutyCycle duty_cycle = atoi(data_buffer + 9);
  TimeMillis period = atoi(data_buffer + 13);

  updateCommand(valves[idx_valve], enable, duty_cycle, period);
}

void updateCommand(char data_buffer[], Valve valves[], size_t n_valves, size_t buffer_length){
  if (Serial.available() <= 0) { return; }
  updateBuffer(data_buffer, buffer_length);
  updateCommand(data_buffer, valves, n_valves);
}

void resetStatus(Valve& v, TimeMillis now) {
  v.period_start = now;
  v.status = v.enable && v.duty_cycle > DUTY_CLOSED ? HIGH : LOW;
}

void resetStatus(Valve& v) {
  resetStatus(v, millis());
}

void updateStatus(Valve& v, TimeMillis now) {
  TimeMillis elapsed = now - v.period_start;

  // force reset in unlikely event the program is out of sync
  if (elapsed >= 2 * v.period || now < v.period_start) {
    return resetStatus(v);
  } else if (elapsed >= v.period) {
    v.period_start += v.period;
  }

  // pulse window is front of duty cycle
  // Note that `elasped % period in {0 ... period-1}`, so
  // - `duty_cycle ==   0 =>` always LOW (since window == 0)
  // - `duty_cycle == 100` => always HIGH (since window == period)
  bool within_pulse_window = elapsed % v.period < v.pulse_window;
  v.status = v.enable && within_pulse_window ? HIGH : LOW;
}

void updateStatus(Valve& v) {
  updateStatus(v, millis());
}

void updateStatus(Valve valves[], size_t n_valves) {
  for (size_t idx = 0; idx < n_valves; idx++) {
    Valve &v = valves[idx];
    updateStatus(v);
  }
}

/*
  PWM Logic
*/
void updateActuator(Valve &v) {
  digitalWrite(v.pin, v.status);
  // HIGH — far-side inlet is open (non-normal)
  // LOW — pin-side inlet is open (normal)
  Serial.print("[Pin ");
  Serial.print(v.pin);
  Serial.println(v.status == HIGH ? " HIGH]" : " LOW]");
}

void updateActuator(Valve valves[], size_t n_valves) {
  for (size_t idx = 0; idx < n_valves; idx++) {
    Valve &v = valves[idx];
    updateActuator(v);
  }
}

#endif
