
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>
// #include "packet_encoder.h"


// enum SeriaPacketsEvent {
//   CONNECTED = 1,
//   DISCONNECTED = 2,
// };

class SerialPacketData {
 public:
  SerialPacketData(uint16_t capacity = 64) {
    alloc_buffer(capacity);
  }
  ~SerialPacketData() { release_buffer(); }

  // Disable copying and assignment.
  SerialPacketData(const SerialPacketData& other) = delete;
  SerialPacketData& operator=(const SerialPacketData& other) = delete;

  uint16_t capacity() const { return _capacity; }
  uint16_t size() const { return _size; }
  uint16_t bytes_to_read() const { return _size - _bytes_read; }
  bool all_read() const  { return _bytes_read >= _size; }
  bool read_error() const { return _read_error; }
  bool write_error() const  { return _read_error; }

  bool all_read_ok() { return _bytes_read >= _size && !_read_error; }

  void dump_data(Stream& s);

  void clear() {
    _size = 0;
    _bytes_read = 0;
    _read_error = false;
    _write_error = false;
  }

  void dump(const char* title, Stream& s);

  inline uint16_t bytes_read() { return _bytes_read; }
  inline uint16_t unread_bytes() { return _size - _bytes_read; }
  inline uint16_t free_bytes() { return _capacity - _size; }
  inline bool is_full() { return _size >= _capacity; }
  inline bool is_empty() { return _size == 0; }
  uint16_t crc16();

  bool add_uint8(byte v) {
    if (_write_error || 1 > free_bytes()) {
      _write_error = true;
      return false;
    }
    _buffer[_size++] = v;
    return true;
  }

  bool add_uint16(uint16_t v) {
    if (_write_error || 2 > free_bytes()) {
      _write_error = true;
      return false;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 8;
    p[1] = v >> 0;
    _size += 2;
    return true;
  }

  bool add_uint32(uint32_t v) {
    if (_write_error || 4 > free_bytes()) {
      _write_error = true;
      return false;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v >> 0;
    _size += 4;
    return true;
  }

  bool add_bytes(byte bytes[], uint32_t num_bytes) {
    if (_write_error || num_bytes > free_bytes()) {
      _write_error = true;
      return false;
    }
    // _buffer is available only if _size > 0.
    if (num_bytes > 0) {
      memcpy(&_buffer[_size], bytes, num_bytes);
      _size += num_bytes;
    }
    return true;
  }

  bool add_data(const SerialPacketData& data) {
    if (_write_error || data._size > free_bytes()) {
      _write_error = true;
      return false;
    }
    // _buffer is available only if _size > 0.
    if (data._size > 0) {
      memcpy(&_buffer[_size], data._buffer, data._size);
      _size += data._size;
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

  bool read_data(SerialPacketData& data, uint32_t bytes_to_read) {
    if (_read_error || bytes_to_read > unread_bytes() ||
        bytes_to_read > data.free_bytes()) {
      // Note that we don't set the write error of data.
      _read_error = true;
      return false;
    }
    // _buffer is available only if _size > 0.
    if (bytes_to_read > 0) {
      memcpy(&data._buffer[data._size], &_buffer[_bytes_read],
             bytes_to_read);
      _bytes_read += bytes_to_read;
      data._size += bytes_to_read;
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
 // Buffer size in bytes.
  uint16_t _capacity = 0;
  // Actual data bytes. <= capacity.
  uint16_t _size = 0;
  uint16_t _bytes_read = 0;
  bool _read_error = false;
  bool _write_error = false;
  // Null IFF _buffer_size is zero.
  byte* _buffer = nullptr;

  void release_buffer();
  bool alloc_buffer(uint16_t buffer_size);

  friend class PacketEncoder;
  friend class SerialPacketsClient;
};
