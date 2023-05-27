// Packet CRC function.

#pragma once

#include <Arduino.h>

uint16_t serial_packets_gen_crc16(const uint8_t *data, int size);
