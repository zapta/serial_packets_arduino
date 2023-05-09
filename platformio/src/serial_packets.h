
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>
#include "packet_encoder.h"

constexpr uint16_t MAX_DATA_LEN = 1024;

enum SeriaPacketsEvent {
  CONNECTED = 1,
  DISCONNECTED = 2,
};

class SerialPacketsData {
 public:
  SerialPacketsData(uint16_t max_data_size = 64) {
    alloc_buffer(max_data_size);
  }
  ~SerialPacketsData() { release_buffer(); }

  // Disable copying and assignment.
  SerialPacketsData(const SerialPacketsData& other) = delete;
  SerialPacketsData& operator=(const SerialPacketsData& other) = delete;

  uint16_t capacity() { return _buffer_size; }
  uint16_t data_size() { return _data_size; }
  uint16_t bytes_to_read() { return _data_size - _bytes_read; }
  bool all_read() { return _bytes_read >= _data_size; }
  bool read_error() { return _read_error; }
  bool write_error() { return _read_error; }

  bool all_read_ok() { return _bytes_read >= _data_size && !_read_error; }

  void dump_data(Stream& s);

  void clear() {
    _data_size = 0;
    _bytes_read = 0;
    _read_error = false;
    _write_error = false;
  }

  void dump(const char* title, Stream& s);

  inline uint16_t bytes_read() { return _bytes_read; }
  inline uint16_t unread_bytes() { return _data_size - _bytes_read; }
  inline uint16_t free_bytes() { return _buffer_size - _data_size; }
  inline bool is_full() { return _data_size >= _buffer_size; }
  inline bool is_empty() { return _data_size == 0; }
  uint16_t crc16();

  bool add_uint8(byte v) {
    if (_write_error || 1 > free_bytes()) {
      _write_error = true;
      return false;
    }
    _buffer[_data_size++] = v;
    return true;
  }

  bool add_uint16(uint16_t v) {
    if (_write_error || 2 > free_bytes()) {
      _write_error = true;
      return false;
    }
    uint8_t* p = &_buffer[_data_size];
    p[0] = v >> 8;
    p[1] = v >> 0;
    _data_size += 2;
    return true;
  }

  bool add_uint32(uint32_t v) {
    if (_write_error || 4 > free_bytes()) {
      _write_error = true;
      return false;
    }
    uint8_t* p = &_buffer[_data_size];
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v >> 0;
    _data_size += 4;
    return true;
  }

  bool add_bytes(byte bytes[], uint32_t num_bytes) {
    if (_write_error || num_bytes > free_bytes()) {
      _write_error = true;
      return false;
    }
    // _buffer is available only if _data_size > 0.
    if (num_bytes > 0) {
      memcpy(&_buffer[_data_size], bytes, num_bytes);
      _data_size += num_bytes;
    }
    return true;
  }

  bool add_data(const SerialPacketsData& data) {
    if (_write_error || data._data_size > free_bytes()) {
      _write_error = true;
      return false;
    }
    // _buffer is available only if _data_size > 0.
    if (data._data_size > 0) {
      memcpy(&_buffer[_data_size], data._buffer, data._data_size);
      _data_size += data._data_size;
    }
    return true;
  }

  bool read_byte(byte& v) {
    if (_read_error || 1 > unread_bytes()) {
      _read_error = true;
      return false;
    }
    v = _buffer[_bytes_read++];
    return true;
  }

  bool read_uint16(uint16_t& v) {
    if (_read_error || 2 > unread_bytes()) {
      _read_error = true;
      return false;
    }
    byte* p = _buffer + _bytes_read;
    v = ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
    _bytes_read += 2;
    return true;
  }

