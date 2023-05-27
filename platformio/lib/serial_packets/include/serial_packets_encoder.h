#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"
#include "serial_packets_data.h"
#include "serial_packets_logger.h"

// namespace serial_packets {

class SerialPacketsEncoder {
 public:
  // Keeps a reference to logger.
  SerialPacketsEncoder(SerialPacketsLogger& logger)
      : _logger(logger) {}

  // Return true iff ok.
  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const SerialPacketsData& data, bool insert_pre_flag,
                             SerialPacketsData* out);

  // Return true iff ok.
  bool encode_response_packet(uint32_t cmd_id, uint8_t status,
                              const SerialPacketsData& data, bool insert_pre_flag,
                              SerialPacketsData* out);

  // Return true iff ok.
  bool encode_message_packet(uint8_t endpoint, const SerialPacketsData& data,
                             bool insert_pre_flag, SerialPacketsData* out);



 private:
  // For testing.
  friend class PacketEncoderInspector;

  // Non null.
  SerialPacketsLogger& _logger;

  // Used to encode the packet.
  SerialPacketsData _tmp_data;

  bool byte_stuffing(const SerialPacketsData& in, bool insert_pre_flag,
                     SerialPacketsData* out);



};

