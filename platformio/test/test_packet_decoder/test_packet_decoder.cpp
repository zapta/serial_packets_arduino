// Uring test of the packet decoder function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>

#include <memory>
#include <vector>

#include "../common/serial_packets_test_utils.h"
#include "packet_decoder.h"

// For STM32 'black pill'.
// #define BUILTIN_LED PC13

static std::unique_ptr<PacketLogger> logger;
static std::unique_ptr<PacketDecoder> decoder;
static std::unique_ptr<PacketDecoderInspector> inspector;

void setUp(void) {
  inspector.reset();
  decoder.reset();
  // NOTE: Run platformio verbose test to see any logger output.
  logger = std::make_unique<PacketLogger>(PacketLogger::VERBOSE);
  logger->set_stream(&Serial);
  decoder = std::make_unique<PacketDecoder>(*logger);
  inspector = std::make_unique<PacketDecoderInspector>(*decoder);
}


// Helper to to decode a list bytes with expected decoder returned statuses.
static void check_dedoding(PacketDecoder& decoder,
                           const std::vector<uint8_t>& bytes,
                           const std::vector<bool>& expected_statuses) {
  TEST_ASSERT_EQUAL(bytes.size(), expected_statuses.size());
  for (int i = 0; i < bytes.size(); i++) {
    const bool packet_available = decoder.decode_next_byte(bytes.at(i));
    TEST_ASSERT_EQUAL(expected_statuses.at(i), packet_available);
  }
}



void test_initial_state() {
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_FALSE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
}

// A fresh decoder should not decode the first packet without an expclicit pre
// flag.
void test_first_packet_without_pre_flag() {
  check_dedoding(*decoder, {0x03, 0x20, 0x11, 0x22, 0xad, 0xb8, 0x7e},
                 {0, 0, 0, 0, 0, 0, 0});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
}

// A fresh decoder should decode a packet with a pre flag..
void test_first_packet_with_pre_flag() {
  check_dedoding(*decoder, {0x7e, 0x03, 0x20, 0x11, 0x22, 0xad, 0xb8, 0x7e},
                 {0, 0, 0, 0, 0, 0, 0, 1});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
  TEST_ASSERT_EQUAL(TYPE_MESSAGE, decoder->packet_metadata().packet_type);
  TEST_ASSERT_EQUAL(0x20, decoder->packet_metadata().message.endpoint);
  assert_data_equals(decoder->packet_data(), {0x11, 0x22});
}

// CRC set to 0x9999 instead of 0xadb8.
void test_bad_crc() {
  check_dedoding(*decoder, {0x7e, 0x03, 0x20, 0x11, 0x22, 0x99, 0x99, 0x7e},
                 {0, 0, 0, 0, 0, 0, 0, 0});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
}

void test_command_decoding() {
  check_dedoding(*decoder,
                 {0x7e, 0x01, 0x12, 0x34, 0x56, 0x78, 0x20, 0x11, 0x7d, 0x5e,
                  0x22, 0x7d, 0x5d, 0x38, 0x28, 0x7e},
                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
  TEST_ASSERT_EQUAL(TYPE_COMMAND, decoder->packet_metadata().packet_type);
  TEST_ASSERT_EQUAL_HEX32(0x12345678,
                          decoder->packet_metadata().command.cmd_id);
  TEST_ASSERT_EQUAL_HEX8(0x20, decoder->packet_metadata().command.endpoint);
  assert_data_equals(decoder->packet_data(), {0x11, 0x7e, 0x22, 0x7d});
}

void test_response_decoding() {
  check_dedoding(*decoder,
                 {0x7e, 0x02, 0x12, 0x34, 0x56, 0x78, 0x20, 0x11, 0x7d, 0x5e,
                  0x22, 0x7d, 0x5d, 0x89, 0xe7, 0x7e},
                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
  TEST_ASSERT_EQUAL(TYPE_RESPONSE, decoder->packet_metadata().packet_type);
  TEST_ASSERT_EQUAL_HEX32(0x12345678,
                          decoder->packet_metadata().response.cmd_id);
  TEST_ASSERT_EQUAL_HEX8(0x20, decoder->packet_metadata().response.status);
  assert_data_equals(decoder->packet_data(), {0x11, 0x7e, 0x22, 0x7d});
}

void test_message_decoding() {
  check_dedoding(
      *decoder,
      {0x7e, 0x03, 0x20, 0x11, 0x7d, 0x5e, 0x22, 0x7d, 0x5d, 0xa9, 0xe0, 0x7e},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1});
  TEST_ASSERT_EQUAL(0, inspector->packet_len());
  TEST_ASSERT_TRUE(inspector->in_packet());
  TEST_ASSERT_FALSE(inspector->pending_escape());
  TEST_ASSERT_EQUAL(TYPE_MESSAGE, decoder->packet_metadata().packet_type);
  TEST_ASSERT_EQUAL(0x20, decoder->packet_metadata().message.endpoint);
  assert_data_equals(decoder->packet_data(), {0x11, 0x7e, 0x22, 0x7d});
}

//  def test_first_packet_no_pre_flag(self):
//         """Tests that the decoder requires an initial packet flag."""
//         d = PacketDecoder(self.packet_callback)
//         d.receive(bytes([0x03, 0x20, 0x11, 0x7d, 0x5e, 0x22, 0x7d, 0x5d,
//         0xa9, 0xe0, 0x7e])) self.assertEqual(len(self.packets), 0)
//         self.assertEqual(len(d._PacketDecoder__packet_bfr), 0)
//         self.assertTrue(d._PacketDecoder__in_packet)
//         self.assertFalse(d._PacketDecoder__pending_escape)

void setup() {
  common_setup_init();

  UNITY_BEGIN();

  RUN_TEST(test_initial_state);
  RUN_TEST(test_first_packet_without_pre_flag);
  RUN_TEST(test_first_packet_with_pre_flag);
  RUN_TEST(test_bad_crc);
  RUN_TEST(test_command_decoding);
  RUN_TEST(test_response_decoding);
  RUN_TEST(test_message_decoding);

  UNITY_END();
}

void loop() { common_loop_body(); }
