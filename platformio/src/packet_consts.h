// Internal consts.

#pragma once

#include <Arduino.h>

#include "serial_packets.h"

// Flag and escape bytes per HDLC specification.
constexpr uint8_t PACKET_FLAG = 0x7E;
constexpr uint8_t PACKET_ESC = 0X7D;

// Prefix a packet with a flag byte only if interval from previous
// encoded packet is longer that this time in milliseconds.
constexpr uint16_t PRE_FLAG_TIMEOUT_MILLIS = 1000;

// Packet sizes in bytes, with zero data length, and before
// byte stuffing, and flagging.
constexpr uint16_t MIN_PACKET_OVERHEAD = 4;
constexpr uint16_t MAX_PACKET_OVERHEAD = 8;

// Sizes before flagging and byte stuffing.
constexpr uint16_t MIN_PACKET_LEN = MIN_PACKET_OVERHEAD;
constexpr uint16_t MAX_PACKET_LEN = MAX_PACKET_OVERHEAD + MAX_DATA_LEN;

// Range of command timeout values in millis.
constexpr uint16_t MIN_CMD_TIMEOUT = 100;
constexpr uint16_t MAX_CMD_TIMEOUT = 10000;
constexpr uint16_t DEFAULT_CMD_TIMEOUT = 1000;

// Numeric values are serialized to the wire so do not change.
enum PacketType {
  TYPE_COMMAND = 1,
  TYPE_RESPONSE = 2,
  TYPE_MESSAGE = 3,
};
