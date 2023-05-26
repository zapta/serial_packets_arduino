
#include "serial_packets_test_utils.h"

#include <unity.h>

// For STM32 'black pill'.
#define BUILTIN_LED PC13

void populate_data(PacketData& data, const std::vector<uint8_t> bytes) {
  data.clear();
  TEST_ASSERT_GREATER_OR_EQUAL(bytes.size(), data.capacity());
  for (int i = 0; i < bytes.size(); i++) {
    data.write_uint8(bytes.at(i));
    TEST_ASSERT_FALSE(data.write_errors());
  }
  TEST_ASSERT_GREATER_OR_EQUAL(bytes.size(), data.size());
}

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



void common_setup_init() {
   pinMode(BUILTIN_LED, OUTPUT);

  // Time for the USB/CDC serial to stabalize.
  delay(2000);
}

void common_loop_body() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
}

// // Side affect of reseting the reading.
// void populate_data(PacketData& data, const std::vector<uint8_t> bytes) {
//   data.clear();
//   for (int i = 0; i < bytes.size(); i++) {
//     data.write_uint8(bytes.at(i));
//   }
//   TEST_ASSERT_FALSE(data.write_errors());
//   assert_data_equals(data, bytes);
// }
