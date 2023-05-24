
#include "packet_decoder.h"

#include <Arduino.h>

#include "crc.h"
#include "packet_consts.h"

PacketDecoder::DecoderStatus PacketDecoder::decode_next_byte(uint8_t b) {
  // When not in packet, wait for next  flag byte.
  if (!_in_packet) {
    if (b == PACKET_FLAG) {
      _in_packet = true;
      _packet_len = 0;
      _pending_escape = false;
      return PACKET_START;
    }
  }

  // Here _in_packet = true. Handle end of packet flag byte.
  if (b == PACKET_FLAG) {
    const DecoderStatus status = process_packet();
    // NOTE: We don't require an additional pre flag for the
    // next packet.
    _in_packet = true;
    _packet_len = 0;
    _pending_escape = false;
    return status;
  }

  // Check for size overrun. At this point, we know that the packet will
  // have at least one more  byte.
  if (_packet_len >= MAX_PACKET_LEN) {
    _in_packet = false;
    _packet_len = 0;
    _pending_escape = false;
    return OVERRUN;
  }

  // Handle escape byte
  if (b == PACKET_ESC) {
    if (_pending_escape) {
      _in_packet = false;
      _packet_len = 0;
      _pending_escape = false;
      return CONSECUTIVE_ESCAPES;
    }

    _pending_escape = true;

    return IN_PACKET;
  }

  // Handle escaped byte.
  if (_pending_escape) {
    const uint8_t b1 = b ^ 0x20;
    if (b1 != PACKET_FLAG && b1 != PACKET_ESC) {
      _in_packet = false;
      _packet_len = 0;
      _pending_escape = false;
      return INVALID_ESCAPE;
    }
    _packet_buffer[_packet_len++] = b1;
    _pending_escape = false;

    return IN_PACKET;
  }

  _packet_buffer[_packet_len++] = b;
  return IN_PACKET;
}

PacketDecoder::DecoderStatus PacketDecoder::process_packet() {
  // This is normal in packets that insert pre packet flags.
  if (!_packet_len) {
    return IN_PACKET;
  }

  if (_packet_len < MIN_PACKET_LEN) {
    return PACKET_TOO_SHORT;
  }

  // Check CRC. These are the last two bytes in big endian oder.
  const uint16_t packet_crc = decode_uint16_at_index(_packet_len - 2);
  const uint16_t computed_crc = gen_crc16(_packet_buffer, _packet_len - 2);
  if (packet_crc != computed_crc) {
    Serial.printf("crc: %04hx vs %04hx\n", packet_crc, computed_crc);
    return BAD_CRC;
  }

  // Construct the decoded packet by its type.
  const uint8_t packet_type = _packet_buffer[0];

  // Decode a command packet.
  if (packet_type == TYPE_COMMAND) {
    if (_packet_len < 8) {
      return COMMAND_PACKET_TOO_SHORT;
    }
    _decoded_metadata.packet_type = TYPE_COMMAND;
    _decoded_metadata.command.cmd_id = decode_uint32_at_index(1);
    _decoded_metadata.command.endpoint = _packet_buffer[5];
    _decoded_data.clear();
    _decoded_data.add_bytes(&_packet_buffer[6], _packet_len - 8);
    return PACKET_DECODED;
  }

  // Decode a response packet.
  if (packet_type == TYPE_RESPONSE) {
    if (_packet_len < 8) {
      return RESPONSE_PACKET_TOO_SHORT;
    }
    _decoded_metadata.packet_type = TYPE_RESPONSE;
    _decoded_metadata.response.cmd_id = decode_uint32_at_index(1);
    _decoded_metadata.response.status = _packet_buffer[5];
    _decoded_data.clear();
    _decoded_data.add_bytes(&_packet_buffer[6], _packet_len - 8);
    return PACKET_DECODED;
  }

  // Decode a message packet.
  if (packet_type == TYPE_MESSAGE) {
    if (_packet_len < 4) {
      return MESSAGE_PACKET_TOO_SHORT;
    }
    _decoded_metadata.packet_type = TYPE_MESSAGE;
    _decoded_metadata.message.endpoint = _packet_buffer[1];
    _decoded_data.clear();
    _decoded_data.add_bytes(&_packet_buffer[2], _packet_len - 4);
    return PACKET_DECODED;
  }

  return INVALID_PACKET_TYPE;
}
