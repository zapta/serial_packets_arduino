
#include "serial_packets_test_utils.h"

#include <unity.h>


void assert_vectors_equal(const std::vector<uint8_t> expected,
                          const std::vector<uint8_t> actual) {
  TEST_ASSERT_EQUAL(expected.size(), actual.size());
  for (int i = 0; i < expected.size(); i++) {
    TEST_ASSERT_EQUAL_HEX8(expected.at(i), actual.at(i));
  }
}

void loop_client(SerialPacketsClient& client, uint32_t time_millis) {
  const uint32_t start_time_millis = millis();
  for (;;) {
    client.loop();
    if ((millis() - start_time_millis) >= time_millis) {
      return;
    }
  }
}

void common_setup_init() {
  pinMode(LED_BUILTIN, OUTPUT);

  // Time for the USB/CDC serial to stabalize.
  delay(2000);
}

void common_loop_body() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(600);
}
