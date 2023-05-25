// Uring test of the packet crc function.

// For test example see here
// https://github.com/platformio/platformio-examples/blob/develop/unit-testing/calculator/test/test_embedded/test_calculator.cpp

#include <Arduino.h>
#include <unity.h>
#include "packet_crc.h"

#define LED PC13

// void setUp(void) {
//   // set stuff up here
// }

// void tearDown(void) {
//   // clean stuff up here
// }

// void test_calculator_addition(void) {
//   // TEST_ASSERT_EQUAL(32, calc.add(25, 7));
// }

// void test_calculator_subtraction(void) {
//   // TEST_ASSERT_EQUAL(20, calc.sub(23, 3));
// }

// void test_calculator_multiplication(void) {
//   // TEST_ASSERT_EQUAL(50, calc.mul(25, 2));
//      TEST_ASSERT_EQUAL(1, 2);

// }

void test_empty_data(void) {
  const uint8_t data[] = {};
  const uint16_t crc = packet_crc::gen_crc16(data, sizeof(data));
  TEST_ASSERT_EQUAL(0xffff, crc);
}

void setup() {
  // pinMode(LED, OUTPUT);
  // digitalWrite(LED, LOW);

  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();
  // RUN_TEST(test_calculator_addition);
  // RUN_TEST(test_calculator_subtraction);
  // RUN_TEST(test_calculator_multiplication);
  RUN_TEST(test_empty_data);
  UNITY_END();
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
