#pragma once

#include <Arduino.h>

class Logger {
 public:
  // Assignment operators.
  // Logger& operator=(const Logger& other)   {
  //   _stream = other._stream;
  //   return *this;
  // }

  // Set nullptr supress logging.
  void set_stream(Stream* stream) { _stream = stream; }


  void log(const char* format, ...) const {
    if (_stream) {
      va_list ap;
      va_start(ap, format);
      _stream->vprintf(format, ap);
      _stream->println();
      va_end(ap);
    }
  }

 private:
  // Optionally null. Not owning this pointer.
  mutable  Stream* _stream = nullptr;
};