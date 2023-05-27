// A simple logger to an optional serial stream.

#pragma once

#include <Arduino.h>

// namespace serial_packets {

class SerialPacketsLogger {
 public:
  enum Level { VERBOSE = 1, INFO = 2, WARNING = 3, ERROR = 4, NONE = 5 };

  SerialPacketsLogger() : _level(INFO) {}

  SerialPacketsLogger(Level level) : _level(constrain_level(level)) {}

  // Setting to nullptr equivalent to no logging.
  void set_stream(Stream* stream) { _optional_stream = stream; }

  void set_level(Level level) { level = constrain_level(level); }

  Level level() { return _level; }

  inline bool is_level(Level level) const { return level >= _level; }

  inline bool is_verbose() const { return is_level(VERBOSE); }

  inline bool is_info() const { return is_level(INFO); }

  inline bool is_warning() const { return is_level(WARNING); }

  inline bool is_error() const { return is_level(ERROR); }

  inline bool is_none() const { return is_level(NONE); }

  static Level constrain_level(Level level) {
    return constrain(level, VERBOSE, NONE);
  }

  // Log at verbose level.
  void verbose(const char* format, ...) const {
    if (_optional_stream && is_verbose()) {
      va_list ap;
      va_start(ap, format);
      _vlog("V", format, ap);
      va_end(ap);
    }
  }

  // Log at info level.
  void info(const char* format, ...) const {
    if (_optional_stream && is_info()) {
      va_list ap;
      va_start(ap, format);
      _vlog("I", format, ap);
      va_end(ap);
    }
  }

  // Log at warning level.
  void warning(const char* format, ...) const {
    if (_optional_stream && is_warning()) {
      va_list ap;
      va_start(ap, format);
      _vlog("W", format, ap);
      va_end(ap);
    }
  }

  // Log at error level.
  void error(const char* format, ...) const {
    if (_optional_stream && is_error()) {
      va_list ap;
      va_start(ap, format);
      _vlog("E", format, ap);
      va_end(ap);
    }
  }

 private:
  // Null if not availble. Not the owner of this pointer.
  mutable Stream* _optional_stream = nullptr;

  Level _level = VERBOSE;

  // _optional_stream should be checked by the caller to be non null.
  void _vlog(const char* level_str, const char* format, va_list args) const {
    _optional_stream->print(level_str);
    _optional_stream->print(": ");
    _optional_stream->vprintf(format, args);
    _optional_stream->println();
  }
};

// } // namespace serial_packets