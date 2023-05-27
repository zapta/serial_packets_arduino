// A buffer for serialization/deserialization of packet data.

#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"

class SerialPacketsData {
 public:
  SerialPacketsData() {}
  ~SerialPacketsData() {}

  // Disable copying and assignment, to avoid unintentinal overhead.
  // These buffers can be large.
  SerialPacketsData(const SerialPacketsData& other) = delete;
  SerialPacketsData& operator=(const SerialPacketsData& other) = delete;

  inline uint16_t capacity() const { return sizeof(_buffer); }
  inline uint16_t size() const { return _size; }
  inline uint16_t bytes_to_read() const { return _size - _bytes_read; }
  inline bool all_read() const { return _bytes_read >= _size; }
  inline bool had_read_errors() const { return _had_read_errors; }
  inline bool had_write_errors() const { return _had_write_errors; }
  inline uint16_t bytes_read() const { return _bytes_read; }
  inline uint16_t unread_bytes() const { return _size - _bytes_read; }
  inline uint16_t free_bytes() const { return sizeof(_buffer) - _size; }
  inline bool is_full() const { return _size >= sizeof(_buffer); }
  inline bool is_empty() const { return _size == 0; }

  inline bool all_read_ok() const { return all_read() && !had_read_errors(); }

  void clear() {
    _size = 0;
    _had_write_errors = false;
    reset_reading();
  }

  inline void reset_reading() const {
    _bytes_read = 0;
    _had_read_errors = false;
  }

  // Write content to a serial port or another stream.
  void dump(const char* title, Stream& s) const;

  // Compute the data's CRC.
  uint16_t crc16() const;

  void write_uint8(byte v) {
    if (_had_write_errors || 1 > free_bytes()) {
      _had_write_errors = true;
      return;
    }
    _buffer[_size++] = v;
  }

  void write_uint16(uint16_t v) {
    if (_had_write_errors || 2 > free_bytes()) {
      _had_write_errors = true;
      return;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 8;
    p[1] = v >> 0;
    _size += 2;
  }

  void write_uint32(uint32_t v) {
    if (_had_write_errors || 4 > free_bytes()) {
      _had_write_errors = true;
      return;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v >> 0;
    _size += 4;
  }

  void write_bytes(const byte bytes[], uint32_t num_bytes) {
    if (_had_write_errors || num_bytes > free_bytes()) {
      _had_write_errors = true;
      return;
    }
    memcpy(&_buffer[_size], bytes, num_bytes);
    _size += num_bytes;
  }

  void write_data(const SerialPacketsData& source) {
    if (_had_write_errors || source._size > free_bytes()) {
      _had_write_errors = true;
      return;
    }
    memcpy(&_buffer[_size], source._buffer, source._size);
    _size += source._size;
  }

  uint8_t read_uint8() const {
    if (_had_read_errors || 1 > unread_bytes()) {
      _had_read_errors = true;
      return 0;
    }
    return _buffer[_bytes_read++];
  }

  uint16_t read_uint16() const {
    if (_had_read_errors || 2 > unread_bytes()) {
      _had_read_errors = true;
      return false;
    }
    const uint8_t* p = _buffer + _bytes_read;
    _bytes_read += 2;

    return ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
  }

  uint32_t read_uint32() const {
    if (_had_read_errors || 4 > unread_bytes()) {
      _had_read_errors = true;
      return 0;
    }
    const byte* p = _buffer + _bytes_read;
    _bytes_read += 4;
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
  }

  void read_bytes(byte bytes_buffer[], uint32_t bytes_to_read) const {
    if (_had_read_errors || bytes_to_read > unread_bytes()) {
      memset(bytes_buffer, 0, bytes_to_read);
      _had_read_errors = true;
      return;
    }
    memcpy(bytes_buffer, &_buffer[_bytes_read], bytes_to_read);
    _bytes_read += bytes_to_read;
  }

  // Append read bytes to destination data.
  void read_data(SerialPacketsData& destination, uint32_t bytes_to_read) const {
    if (_had_read_errors || bytes_to_read > unread_bytes() ||
        bytes_to_read > destination.free_bytes()) {
      // Note that we don't set the write error of the destination data.
      _had_read_errors = true;
      return;
    }

    memcpy(&destination._buffer[destination._size], &_buffer[_bytes_read],
           bytes_to_read);
    _bytes_read += bytes_to_read;
    destination._size += bytes_to_read;
  }

  void skip_bytes(uint32_t bytes_to_skip) const {
    if (_had_read_errors || bytes_to_skip > unread_bytes()) {
      _had_read_errors = true;
      return;
    }
    _bytes_read += bytes_to_skip;
  }

 private:
  // The encoder and decoder access internal functionality for perofrmance.
  friend class SerialPacketsEncoder;
  friend class SerialPacketsClient;

  uint16_t _size = 0;
  uint8_t _buffer[MAX_PACKET_DATA_LEN];

  mutable uint16_t _bytes_read = 0;
  mutable bool _had_read_errors = false;
  mutable bool _had_write_errors = false;
};
