

#pragma once

#include <Arduino.h>
#include <vector>
#include "serial_packets_client.h"


// Side affect of reseting the reading.
void populate_data(SerialPacketsData& data, const std::vector<uint8_t> bytes);

std::vector<uint8_t> copy_data(const SerialPacketsData& data);

void assert_data_equals(const SerialPacketsData& data,
                        const std::vector<uint8_t> expected);

void assert_vectors_equal(const std::vector<uint8_t> expected,
                          const std::vector<uint8_t> actual);

class PacketEncoderInspector {
 public:
  PacketEncoderInspector(SerialPacketsEncoder& encoder) : _encoder(encoder) {}

  bool run_byte_stuffing(const SerialPacketsData& in, bool insert_pre_flag,
                         SerialPacketsData* out) {
    return _encoder.byte_stuffing(in, insert_pre_flag, out);
  }

 private:
  SerialPacketsEncoder& _encoder;
};

class PacketDecoderInspector {
 public:
  PacketDecoderInspector(const SerialPacketsDecoder& decoder)
      : _decoder(decoder) {}

  uint16_t packet_len() { return _decoder._packet_len; }
  bool in_packet() { return _decoder._in_packet; }
  bool pending_escape() { return _decoder._pending_escape; };

 private:
  const SerialPacketsDecoder& _decoder;
};

class SerialPacketsClientInspector {
 public:
  SerialPacketsClientInspector(SerialPacketsClient& client) : _client(client){};

  void ignore_rx_for_testing(bool value) {
    _client._ignore_rx_for_testing = value;
  }

 private:
  SerialPacketsClient& _client;
};

void loop_client(SerialPacketsClient& client, uint32_t time_millis);

// Should be invoked at the begining of setup();
void common_setup_init();

void common_loop_body();
