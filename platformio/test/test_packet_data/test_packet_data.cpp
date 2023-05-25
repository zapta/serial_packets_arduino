// Uring test of the packet crc function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>

#include "packet_data.h"

// For STM32 'black pill'.
#define BUILTIN_LED PC13

void assert_data_equals(const PacketData& data, const uint8_t* expected,
                        int expected_size) {
  data.reset_reading();
  for (int i = 0; i < expected_size; i++) {
    const uint8_t b = data.read_uint8();
    TEST_ASSERT_FALSE(data.read_errors());
    TEST_ASSERT_EQUAL_HEX8(b, expected[i]);
    TEST_ASSERT_FALSE(data.read_errors());
  }
  TEST_ASSERT_TRUE(data.all_read_ok());
}

void test_constructor(void) {
  PacketData d;
  TEST_ASSERT_EQUAL(0, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(0, d.bytes_left_to_read());
  TEST_ASSERT_TRUE(d.all_read());
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_FALSE(d.write_errors());
  TEST_ASSERT_TRUE(d.all_read_ok());
}

void test_write_uint8(void) {
  PacketData d;
  d.write_uint8(0x02);
  TEST_ASSERT_EQUAL(1, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
  const uint8_t expected[] = {0x02};
  assert_data_equals(d, expected, sizeof(expected));
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalizes.
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_constructor);
  RUN_TEST(test_write_uint8);
  UNITY_END();
}

void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
}
