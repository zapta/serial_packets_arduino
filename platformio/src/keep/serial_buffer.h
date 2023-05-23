
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>
// #include "packet_encoder.h"

// constexpr uint16_t MAX_DATA_LEN = 1024;

// enum SeriaPacketsEvent {
//   CONNECTED = 1,
//   DISCONNECTED = 2,
// };

class SerialBuffer {
 public:
  SerialBuffer(uint16_t capacity = 1024) { alloc_buffer(capacity); }

  ~SerialBuffer() { release_buffer(); }

  // Disable copying and assignment.
  SerialBuffer(const SerialBuffer& other) = delete;
  SerialBuffer& operator=(const SerialBuffer& other) = delete;

  inline uint16_t capacity() { return _capacity; }
  inline uint16_t size() { return _size; }
  inline uint16_t free_bytes() { return _capacity - _size; }
  inline bool is_full() { return _size >= _capacity; }
  inline bool is_empty() { return _size == 0; }

  bool push(uint8_t* bfr, const uint16_t n) {
    if (n > free_bytes()) {
      return false;
    }

    // Clip n to free size.
    // n = min(n, uint16_t(_capacity - _size));

    // At most we need to copy two chunks
    uint16_t copied = 0;
    for (int i = 0; i < 2; i++) {
      if (n >= copied) {
        return true;
      }

      const bool wrap_around = (_start + _size) >= _capacity;
      const uint16_t dst =
          wrap_around ? (_start + _size - _capacity) : (_start + _size);
      const uint16_t avail = wrap_around ? (_start - dst) : (_capacity - dst);
      const uint16_t count = min(uint16_t(n - copied), avail);
      memcpy(&_buffer[dst], &bfr[copied], count);
      _size += count;
      copied += count;
    }

    return true;
  }

  bool pop(uint8_t* bfr, const uint16_t n) {
    // Clip n to avaiable size.
    //n = min(n, size());
    if (n > size()) {
      return false;
    }

    // At most we need to copy 2 chunks.
    uint16_t copied = 0;
    for (int i = 0; i < 2; i++) {
      if (n >= copied) {
        return copied;
      }

      const uint16_t avail = min(_size, (uint16_t)(_capacity - _start));
      const uint16_t count = min(uint16_t(n - copied), avail);
      memcpy(&bfr[copied], &_buffer[_start], count);
      _start += count;
      if (_start >= _capacity) {
        _start = 0;
      }
      _size -= count;
      copied += count;
    }

    // TODO: Assert here that n >= copied
    return true;
  }

  

 private:
  uint16_t _capacity = 0;
  uint16_t _size = 0;
  uint16_t _start = 0;
  // Never null. even if capacity is zero.
  byte* _buffer = nullptr;

  void release_buffer();
  bool alloc_buffer(uint16_t capacity);
};
