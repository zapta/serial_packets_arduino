// A simple logger to an optional serial stream.

#pragma once

#include <Arduino.h>

// namespace serial_packets {

enum SerialPacketsLogLevel {
  SERIAL_PACKETS_LOG_VERBOSE = 1,
  SERIAL_PACKETS_LOG_INFO = 2,
  SERIAL_PACKETS_LOG_WARNING = 3,
  SERIAL_PACKETS_LOG_ERROR = 4,
  SERIAL_PACKETS_LOG_NONE = 5
};

class SerialPacketsLogger {
 public:
  // enum Level { VERBOSE = 1, INFO = 2, WARNING = 3, ERROR = 4, NONE = 5 };

  SerialPacketsLogger() : _level(SERIAL_PACKETS_LOG_INFO) {}

  SerialPacketsLogger(SerialPacketsLogLevel level) : _level(constrain_level(level)) {}

  // Setting to nullptr equivalent to no logging.
  void set_stream(Stream* stream) { _optional_stream = stream; }

  void set_level(SerialPacketsLogLevel level) { _level = constrain_level(level); }

  SerialPacketsLogLevel level() { return _level; }

  inline bool is_level(SerialPacketsLogLevel level) const { return level >= _level; }

  inline bool is_verbose() const { return is_level(SERIAL_PACKETS_LOG_VERBOSE); }

  inline bool is_info() const { return is_level(SERIAL_PACKETS_LOG_INFO); }

  inline bool is_warning() const { return is_level(SERIAL_PACKETS_LOG_WARNING); }

  inline bool is_error() const { return is_level(SERIAL_PACKETS_LOG_ERROR); }

  inline bool is_none() const { return is_level(SERIAL_PACKETS_LOG_NONE); }

  static SerialPacketsLogLevel constrain_level(SerialPacketsLogLevel level) {
    return constrain(level, SERIAL_PACKETS_LOG_VERBOSE, SERIAL_PACKETS_LOG_NONE);
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

  SerialPacketsLogLevel _level = SERIAL_PACKETS_LOG_VERBOSE;

  // _optional_stream should be checked by the caller to be non null.
  void _vlog(const char* level_str, const char* format, va_list args) const {
    _optional_stream->print(level_str);
    _optional_stream->print(": ");
    _optional_stream->vprintf(format, args);
    _optional_stream->println();
  }
};

// } // namespace serial_packets