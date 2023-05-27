// Packet decoder.

#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"
#include "serial_packets_data.h"
#include "serial_packets_logger.h"

using serial_packets_consts::MAX_PACKET_LEN;
using serial_packets_consts::PacketType;

struct DecodedCommandMetadata {
  uint32_t cmd_id;
  uint8_t endpoint;
};

struct DecodedResponseMetadata {
  uint32_t cmd_id;
  uint8_t status;
};

struct DecodedMessageMetadata {
  uint8_t endpoint;
};

struct DecodedPacketMetadata {
  PacketType packet_type;
  union {
    DecodedCommandMetadata command;
    DecodedResponseMetadata response;
    DecodedMessageMetadata message;
  };
};

class SerialPacketsDecoder {
 public:
   // Keeps a reference to logger.
  SerialPacketsDecoder(SerialPacketsLogger& logger) : _logger(logger) {}

  // Returns true if a decoded packet became available.
  bool decode_next_byte(uint8_t);

  // Accessors to the decoded packet.
  const DecodedPacketMetadata& packet_metadata() { return _decoded_metadata; }
  const SerialPacketsData& packet_data() { return _decoded_data; }

  // For debugging. Return the current number of bytes in the packet buffer.
  // uint16_t len() { return _packet_len; }

 private:
  // For testing.
  friend class PacketDecoderInspector;

  SerialPacketsLogger& _logger;

  uint8_t _packet_buffer[MAX_PACKET_LEN];
  uint16_t _packet_len = 0;

  // True if collecting packet bytes. False, if waitint for a
  // flag byte to start a new packet.
  bool _in_packet = false;

  // Valid when _in_packet is true. Indicates if last byte
  // was the escape byte.
  bool _pending_escape = false;

  // The decoded packets are stored here for the client to process.
  DecodedPacketMetadata _decoded_metadata;
  SerialPacketsData _decoded_data;

  // Called to decode a packet from the packet buffer. Returns true iff a
  // new packet is available.
  bool process_packet();

  // Decode a big endian uint16.
  inline uint16_t decode_uint16_at_index(uint16_t i) {
    return (((uint16_t)_packet_buffer[i]) << 8) |
           (((uint16_t)_packet_buffer[i + 1]) << 0);
  }

  // Decode a big endian uint32.
  inline uint32_t decode_uint32_at_index(uint16_t i) {
    return (((uint32_t)_packet_buffer[i]) << 24) |
           (((uint32_t)_packet_buffer[i + 1]) << 16) |
           (((uint32_t)_packet_buffer[i + 2]) << 8) |
           (((uint32_t)_packet_buffer[i + 3]) << 0);
  }
};
