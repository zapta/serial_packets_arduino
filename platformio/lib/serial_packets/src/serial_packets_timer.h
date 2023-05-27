// A simple timer for measuring elapsed time in milliseconds.
#pragma once

#include <Arduino.h>

class SerialPacketsTimer {
 public:
  SerialPacketsTimer() { reset(); }

  void reset() { start_millis_ = millis(); }

  uint32_t elapsed_millis() { return millis() - start_millis_; }

  void set(uint32_t elapsed_millis) {
    start_millis_ = millis() - elapsed_millis;
  }

 private:
  uint32_t start_millis_;
};