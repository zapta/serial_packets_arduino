#pragma once

#include <Arduino.h>

#include "packet_data.h"

class PacketEncoder {
 public:
  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const SerialPacketData& data,
                             SerialPacketData& out, bool insert_pre_flag);
 private:
 
   SerialPacketData _tmp_data;

  bool byte_stuffing(const SerialPacketData& in, SerialPacketData& out,
                     bool insert_pre_flag);
};
