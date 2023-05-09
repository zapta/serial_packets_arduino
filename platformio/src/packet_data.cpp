#include "serial_packets.h"
#include "crc.h"

void SerialPacketsData::dump_data(Stream& s) {
  s.println("SerialPacketsData dump TBD");
}

void SerialPacketsData::release_buffer() {
  if (_buffer) {
    delete[] _buffer;
    _buffer = nullptr;
  }
  _buffer_size = 0;
  _data_size = 0;
  _bytes_read = 0;
}

bool SerialPacketsData::alloc_buffer(uint16_t buffer_size) {
  release_buffer();
  // For zero capacity we don't allocate a buffer.
  if (buffer_size == 0) {
    return true;
  }

  // Sanity check the requested size.
  if (buffer_size > 1024) {
    return false;
  }

  // NOTE: The () zeros the array for determinism.
  _buffer = new byte[buffer_size]();
  if (!_buffer) {
    return false;
  }
  _buffer_size = buffer_size;
  return true;
}

void SerialPacketsData::dump(const char* title, Stream& s) {
  s.println(title);
  s.print("  size: ");
  s.println(_data_size);
  s.print("  bytes read: ");
  s.print(_bytes_read);
  if (_read_error) {
    s.print(" (error)");
  }
  s.println();
  s.print("  capacity: ");
  s.println(_buffer_size);
  s.print("  data:");
  for (uint16_t i = 0; i < _data_size; i++) {
    s.print(' ');
    s.print(_buffer[i], HEX);
  }
  s.println();
}

  uint16_t SerialPacketsData::crc16() {
    if (!_data_size) {
      return 0;
    }
    return gen_crc16(_buffer, _data_size);
  }



