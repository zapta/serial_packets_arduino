#pragma once

#include <Arduino.h>

// namespace serial_packets {
  
uint16_t serial_packets_gen_crc16(const uint8_t *data, int size);

// } // namespace serial_packets
