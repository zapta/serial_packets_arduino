#pragma once

#include <Arduino.h>

class PacketLogger {
 public:
  enum Level { VERBOSE = 1, INFO = 2, WARNING = 3, ERROR = 4, NONE = 5 };

  // Setting to nullptr equivalent to no logging.
  void set_stream(Stream* stream) { _stream = stream; }

  void set_level(Level level) { level = constrain(level, VERBOSE, NONE); }

  inline bool is_level(Level level) const { return level >= _level; }

  inline bool is_verbose() const { return is_level(VERBOSE); }

  inline bool is_info() const { return is_level(INFO); }

  inline bool is_warning() const { return is_level(WARNING); }

  inline bool is_error() const { return is_level(ERROR); }

  inline bool is_none() const { return is_level(NONE); }



  void verbose(const char* format, ...) const {
    if (_stream && is_verbose()) {
      va_list ap;
      va_start(ap, format);
      _vlog("V", format, ap);
      va_end(ap);
    }
  }

  void info(const char* format, ...) const {
    if (_stream && is_info()) {
      va_list ap;
      va_start(ap, format);
      _vlog("I", format, ap);
      va_end(ap);
    }
  }

    void warning(const char* format, ...) const {
    if (_stream && is_warning()) {
      va_list ap;
      va_start(ap, format);
      _vlog("W", format, ap);
      va_end(ap);
    }
  }

  void error(const char* format, ...) const {
    if (_stream && is_error()) {
      va_list ap;
      va_start(ap, format);
      _vlog("E", format, ap);
      va_end(ap);
    }
  }

  // Temp for testing.
  // void log(const char* format, ...) const {
  //   if (_stream ) {
  //     va_list ap;
  //     va_start(ap, format);
  //     _vlog("XXX", format, ap);
  //     va_end(ap);
  //   }
  // }

  // void log(const char* format, ...) const {
  //   if (_stream) {
  //     va_list ap;
  //     va_start(ap, format);
  //     _stream->vprintf(format, ap);
  //     _stream->println();
  //     va_end(ap);
  //   }
  // }

 private:
  // Optionally null. Not owning this pointer.
  mutable Stream* _stream = nullptr;
  // Will log only levels >= this one.
  Level _level = WARNING;

  // _stream should be checked by the caller to be non null.
  void _vlog(const char* level_str, const char* format, va_list args) const {
    // if (_stream) {
    // va_list ap;
    // va_start(ap, format);
    _stream->print(level_str);
    _stream->print(": ");
    _stream->vprintf(format, args);
    _stream->println();
    // va_end(ap);
  }
};