  bool read_uint32(uint32_t& v) {
    if (_read_error || 4 > unread_bytes()) {
      _read_error = true;
      return false;
    }
    byte* p = _buffer + _bytes_read;
    v = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
        ((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
    _bytes_read += 4;
    return true;
  }

  bool read_bytes(byte bytes_buffer[], uint32_t bytes_to_read) {
    if (_read_error || bytes_to_read > unread_bytes()) {
      _read_error = true;
      return false;
    }
    // _buffer is available only if _data_size > 0.
    if (bytes_to_read > 0) {
      memcpy(bytes_buffer, &_buffer[_bytes_read], bytes_to_read);
      _bytes_read += bytes_to_read;
    }
    return true;
  }

  bool read_data(SerialPacketsData& data, uint32_t bytes_to_read) {
    if (_read_error || bytes_to_read > unread_bytes() ||
        bytes_to_read > data.free_bytes()) {
      // Note that we don't set the write error of data.
      _read_error = true;
      return false;
    }
    // _buffer is available only if _data_size > 0.
    if (bytes_to_read > 0) {
      memcpy(&data._buffer[data._data_size], &_buffer[_bytes_read],
             bytes_to_read);
      _bytes_read += bytes_to_read;
      data._data_size += bytes_to_read;
    }
    return true;
  }

  bool skip_bytes(uint32_t bytes_to_skip) {
    if (_read_error || bytes_to_skip > unread_bytes()) {
      _read_error = true;
      return false;
    }
    _bytes_read += bytes_to_skip;
    return true;
  }

 private:
  uint16_t _buffer_size = 0;
  uint16_t _data_size = 0;
  uint16_t _bytes_read = 0;
  bool _read_error = false;
  bool _write_error = false;
  // Null IFF _buffer_size is zero.
  byte* _buffer = nullptr;

  void release_buffer();
  bool alloc_buffer(uint16_t buffer_size);

  friend class PacketEncoder;
};

typedef void (*SerialPacketsIncomingCommandHandler)(
    byte command_endpoint, const SerialPacketsData& command_data,
    byte& response_status, SerialPacketsData& response_data);

enum OutcomeCode {
  OUTCOME_RESPONSE,
  OUTCOME_TIMEOUT,
  OUTCOME_INTERNAL_ERROR,
  OUTCOME_CANCELED,
};

typedef void (*SerialPacketsCommandOutcomeHandler)(
    uint32_t cmd_id, OutcomeCode cmd_outcome, byte response_status,
    const SerialPacketsData& response_data);

typedef void (*SerialPacketsIncomingMessageHandler)(
    byte message_endpoint, const SerialPacketsData& message_data);

typedef void (*SerialPacketsEventHandler)(SeriaPacketsEvent event);

class SerialPacketsClient {
 public:
  SerialPacketsClient(
      SerialPacketsIncomingCommandHandler command_handler = nullptr,
      SerialPacketsIncomingMessageHandler message_handler = nullptr,
      SerialPacketsEventHandler event_handler = nullptr)
      : _command_handler(command_handler),
        _message_handler(message_handler),
        _event_handler(event_handler) {}

  void begin(Stream& data_stream, Print& debug_printer);

  void loop();

  bool sendCommand(byte endpoint, const SerialPacketsData& data,

                   SerialPacketsCommandOutcomeHandler handle, uint32_t& cmd_id,
                   uint16_t timeout);

  bool sendMessage(byte message_endpoint,
                   const SerialPacketsData& message_data);

 private:
  // Callback handlers. Set by the constructor.
  SerialPacketsIncomingCommandHandler const _command_handler;
  SerialPacketsIncomingMessageHandler const _message_handler;
  SerialPacketsEventHandler const _event_handler;

  // Serial ports. Set in begin()
  Stream* _data_stream = nullptr;
  Print* _debug_printer = nullptr;

  SerialPacketsData _tmp_data;
  PacketEncoder _packet_encoder;

  // Used to assign command ids. Wraparound is ok. Skipping zero value.
  uint32_t _cmd_id_counter = 0;
  // Used to insert pre flag when packates are sparse.
  Elapsed _pre_flag_timer;
};
