#include "crc.h"

#include <Arduino.h>

// From
// https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum

#define CRC16 0x8005

uint16_t gen_crc16(const uint8_t*  data, const uint16_t size) {
  uint16_t n = size;
  const uint8_t* p_data = data;

  uint16_t out = 0;
  int bits_read = 0, bit_flag;

  /* Sanity check: */
  if (p_data == NULL) return 0;

  while (n > 0) {
    bit_flag = out >> 15;

    /* Get next bit: */
    out <<= 1;
    out |= (*p_data >> bits_read) &
           1;  // item a) work from the least significant bits

    /* Increment bit counter: */
    bits_read++;
    if (bits_read > 7) {
      bits_read = 0;
      p_data++;
      n--;
    }

    /* Cycle check: */
    if (bit_flag) out ^= CRC16;
  }

  // item b) "push out" the last 16 bits
  int i;
  for (i = 0; i < 16; ++i) {
    bit_flag = out >> 15;
    out <<= 1;
    if (bit_flag) out ^= CRC16;
  }

  // item c) reverse the bits
  uint16_t crc = 0;
  i = 0x8000;
  int j = 0x0001;
  for (; i != 0; i >>= 1, j <<= 1) {
    if (i & out) crc |= j;
  }

  Serial.printf("\nCRC: ");
  for (int i = 0; i < size; i++) {
    Serial.printf(" %02hx", data[i]);
  }
  Serial.printf(" -> %04hx\n\n", crc);

  return crc;
}