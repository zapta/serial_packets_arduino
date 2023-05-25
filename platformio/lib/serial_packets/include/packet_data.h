
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>
// #include "packet_encoder.h"

// enum SeriaPacketsEvent {
//   CONNECTED = 1,
//   DISCONNECTED = 2,
// };

class PacketData {
 public:
  PacketData(uint16_t capacity = 64) { alloc_buffer(capacity); }
  ~PacketData() { release_buffer(); }

  // Disable copying and assignment.
  PacketData(const PacketData& other) = delete;
  PacketData& operator=(const PacketData& other) = delete;

  inline uint16_t capacity() const { return _capacity; }
  inline uint16_t size() const { return _size; }
  inline uint16_t bytes_left_to_read() const { return _size - _bytes_read; }
  inline bool all_read() const { return _bytes_read >= _size; }
  inline bool read_errors() const { return _read_errors; }
  inline bool write_errors() const { return _read_errors; }
  inline bool all_read_ok() const { return all_read() && !read_errors(); }

  void clear() {
    _size = 0;
    _write_errors = false;
    reset_reading();
    // _bytes_read = 0;
    // _read_error = false;
    // _write_error = false;
  }

  void dump(const char* title, Stream& s) const;

  inline uint16_t bytes_read() const { return _bytes_read; }
  inline uint16_t unread_bytes() const { return _size - _bytes_read; }
  inline void reset_reading() const {
    _bytes_read = 0;
    _read_errors = false;
  }

  inline uint16_t free_bytes() const { return _capacity - _size; }
  inline bool is_full() const { return _size >= _capacity; }
  inline bool is_empty() const { return _size == 0; }
  uint16_t crc16() const;

  void write_uint8(byte v) {
    if (_write_errors || 1 > free_bytes()) {
      _write_errors = true;
      return;
    }
    _buffer[_size++] = v;
  }

  void write_uint16(uint16_t v) {
    if (_write_errors || 2 > free_bytes()) {
      _write_errors = true;
      return ;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 8;
    p[1] = v >> 0;
    _size += 2;
  }

  void write_uint32(uint32_t v) {
    if (_write_errors || 4 > free_bytes()) {
      _write_errors = true;
      return ;
    }
    uint8_t* p = &_buffer[_size];
    p[0] = v >> 24;
    p[1] = v >> 16;
    p[2] = v >> 8;
    p[3] = v >> 0;
    _size += 4;
    // return true;
  }

  void write_bytes(byte bytes[], uint32_t num_bytes) {
    if (_write_errors || num_bytes > free_bytes()) {
      _write_errors = true;
      return ;
    }
    // _buffer is available only if _size > 0.
    if (num_bytes > 0) {
      memcpy(&_buffer[_size], bytes, num_bytes);
      _size += num_bytes;
    }
    // return true;
  }

  void write_data(const PacketData& source) {
    if (_write_errors || source._size > free_bytes()) {
      _write_errors = true;
      return ;
    }
    // _buffer is available only if _size > 0.
    if (source._size > 0) {
      memcpy(&_buffer[_size], source._buffer, source._size);
      _size += source._size;
    }
    // return true;
  }

  uint8_t read_uint8() const {
    if (_read_errors || 1 > unread_bytes()) {
      _read_errors = true;
      return 0;
    }
    return _buffer[_bytes_read++];
  }

  uint16_t read_uint16() const {
    if (_read_errors || 2 > unread_bytes()) {
      _read_errors = true;
      return false;
    }
    byte* p = _buffer + _bytes_read;
    _bytes_read += 2;

    return ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
  }

  uint32_t read_uint32() const {
    if (_read_errors || 4 > unread_bytes()) {
      _read_errors = true;
      return 0;
    }
    byte* p = _buffer + _bytes_read;
    _bytes_read += 4;
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
  }

  void read_bytes(byte bytes_buffer[], uint32_t bytes_to_read) const {
    if (_read_errors || bytes_to_read > unread_bytes()) {
      memset(bytes_buffer, 0, bytes_to_read);
      _read_errors = true;
      return;
    }
    // _buffer is available only if _data_size > 0.
    if (bytes_to_read > 0) {
      memcpy(bytes_buffer, &_buffer[_bytes_read], bytes_to_read);
      _bytes_read += bytes_to_read;
    }
    // return true;
  }

  // Append read bytes to destination data.
  void read_data(PacketData& destination, uint32_t bytes_to_read) const {
    if (_read_errors || bytes_to_read > unread_bytes() ||
        bytes_to_read > destination.free_bytes()) {
      // Note that we don't set the write error of the destinationdata.
      _read_errors = true;
      return;
    }
    // _buffer is available only if _size > 0.
    if (bytes_to_read > 0) {
      memcpy(&destination._buffer[destination._size], &_buffer[_bytes_read],
             bytes_to_read);
      _bytes_read += bytes_to_read;
      destination._size += bytes_to_read;
    }
    // return ;
  }

  void skip_bytes(uint32_t bytes_to_skip) const {
    if (_read_errors || bytes_to_skip > unread_bytes()) {
      _read_errors = true;
      return;
    }
    _bytes_read += bytes_to_skip;
    // return true;
  }

 private:
  // Buffer size in bytes.
  uint16_t _capacity = 0;
  // Actual data bytes. <= capacity.
  uint16_t _size = 0;
  // Null IFF _buffer_size is zero.
  byte* _buffer = nullptr;

  mutable uint16_t _bytes_read = 0;
  mutable bool _read_errors = false;
  mutable bool _write_errors = false;

  void release_buffer();
  bool alloc_buffer(uint16_t buffer_size);

  friend class PacketEncoder;
  friend class SerialPacketsClient;
};
