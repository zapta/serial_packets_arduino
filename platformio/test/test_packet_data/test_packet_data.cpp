// Uring test of the packet crc function.

// Based on this example.
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>

#include <vector>

#include "packet_data.h"

// For STM32 'black pill'.
#define BUILTIN_LED PC13

// TODO: Move these two testing function to a shared place
//  and use in other tests as well.

// Side affect of reseting the reading.
void assert_data_equals(const PacketData& data,
                        const std::vector<uint8_t> expected) {
  TEST_ASSERT_EQUAL(data.size(), expected.size());
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
void populate_data(PacketData& data, const std::vector<uint8_t> bytes) {
  data.clear(); 
  for (int i = 0; i < bytes.size(); i++) {
    data.write_uint8(bytes.at(i));
  }
  TEST_ASSERT_FALSE(data.write_errors());
  assert_data_equals(data, bytes);
}

void test_constructor() {
  PacketData d(20);
  TEST_ASSERT_EQUAL(20, d.capacity());  // Currently default is 16
  TEST_ASSERT_EQUAL(0, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(0, d.bytes_left_to_read());
  TEST_ASSERT_TRUE(d.all_read());
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_FALSE(d.write_errors());
  TEST_ASSERT_TRUE(d.all_read_ok());
}



void test_write_uint8() {
  PacketData d(20);
  d.write_uint8(0x02);
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(1, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
  assert_data_equals(d, {0x02});
}

void test_write_uint8_new_error() {
  PacketData d(4);
  populate_data(d, {0x11, 0x22, 0x33, 0x44});
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_TRUE(d.is_full());
  d.write_uint8(0x02);
  TEST_ASSERT_TRUE(d.write_errors());
  TEST_ASSERT_EQUAL(4, d.size());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44});
}

void test_write_uint16() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22});
  d.write_uint16(0x3344);
  TEST_ASSERT_EQUAL(4, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(4, d.bytes_left_to_read());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44});
}

void test_write_uint16_new_error() {
  PacketData d(4);
  populate_data(d, {0x11, 0x22, 0x33});
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(1, d.free_bytes());
  d.write_uint16(0x8899);
  TEST_ASSERT_TRUE(d.write_errors());
  TEST_ASSERT_EQUAL(3, d.size());
  assert_data_equals(d, {0x11, 0x22, 0x33});
}

void test_write_uint32() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22});
  d.write_uint32(0x33445566);
  TEST_ASSERT_EQUAL(6, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(6, d.bytes_left_to_read());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
}

void test_write_uint32_new_error() {
  PacketData d(6);
  populate_data(d, {0x11, 0x22, 0x33});
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(3, d.free_bytes());
  d.write_uint32(0x8899aabb);
  TEST_ASSERT_TRUE(d.write_errors());
  TEST_ASSERT_EQUAL(3, d.size());
  assert_data_equals(d, {0x11, 0x22, 0x33});
}

void test_write_bytes() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22});
  const uint8_t bytes[] = {0x33, 0x44, 0x55};
  d.write_bytes(bytes, sizeof(bytes));
  TEST_ASSERT_EQUAL(5, d.size());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(5, d.bytes_left_to_read());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44, 0x55});
}

void test_write_bytes_new_error() {
  PacketData d(5);
  populate_data(d, {0x11, 0x22, 0x33});
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(2, d.free_bytes());
  const uint8_t bytes[] = {0x33, 0x44, 0x55};
  d.write_bytes(bytes, sizeof(bytes));
  TEST_ASSERT_TRUE(d.write_errors());
  TEST_ASSERT_EQUAL(3, d.size());
  assert_data_equals(d, {0x11, 0x22, 0x33});
}

void test_write_data() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22});
  PacketData value(20);
  populate_data(value, {0x33, 0x44, 0x55});
  d.write_data(value);
  TEST_ASSERT_EQUAL(5, d.size());
  TEST_ASSERT_EQUAL_UINT8(0, d.bytes_read());
  TEST_ASSERT_EQUAL(5, d.bytes_left_to_read());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44, 0x55});
}

void test_write_data_new_error() {
  PacketData d(5);
  populate_data(d, {0x11, 0x22, 0x33});
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL(2, d.free_bytes());
  PacketData value(20);
  populate_data(value, {0x33, 0x44, 0x55});
  d.write_data(value);
  TEST_ASSERT_TRUE(d.write_errors());
  TEST_ASSERT_EQUAL(3, d.size());
  assert_data_equals(d, {0x11, 0x22, 0x33});
}

void test_read_uint8() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33});
  const uint8_t v1 = d.read_uint8();
  const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX8(v1, 0x11);
  TEST_ASSERT_EQUAL_HEX8(v2, 0x22);
  TEST_ASSERT_EQUAL(2, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
}

void test_read_uint8_new_error() {
  PacketData d(20);
  // populate_data(d, {0x11, 0x22, 0x33});
  const uint8_t v = d.read_uint8();
  // const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_TRUE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX8(v, 0x00);
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(0, d.bytes_left_to_read());
}

