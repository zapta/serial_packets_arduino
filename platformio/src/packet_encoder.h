#pragma once

#include <Arduino.h>

#include "serial_packets.h"

class PacketEncoder {
 public:
  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const SerialPacketsData& data,
                             SerialPacketsData& out, bool insert_pre_flag);
 private:
 
   SerialPacketsData _tmp_data;

  bool byte_stuffing(const SerialPacketsData& in, SerialPacketsData& out,
                     bool insert_pre_flag);
};
