#pragma once

#include <Arduino.h>

namespace packet_crc {
  
uint16_t gen_crc16(const uint8_t *data, int size);

} // namespace packet_crc