void test_read_uint16() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33});
  const uint16_t v = d.read_uint16();
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX16(v, 0x1122);
  TEST_ASSERT_EQUAL(2, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
}

void test_read_uint16_new_error() {
  PacketData d(20);
  populate_data(d, {0x11});
  const uint16_t v = d.read_uint16();
  // const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_TRUE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX16(v, 0);
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
}

void test_read_uint32() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33, 0x44, 0x55});
  const uint32_t v = d.read_uint32();
  TEST_ASSERT_FALSE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX32(v, 0x11223344);
  TEST_ASSERT_EQUAL(4, d.bytes_read());
  TEST_ASSERT_EQUAL(1, d.bytes_left_to_read());
}

void test_read_uint32_new_error() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33});
  const uint32_t v = d.read_uint32();
  // const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_TRUE(d.read_errors());
  TEST_ASSERT_EQUAL_HEX32(v, 0x00);
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(3, d.bytes_left_to_read());
}

void test_read_bytes() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33, 0x44, 0x55});
  uint8_t actual[] = {0x99, 0x99, 0x99, 0x99};
  // We read into the second and third bytes.
  d.read_bytes(actual + 1, 2);
  TEST_ASSERT_FALSE(d.read_errors());
  const uint8_t expected[] = {0x99, 0x11, 0x22, 0x99};
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, actual, 4);
  TEST_ASSERT_EQUAL(2, d.bytes_read());
  TEST_ASSERT_EQUAL(3, d.bytes_left_to_read());
}

void test_read_bytes_new_error() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22});
  uint8_t actual[] = {0x11, 0x22, 0x33};
  d.read_bytes(actual, sizeof(actual));
  // const uint8_t v2 = d.read_uint8();
  TEST_ASSERT_TRUE(d.read_errors());
  const uint8_t expected[] = {0x00, 0x00, 0x00};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, 3);
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(2, d.bytes_left_to_read());
}

void test_read_data() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33, 0x44, 0x55});
  PacketData value(20);
  populate_data(value, {0x88, 0x99});
  d.read_data(value, 2);
  TEST_ASSERT_FALSE(d.read_errors());
  assert_data_equals(value, {0x88, 0x99, 0x11, 0x22});
  TEST_ASSERT_EQUAL(2, d.bytes_read());
  TEST_ASSERT_EQUAL(3, d.bytes_left_to_read());
}

// Trying to read more bytes than available in the source data.
void test_read_data_new_src_error() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33, 0x44});
  PacketData value(10);
  populate_data(value, {0xaa, 0xbb});
  d.read_data(value, 5);
  TEST_ASSERT_TRUE(d.read_errors());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(4, d.bytes_left_to_read());
    TEST_ASSERT_FALSE(value.read_errors());
    TEST_ASSERT_FALSE(value.write_errors());

  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44});
  assert_data_equals(value, {0xaa, 0xbb});

}

// Trying to read more bytes than available in the destination data.
void test_read_data_new_dst_error() {
  PacketData d(20);
  populate_data(d, {0x11, 0x22, 0x33, 0x44});
  PacketData value(3);
  d.read_data(value, 4);
  TEST_ASSERT_TRUE(d.read_errors());
  TEST_ASSERT_EQUAL(0, d.bytes_read());
  TEST_ASSERT_EQUAL(4, d.bytes_left_to_read());
     TEST_ASSERT_FALSE(value.read_errors());
    TEST_ASSERT_FALSE(value.write_errors());
  assert_data_equals(d, {0x11, 0x22, 0x33, 0x44});
  assert_data_equals(value, {});
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalize.
  delay(2000);

  UNITY_BEGIN();
  RUN_TEST(test_constructor);

  // Writing
  RUN_TEST(test_write_uint8);
  RUN_TEST(test_write_uint8_new_error);
  RUN_TEST(test_write_uint16);
  RUN_TEST(test_write_uint16_new_error);
  RUN_TEST(test_write_uint32);
  RUN_TEST(test_write_uint32_new_error);
  RUN_TEST(test_write_bytes);
  RUN_TEST(test_write_bytes_new_error);
  RUN_TEST(test_write_data);
  RUN_TEST(test_write_data_new_error);

  // Reading
  RUN_TEST(test_read_uint8);
  RUN_TEST(test_read_uint8_new_error);
  RUN_TEST(test_read_uint16);
  RUN_TEST(test_read_uint16_new_error);
  RUN_TEST(test_read_uint32);
  RUN_TEST(test_read_uint32_new_error);
  RUN_TEST(test_read_bytes);
  RUN_TEST(test_read_bytes_new_error);
  RUN_TEST(test_read_data);
  RUN_TEST(test_read_data_new_src_error);
  RUN_TEST(test_read_data_new_dst_error);

  UNITY_END();
}

void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
}
