// Uring test of the packet encoder function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>

#include <memory>
#include <vector>

#include "../common/serial_packets_test_utils.h"
#include "packet_encoder.h"

// For STM32 'black pill'.
// #define BUILTIN_LED PC13

static std::unique_ptr<PacketLogger> logger;
static std::unique_ptr<PacketEncoder> encoder;
static std::unique_ptr<PacketEncoderInspector> inspector;

void setUp(void) {
  inspector.reset();
  encoder.reset();
  logger = std::make_unique<PacketLogger>();
  encoder = std::make_unique<PacketEncoder>(*logger);
  inspector = std::make_unique<PacketEncoderInspector>(*encoder);
}

void test_byte_sutffing_with_pre_flag() {
  // const uint8_t bytes[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(bytes, sizeof(bytes));
  PacketData out(30);
  TEST_ASSERT_TRUE(inspector->run_byte_stuffing(in, true, &out));
  // TEST_ASSERT_EQUAL(10, out.size());
  // uint8_t actual[10] = {};
  // out.read_bytes(actual, sizeof(actual));
  // TEST_ASSERT_TRUE(out.all_read_ok());
  assert_data_equals(
      out, {0x7e, 0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d, 0x5d, 0x99, 0x7e});
  // const uint8_t expected[10] = {0x7e, 0xff, 0x00, 0x7d, 0x5e,
  //                               0x22, 0x7d, 0x5d, 0x99, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
}

void test_byte_sutffing_without_pre_flag() {
  // const uint8_t bytes[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(bytes, sizeof(bytes));
  PacketData out(30);
  TEST_ASSERT_TRUE(inspector->run_byte_stuffing(in, false, &out));
  // TEST_ASSERT_EQUAL(9, out.size());
  // uint8_t actual[9] = {};
  // out.read_bytes(actual, sizeof(actual));
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[9] = {0xff, 0x00, 0x7d, 0x5e, 0x22,
  //                              0x7d, 0x5d, 0x99, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));

  assert_data_equals(out,
                     {0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d, 0x5d, 0x99, 0x7e});
}

void test_encode_command_packet_with_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(
      encoder->encode_command_packet(0x12345678, 0x20, in, true, &out));
  // TEST_ASSERT_EQUAL(18, out.size());
  // uint8_t actual[18] = {};
  // out.read_bytes(actual, 18);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[18] = {0x7e, 0x01, 0x12, 0x34, 0x56, 0x78,
  //                               0x20, 0xff, 0x00, 0x7d, 0x5e, 0x22,
  //                               0x7d, 0x5d, 0x99, 0xD4, 0x80, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(
      out, {0x7e, 0x01, 0x12, 0x34, 0x56, 0x78, 0x20, 0xff, 0x00, 0x7d, 0x5e,
            0x22, 0x7d, 0x5d, 0x99, 0xD4, 0x80, 0x7e});
}

void test_encode_command_packet_without_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(
      encoder->encode_command_packet(0x12345678, 0x20, in, false, &out));
  // TEST_ASSERT_EQUAL(17, out.size());
  // uint8_t actual[17] = {};
  // out.read_bytes(actual, 17);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[17] = {0x01, 0x12, 0x34, 0x56, 0x78, 0x20,
  //                               0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d,
  //                               0x5d, 0x99, 0xD4, 0x80, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(out, {0x01, 0x12, 0x34, 0x56, 0x78, 0x20, 0xff, 0x00, 0x7d,
                           0x5e, 0x22, 0x7d, 0x5d, 0x99, 0xD4, 0x80, 0x7e});
}

void test_encode_response_packet_with_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(
      encoder->encode_response_packet(0x12345678, 0x20, in, true, &out));
  // TEST_ASSERT_EQUAL(18, out.size());
  // uint8_t actual[18] = {};
  // out.read_bytes(actual, 18);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[18] = {0x7e, 0x02, 0x12, 0x34, 0x56, 0x78,
  //                               0x20, 0xff, 0x00, 0x7d, 0x5e, 0x22,
  //                               0x7d, 0x5d, 0x99, 0xd1, 0x1f, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(
      out, {0x7e, 0x02, 0x12, 0x34, 0x56, 0x78, 0x20, 0xff, 0x00, 0x7d, 0x5e,
            0x22, 0x7d, 0x5d, 0x99, 0xd1, 0x1f, 0x7e});
}

void test_encode_response_packet_without_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(
      encoder->encode_response_packet(0x12345678, 0x20, in, false, &out));
  // TEST_ASSERT_EQUAL(17, out.size());
  // uint8_t actual[17] = {};
  // out.read_bytes(actual, 17);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[17] = {0x02, 0x12, 0x34, 0x56, 0x78, 0x20,
  //                               0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d,
  //                               0x5d, 0x99, 0xd1, 0x1f, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(out, {0x02, 0x12, 0x34, 0x56, 0x78, 0x20, 0xff, 0x00, 0x7d,
                           0x5e, 0x22, 0x7d, 0x5d, 0x99, 0xd1, 0x1f, 0x7e});
}

void test_encode_message_packet_with_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(encoder->encode_message_packet(0x20, in, true, &out));
  // TEST_ASSERT_EQUAL(14, out.size());
  // uint8_t actual[14] = {};
  // out.read_bytes(actual, 14);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[14] = {0x7e, 0x03, 0x20, 0xff, 0x00, 0x7d, 0x5e,
  //                               0x22, 0x7d, 0x5d, 0x99, 0xa7, 0x1e, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(out, {0x7e, 0x03, 0x20, 0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d,
                           0x5d, 0x99, 0xa7, 0x1e, 0x7e});
}

void test_encode_message_packet_without_pre_flag() {
  // const uint8_t data[] = {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99};
  PacketData in(30);
  populate_data(in, {0xff, 0x00, 0x7e, 0x22, 0x7d, 0x99});
  // in.write_bytes(data, sizeof(data));
  PacketData out(30);
  TEST_ASSERT_TRUE(encoder->encode_message_packet(0x20, in, false, &out));
  // TEST_ASSERT_EQUAL(13, out.size());
  // uint8_t actual[13] = {};
  // out.read_bytes(actual, 13);
  // TEST_ASSERT_TRUE(out.all_read_ok());
  // const uint8_t expected[13] = {0x03, 0x20, 0xff, 0x00, 0x7d, 0x5e, 0x22,
  //                               0x7d, 0x5d, 0x99, 0xa7, 0x1e, 0x7e};
  // TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, sizeof(expected));
  assert_data_equals(out, {0x03, 0x20, 0xff, 0x00, 0x7d, 0x5e, 0x22, 0x7d, 0x5d,
                           0x99, 0xa7, 0x1e, 0x7e});
}

void setup() {
  common_setup_init();
  // pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalize.
  // delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_byte_sutffing_with_pre_flag);
  RUN_TEST(test_byte_sutffing_without_pre_flag);
  RUN_TEST(test_encode_command_packet_with_pre_flag);
  RUN_TEST(test_encode_command_packet_without_pre_flag);
  RUN_TEST(test_encode_response_packet_with_pre_flag);
  RUN_TEST(test_encode_response_packet_without_pre_flag);
  RUN_TEST(test_encode_message_packet_with_pre_flag);
  RUN_TEST(test_encode_message_packet_without_pre_flag);

  UNITY_END();
}

void loop() {
  common_loop_body();
  // digitalWrite(BUILTIN_LED, HIGH);
  // delay(500);
  // digitalWrite(BUILTIN_LED, LOW);
  // delay(500);
}
