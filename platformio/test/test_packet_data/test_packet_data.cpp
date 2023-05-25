// Uring test of the packet crc function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>
#include <vector>

#include "packet_data.h"

// For STM32 'black pill'.
#define BUILTIN_LED PC13

// Side affect of reseting the reading.
void assert_data_equals(const PacketData& data, const std::vector<uint8_t> expected) {
  data.reset_reading();
  for (int i = 0; i < expected.size(); i++) {
    const uint8_t b = data.read_uint8();
    TEST_ASSERT_FALSE(data.read_errors());
    TEST_ASSERT_EQUAL_HEX8(b, expected.at(i));
    TEST_ASSERT_FALSE(data.read_errors());
  }
  TEST_ASSERT_TRUE(data.all_read_ok());
  data.reset_reading();
}

// Side affect of reseting the reading.
void populate_data(const std::vector<uint8_t> bytes, PacketData* data) {
  data->clear();
  for (int i = 0; i < bytes.size(); i++) {
    data->write_uint8(bytes.at(i));
  }
  TEST_ASSERT_FALSE(data->write_errors());
  assert_data_equals(*data, bytes);
}

void test_constructor() {
  PacketData d;
  TEST_ASSERT_EQUAL(0, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(0, d.bytes_left_to_read());
  TEST_ASSERT_TRUE(d.all_read());
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_FALSE(d.write_errors());
  TEST_ASSERT_TRUE(d.all_read_ok());
}

void test_write_uint8() {
  PacketData d;
  d.write_uint8(0x02);
  TEST_ASSERT_EQUAL(1, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
  assert_data_equals(d, {0x02});
}

void test_read_uint8() {
  PacketData d;
  populate_data({0x11, 0x22, 0x33}, &d);
  const uint8_t v1 = d.read_uint8();
  const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(v1, 0x11);
  TEST_ASSERT_EQUAL(v2, 0x22);
  TEST_ASSERT_EQUAL(2, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalize.
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_constructor);
  RUN_TEST(test_write_uint8);
  RUN_TEST(test_read_uint8);
  UNITY_END();
}

void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
}
