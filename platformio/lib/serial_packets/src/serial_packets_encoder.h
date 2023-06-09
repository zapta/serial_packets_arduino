// Packet encoder.

#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"
#include "serial_packets_data.h"
#include "serial_packets_logger.h"

class SerialPacketsEncoder {
 public:
  // Keeps a reference to logger.
  SerialPacketsEncoder(SerialPacketsLogger& logger) : _logger(logger) {}

  // Encode a command packet. Return true iff ok.
  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const SerialPacketsData& data,
                             bool insert_pre_flag, StuffedPacketBuffer* out);

  // Encode a response packet. Return true iff ok.
  bool encode_response_packet(uint32_t cmd_id, uint8_t status,
                              const SerialPacketsData& data,
                              bool insert_pre_flag, StuffedPacketBuffer* out);

  // Encode a message packet. Return true iff ok.
  bool encode_message_packet(uint8_t endpoint, const SerialPacketsData& data,
                             bool insert_pre_flag, StuffedPacketBuffer* out);

 private:
  // For testing.
  friend class PacketEncoderInspector;

  SerialPacketsLogger& _logger;

  // Used to encode the packet.
  EncodedPacketBuffer _tmp_data;

  bool byte_stuffing(const EncodedPacketBuffer& in, bool insert_pre_flag,
                     StuffedPacketBuffer* out);
};
