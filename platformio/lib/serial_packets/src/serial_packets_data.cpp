#include "serial_packets_data.h"

#include "serial_packets_crc.h"
#include "serial_packets_consts.h"

// void PacketData::dump_data(Stream& s) const {
//   s.println("SerialPacketsData dump TBD");
// }

// void SerialPacketsData::release_buffer() {
//   if (_buffer) {
//     delete[] _buffer;
//     _buffer = nullptr;
//   }
//   _capacity = 0;
//   _size = 0;
//   _bytes_read = 0;
// }

// void SerialPacketsData::alloc_buffer(uint16_t capacity) {
//   release_buffer();

//   // For zero capacity we don't allocate a buffer.
//   if (capacity == 0) {
//     return;
//   }

//   // Constrain max.
//   if (capacity > MAX_PACKET_DATA_LEN) {
//     capacity = MAX_PACKET_DATA_LEN;
//   }

//   // NOTE: The () zeros the array for determinism.
//   _buffer = new byte[capacity]();
//   if (_buffer) {
//   _capacity = capacity;
//   }
// }

void SerialPacketsData::dump(const char* title, Stream& s) const {
  s.println(title);
  s.print("  size: ");
  s.println(_size);
  s.print("  bytes read: ");
  s.print(_bytes_read);
  if (_had_read_errors) {
    s.print(" (error)");
  }
  s.println();
  s.print("  capacity: ");
  s.println(capacity());
  s.print("  data:");
  for (uint16_t i = 0; i < _size; i++) {
    s.print(' ');
    s.print(_buffer[i], HEX);
  }
  s.println();
}

static const uint8_t dummy_buffer[0] = {};

uint16_t SerialPacketsData::crc16() const {
  return serial_packets_gen_crc16(_buffer, _size);
}
