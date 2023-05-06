#include "serial_packets.h"

 void SerialPacketsData::dump_data(Stream& s) {
    s.println("SerialPacketsData dump TBD");
  }

void SerialPacketsData::free_buffer() {
  if (_buffer) {
    delete[] _buffer;
    _buffer = nullptr;
  }
  _buffer_size = 0;
  _data_size = 0;
  _next_read_index = 0;
}

 bool SerialPacketsData::alloc_buffer(uint16_t buffer_size) {
    free_buffer();
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



bool SerialPacketsData::add_byte(byte v) {
    if (_data_size + 1 > _buffer_size) {
      return false;
    }
    _buffer[_data_size++] = v;
    return true;
  }

  bool SerialPacketsData::add_uint16(uint16_t v) {
    if (_data_size + 2 > _buffer_size) {
      return false;
    }
    _buffer[_data_size++] = v >> 8;
    _buffer[_data_size++] = v >> 0;
    return true;
  }

  bool SerialPacketsData::add_uint32(uint32_t v) {
    if (_data_size + 4 > _buffer_size) {
      return false;
    }
    _buffer[_data_size++] = v >> 24;
    _buffer[_data_size++] = v >> 16;
    _buffer[_data_size++] = v >> 8;
    _buffer[_data_size++] = v >> 0;
    return true;
  }

  bool SerialPacketsData::add_bytes_block(byte bytes[], uint32_t num_bytes) {
    if (num_bytes > 255 || _data_size + 1 + num_bytes > _buffer_size) {
      return false;
    }
    // One byte for size.
    _buffer[_data_size++] = num_bytes;
    // Then the actual bytes.
    memcpy(&_buffer[_data_size], bytes, num_bytes);
    _data_size += num_bytes;
    return true;
  }

  // Data decoding
  bool SerialPacketsData::read_byte(byte& v) {
    if (_next_read_index + 1 > _data_size) {
      return false;
    }
    v = _buffer[_next_read_index++];
    return true;
  }

  bool SerialPacketsData::read_uint16(uint16_t& v) {
    if (_next_read_index + 2 > _data_size) {
      return false;
    }
    byte* p = _buffer + _next_read_index;
    v = ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
    _next_read_index += 2;
    return true;
  }

  bool SerialPacketsData::read_uint32(uint32_t& v) {
    if (_next_read_index + 4 > _data_size) {
      return false;
    }
    byte* p = _buffer + _next_read_index;
    v = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
        ((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
    _next_read_index += 4;
    return true;
  }

  bool SerialPacketsData::read_bytes_block(byte bytes_buffer[], uint32_t byte_buffer_size,
                        uint32_t& bytes_read) {
    // Make sure we have at least the size byte.
    if (_next_read_index + 1 > _data_size) {
      return false;
    }
    // Check size, against both the data size and user buffer size.
    uint8_t size = _buffer[_next_read_index];
    if ((_next_read_index + 1 + size) > _data_size || size > byte_buffer_size) {
      return false;
    }
    // We are ready to read.
    memcpy(bytes_buffer, &_buffer[_next_read_index + 1], size);
    bytes_read = size;
    _next_read_index += (1 + size);
    return true;
  }





void SerialPacketsClient::begin(HardwareSerial& data_serial,
                                Print& debug_printer) {
  if (_data_serial) {
    debug_printer.println(
        "ERROR: Already called SerialPacktsBegin.bein(), ignoring.");
  }
  _data_serial = &data_serial;
  _debug_printer = &debug_printer;
}

void SerialPacketsClient::loop() {}

bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketsData& data,
    const SerialPacketsPendingCommandResponse& pending_command) {
  return false;
}

bool SerialPacketsClient::sendMessage(byte endpoint,
                                      const SerialPacketsData& data) {
  return false;
};