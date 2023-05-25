#include "packet_crc.h"
#include "serial_packets.h"

// void PacketData::dump_data(Stream& s) const {
//   s.println("SerialPacketsData dump TBD");
// }

void PacketData::release_buffer() {
  if (_buffer) {
    delete[] _buffer;
    _buffer = nullptr;
  }
  _capacity = 0;
  _size = 0;
  _bytes_read = 0;
}

bool PacketData::alloc_buffer(uint16_t capacity) {
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

void PacketData::dump(const char* title, Stream& s) const {
  s.println(title);
  s.print("  size: ");
  s.println(_size);
  s.print("  bytes read: ");
  s.print(_bytes_read);
  if (_read_errors) {
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

static const uint8_t dummy_buffer[0] = {};

uint16_t PacketData::crc16() const {
  // When _size is zero, _buffer may be null so we pass a
  // dummy pointer instead.
  const uint8_t* p = _size ? _buffer : dummy_buffer;
  return packet_crc::gen_crc16(p, _size);
}