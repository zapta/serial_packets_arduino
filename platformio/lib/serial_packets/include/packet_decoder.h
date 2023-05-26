#pragma once

#include <Arduino.h>

#include "packet_consts.h"
#include "packet_data.h"
#include "packet_logger.h"

// enum DecodedPacketType {
//   COMMAND,
//   RESPONSE,
//   MESSAGE,
// };

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

class PacketDecoder {
 public:
  // enum DecoderStatus {
  //   // In normal operation.
  //   PACKET_START = 1,
  //   IN_PACKET,
  //   PACKET_DECODED,

  //   // Errors.
  //   OVERRUN,
  //   CONSECUTIVE_ESCAPES,
  //   INVALID_ESCAPE,
  //   PACKET_TOO_SHORT,
  //   BAD_CRC,
  //   COMMAND_PACKET_TOO_SHORT,
  //   RESPONSE_PACKET_TOO_SHORT,
  //   MESSAGE_PACKET_TOO_SHORT,
  //   INVALID_PACKET_TYPE,
  // };

  PacketDecoder(PacketLogger& logger)
      : _logger(logger), _decoded_data(MAX_PACKET_DATA_LEN) {}

  // True if a packet is available.
  bool decode_next_byte(uint8_t);

  const DecodedPacketMetadata& packet_metadata() { return _decoded_metadata; }

  const PacketData& packet_data() { return _decoded_data; }

  // For debugging. Return the current number of accomulated bytes.
  uint16_t len() { return _packet_len; }

 private:
  // For testing.
  friend class PacketDecoderInspector;

  // None null.
  PacketLogger& _logger;

  uint8_t _packet_buffer[MAX_PACKET_LEN];
  uint16_t _packet_len = 0;

  //  enum State {
  //   IDLE,
  //   IN_PACKET,
  //   PACKET_AVAILABLE,
  //  };

  //  State _state = IDLE;

  bool _in_packet = false;
  // Valid in IN_PACKET state only.
  bool _pending_escape = false;

  // Valid after returning the status PACKET_AVAILABLE.
  DecodedPacketMetadata _decoded_metadata;
  PacketData _decoded_data;

  // Returns true iff a new packet is available.
  bool process_packet();

  inline uint16_t decode_uint16_at_index(uint16_t i) {
    return (((uint16_t)_packet_buffer[i]) << 8) |
           (((uint16_t)_packet_buffer[i + 1]) << 0);
  }

  inline uint32_t decode_uint32_at_index(uint16_t i) {
    return (((uint32_t)_packet_buffer[i]) << 24) |
           (((uint32_t)_packet_buffer[i + 1]) << 16) |
           (((uint32_t)_packet_buffer[i + 2]) << 8) |
           (((uint32_t)_packet_buffer[i + 3]) << 0);
  }
};