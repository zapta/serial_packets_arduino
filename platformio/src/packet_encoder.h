#pragma once

#include <Arduino.h>

#include "packet_data.h"
#include "packet_consts.h"

class PacketEncoder {
 public:
  PacketEncoder() : _tmp_data(MAX_DATA_LEN) {}

  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const SerialPacketData& data,
                             SerialPacketData& out, bool insert_pre_flag);

  bool encode_response_packet(uint32_t cmd_id, uint8_t status,
                              const SerialPacketData& data,
                              SerialPacketData& out, bool insert_pre_flag);

  bool encode_message_packet(uint8_t endpoint, const SerialPacketData& data,
                             SerialPacketData& out, bool insert_pre_flag);

 private:
  SerialPacketData _tmp_data;

  bool byte_stuffing(const SerialPacketData& in, SerialPacketData& out,
                     bool insert_pre_flag);
};
