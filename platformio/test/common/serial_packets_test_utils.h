

#pragma once

#include <Arduino.h>

#include <vector>

#include "packet_data.h"
#include "packet_decoder.h"
#include "packet_encoder.h"

// Side affect of reseting the reading.
void populate_data(PacketData& data, const std::vector<uint8_t> bytes);

void assert_data_equals(const PacketData& data,
                        const std::vector<uint8_t> expected);

class PacketEncoderInspector {
 public:
  PacketEncoderInspector(PacketEncoder& encoder) : _encoder(encoder) {}

  bool run_byte_stuffing(const PacketData& in, bool insert_pre_flag,
                         PacketData* out) {
    return _encoder.byte_stuffing(in, insert_pre_flag, out);
  }
  //  bool pending_escape() { return _decoder._pending_escape;};

 private:
  PacketEncoder& _encoder;
};

// struct InternalPacketDecoderState {
//   const uint8_t* buffer;
//   uint16_t size;
//   bool pending_escape;
// };

class PacketDecoderInspector {
 public:
  PacketDecoderInspector(const PacketDecoder& decoder) : _decoder(decoder) {}

  uint16_t packet_len() { return _decoder._packet_len; }
  bool in_packet() { return _decoder._in_packet; }
  bool pending_escape() { return _decoder._pending_escape; };

 private:
  const PacketDecoder& _decoder;
};


// Should be invoked at the begining of setup();
void common_setup_init();

void common_loop_body();
