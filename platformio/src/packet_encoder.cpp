#include "packet_encoder.h"

#include "io.h"
#include "packet_consts.h"
#include "serial_packets.h"

bool PacketEncoder::byte_stuffing(const PacketData& in,
                                  bool insert_pre_flag, PacketData* out) {
  out->clear();
  const uint16_t capacity = out->capacity();
  uint16_t j = 0;

  // If requested, prepend a flag byte.
  if (insert_pre_flag) {
    if (j + 1 >= capacity) {
      _logger.error(
          "Insufficient space when trying to byte stuff a packet (#1).");
      return false;
    }
    out->_buffer[j++] = PACKET_FLAG;
  }

  // Byte stuff the in bytes.
  for (uint16_t i = 0; i < in._size; i++) {
    const uint8_t b = in._buffer[i];
    if (b == PACKET_FLAG || b == PACKET_ESC) {
      if (j + 2 >= capacity) {
        _logger.error(
            "Insufficient space when trying to byte stuff a packet "
            "(#2).");
        return false;
      }
      out->_buffer[j++] = PACKET_ESC;
      out->_buffer[j++] = b ^ 0x20;
    } else {
      if (j + 1 >= capacity) {
        _logger.error(
            "Insufficient space when trying to byte stuff a packet (#3).");
        return false;
      }
      out->_buffer[j++] = b;
    }
  }

  // Append post flag.
  if (j + 1 >= capacity) {
    _logger.error(
        "Insufficient space when trying to byte stuff a packet (#4).");
    return false;
  }
  out->_buffer[j++] = PACKET_FLAG;

  // Update outptut data size.
  out->_size = j;
  return true;
}

bool PacketEncoder::encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                                          const PacketData& data,
                                          bool insert_pre_flag,
                                          PacketData* out) {
  // Encode packet in _tmp_data.
  _tmp_data.clear();
  _tmp_data.add_uint8(TYPE_COMMAND);
  _tmp_data.add_uint32(cmd_id);
  _tmp_data.add_uint8(endpoint);
  _tmp_data.add_data(data);
  _tmp_data.add_uint16(_tmp_data.crc16());
  if (_tmp_data.write_error()) {
    _logger.error("Error encoding a command packet. Data size: %hu",
                  data.size());
    return false;
  }

  // Byte stuffed into the outupt data.
  return byte_stuffing(_tmp_data, insert_pre_flag, out);
}

bool PacketEncoder::encode_response_packet(uint32_t cmd_id, uint8_t status,
                                           const PacketData& data,
                                           bool insert_pre_flag,
                                           PacketData* out) {
  // Encode packet in _tmp_data.
  _tmp_data.clear();
  _tmp_data.add_uint8(TYPE_RESPONSE);
  _tmp_data.add_uint32(cmd_id);
  _tmp_data.add_uint8(status);
  _tmp_data.add_data(data);
  _tmp_data.add_uint16(_tmp_data.crc16());
  if (_tmp_data.write_error()) {
    _logger.error("Error encoding a response packet. Data size: %hu",
                  data.size());
    return false;
  }

  // Byte stuffed into the outupt data.
  return byte_stuffing(_tmp_data, insert_pre_flag, out);
}

bool PacketEncoder::encode_message_packet(uint8_t endpoint,
                                          const PacketData& data,
                                          bool insert_pre_flag,
                                          PacketData* out) {
  // Encode packet in _tmp_data.
  _tmp_data.clear();
  _tmp_data.add_uint8(TYPE_MESSAGE);
  _tmp_data.add_uint8(endpoint);
  _tmp_data.add_data(data);
  _tmp_data.add_uint16(_tmp_data.crc16());
  if (_tmp_data.write_error()) {
    _logger.error("Error encoding a response packet. Data size: %hu",
                  data.size());
    return false;
  }

  // Byte stuffed into the outupt data.
  return byte_stuffing(_tmp_data, insert_pre_flag, out);
}
