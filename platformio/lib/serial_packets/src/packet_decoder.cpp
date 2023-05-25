
#include "packet_decoder.h"

#include <Arduino.h>

#include "packet_crc.h"
#include "packet_consts.h"

bool PacketDecoder::decode_next_byte(uint8_t b) {
  // When not in packet, wait for next  flag byte.
  if (!_in_packet) {
    if (b == PACKET_FLAG) {
      _in_packet = true;
      _packet_len = 0;
      _pending_escape = false;
      return false;
    }
  }

  // Here _in_packet = true. Handle end of packet flag byte.
  if (b == PACKET_FLAG) {
    const bool has_new_packet = process_packet();
    // NOTE: We don't require an additional pre flag for the
    // next packet.
    _in_packet = true;
    _packet_len = 0;
    _pending_escape = false;
    return has_new_packet;
  }

  // Check for size overrun. At this point, we know that the packet will
  // have at least one more  byte.
  if (_packet_len >= MAX_PACKET_LEN) {
    _in_packet = false;
    _packet_len = 0;
    _pending_escape = false;
    _logger.error("Incoming packet overrun");
    return false;
  }

  // Handle escaped byte.
  if (_pending_escape) {
    const uint8_t b1 = b ^ 0x20;
    if (b1 != PACKET_FLAG && b1 != PACKET_ESC) {
      _logger.error(
          "Incoming packet has the byte %02hx after an escape byte", b1);
      _in_packet = false;
      _packet_len = 0;
      _pending_escape = false;
      return false;
    }
    _packet_buffer[_packet_len++] = b1;
    _pending_escape = false;
    return false;
  }

  // Handle escape byte. Here _pending_escape is false.
  if (b == PACKET_ESC) {
    _pending_escape = true;
    return false;
  }

  _packet_buffer[_packet_len++] = b;
  return false;
}

// Returns true if a new packet is available.
bool PacketDecoder::process_packet() {
  // This is normal in packets that insert pre packet flags.
  if (!_packet_len) {
    return false;
  }

  if (_packet_len < MIN_PACKET_LEN) {
    _logger.error("Incoming packet is too short: %hu", _packet_len);
    return false;
  }

  // Check CRC. These are the last two bytes in big endian oder.
  const uint16_t packet_crc = decode_uint16_at_index(_packet_len - 2);
  const uint16_t computed_crc = packet_crc::gen_crc16(_packet_buffer, _packet_len - 2);
  if (packet_crc != computed_crc) {
    Serial.printf("crc: %04hx vs %04hx\n", packet_crc, computed_crc);
    _logger.error("Incoming packet has bad CRC: %04hx vs %04hx",
                 packet_crc, computed_crc);
    return false;
  }

  // Construct the decoded packet by its type.
  const uint8_t packet_type = _packet_buffer[0];

  // Decode a command packet.
  if (packet_type == TYPE_COMMAND) {
    if (_packet_len < 8) {
      _logger.error("Incoming command packet is too short: %hu",
                   _packet_len);
      return false;
    }
    _decoded_metadata.packet_type = TYPE_COMMAND;
    _decoded_metadata.command.cmd_id = decode_uint32_at_index(1);
    _decoded_metadata.command.endpoint = _packet_buffer[5];
    _decoded_data.clear();
    _decoded_data.write_bytes(&_packet_buffer[6], _packet_len - 8);
    return true;
  }

  // Decode a response packet.
  if (packet_type == TYPE_RESPONSE) {
    if (_packet_len < 8) {
      _logger.error("Incoming response packet is too short: %hu",
                   _packet_len);
      return false;
    }
    _decoded_metadata.packet_type = TYPE_RESPONSE;
    _decoded_metadata.response.cmd_id = decode_uint32_at_index(1);
    _decoded_metadata.response.status = _packet_buffer[5];
    _decoded_data.clear();
    _decoded_data.write_bytes(&_packet_buffer[6], _packet_len - 8);
    return true;
  }

  // Decode a message packet.
  if (packet_type == TYPE_MESSAGE) {
    if (_packet_len < 4) {
      _logger.error("Incoming message packet is too short: %hu",
                   _packet_len);
      return false;
    }
    _decoded_metadata.packet_type = TYPE_MESSAGE;
    _decoded_metadata.message.endpoint = _packet_buffer[1];
    _decoded_data.clear();
    _decoded_data.write_bytes(&_packet_buffer[2], _packet_len - 4);
    return true;
  }

      _logger.error("Incoming packet has an invalid type: %hu", packet_type);
  return false;
}
