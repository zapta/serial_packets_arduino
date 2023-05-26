

#pragma once

#include <Arduino.h>

#include <vector>

#include "serial_packets.h"
// #include "packet_data.h"
// #include "packet_decoder.h"
// #include "packet_encoder.h"

// Side affect of reseting the reading.
void populate_data(PacketData& data, const std::vector<uint8_t> bytes);

std::vector<uint8_t> copy_data(const PacketData& data);

void assert_data_equals(const PacketData& data,
                        const std::vector<uint8_t> expected);

void assert_vectors_equal(const std::vector<uint8_t> expected,
                          const std::vector<uint8_t> actual);

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

class SerialPacketsClientInspector {
 public:
  SerialPacketsClientInspector(const SerialPacketsClient& client)
      : _client(client){};

  int num_pending_commands() {
    int count = 0;
    for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
      if (_client._command_contexts[i].cmd_id) {
        count++;
      }
    }
    return count;
  }
  // PacketDecoderInspector(const PacketDecoder& decoder) : _decoder(decoder) {}

  // uint16_t packet_len() { return _decoder._packet_len; }
  // bool in_packet() { return _decoder._in_packet; }
  // bool pending_escape() { return _decoder._pending_escape; };

 private:
  const SerialPacketsClient& _client;
};

void loop_client(SerialPacketsClient& client, uint32_t time_millis);

// Should be invoked at the begining of setup();
void common_setup_init();

void common_loop_body();
