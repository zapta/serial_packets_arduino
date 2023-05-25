// Uring test of the packet crc function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>

#include "packet_crc.h"

// For STM32 'black pill'.
#define BUILTIN_LED PC13

void test_empty_data(void) {
  const uint8_t data[] = {};
  const uint16_t crc = packet_crc::gen_crc16(data, sizeof(data));
  TEST_ASSERT_EQUAL(0xffff, crc);
}

// Per https://srecord.sourceforge.net/crc16-ccitt.html#results this is
// the wrong result, but it's a common mistake, and compatible with the
// the Python library we use.
void test_data1(void) {
  const uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
  const uint16_t crc = packet_crc::gen_crc16(data, sizeof(data));
  TEST_ASSERT_EQUAL_HEX32(0x29b1, crc);
}

void test_data2(void) {
  const uint8_t data[] = {0x01, 0x00, 0x00, 0x00, 0x07, 0x14,
                          0xc8, 0x00, 0x00, 0x04, 0xd2};
  const uint16_t crc = packet_crc::gen_crc16(data, sizeof(data));
  TEST_ASSERT_EQUAL_HEX32(0x1f49, crc);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalizes.
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_empty_data);
  RUN_TEST(test_data1);
  RUN_TEST(test_data2);
  UNITY_END();
}

void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
}
