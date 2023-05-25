#include <Arduino.h>
#include <unity.h>

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

#define LED PC13

int main(int argc, char **argv) {
  UNITY_BEGIN();

  // Serial.begin(115200);
  pinMode(LED, OUTPUT);

  for (;;) {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

  UNITY_END();
}