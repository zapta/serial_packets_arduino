#pragma once

#include <Arduino.h>

#include "packet_consts.h"
#include "packet_data.h"
#include "packet_logger.h"

class PacketEncoder {
 public:
  // Keeps a reference to logger.
  PacketEncoder(PacketLogger& logger)
      : _logger(logger), _tmp_data(MAX_PACKET_DATA_LEN) {}

  // Return true iff ok.
  bool encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                             const PacketData& data, bool insert_pre_flag,
                             PacketData* out);

  // Return true iff ok.
  bool encode_response_packet(uint32_t cmd_id, uint8_t status,
                              const PacketData& data, bool insert_pre_flag,
                              PacketData* out);

  // Return true iff ok.
  bool encode_message_packet(uint8_t endpoint, const PacketData& data,
                             bool insert_pre_flag, PacketData* out);



 private:
  // For testing.
  friend class PacketEncoderInspector;
  
  // Non null.
  PacketLogger& _logger;

  PacketData _tmp_data;

  bool byte_stuffing(const PacketData& in, bool insert_pre_flag,
                     PacketData* out);



  // friend void test_byte_sutffing_with_pre_flag();
  // friend void test_byte_sutffing_without_pre_flag();
};
