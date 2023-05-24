#include "serial_packets.h"
#include "packet_crc.h"

void SerialPacketData::dump_data(Stream& s) {
  s.println("SerialPacketsData dump TBD");
}

void SerialPacketData::release_buffer() {
  if (_buffer) {
    delete[] _buffer;
    _buffer = nullptr;
  }
  _capacity = 0;
  _size = 0;
  _bytes_read = 0;
}

bool SerialPacketData::alloc_buffer(uint16_t capacity) {
  release_buffer();
  // For zero capacity we don't allocate a buffer.
  if (capacity == 0) {
    return true;
  }

  // Sanity check the requested size.
  if (capacity > 1024) {
    return false;
  }

  // NOTE: The () zeros the array for determinism.
  _buffer = new byte[capacity]();
  if (!_buffer) {
    return false;
  }
  _capacity = capacity;
  return true;
}

void SerialPacketData::dump(const char* title, Stream& s) {
  s.println(title);
  s.print("  size: ");
  s.println(_size);
  s.print("  bytes read: ");
  s.print(_bytes_read);
  if (_read_error) {
    s.print(" (error)");
  }
  s.println();
  s.print("  capacity: ");
  s.println(_capacity);
  s.print("  data:");
  for (uint16_t i = 0; i < _size; i++) {
    s.print(' ');
    s.print(_buffer[i], HEX);
  }
  s.println();
}

  uint16_t SerialPacketData::crc16() {
    if (!_size) {
      return 0;
    }
    return gen_crc16(_buffer, _size);
  }



