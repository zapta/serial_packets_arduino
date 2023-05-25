// Internal consts.

#pragma once

#include <Arduino.h>

// #include "serial_packets.h"

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

// Max size of a packet data. User can override. Impacts memory 
// usage.
#ifndef CONFIG_MAX_PACKET_DATA_LEN
constexpr uint16_t MAX_DATA_LEN = 100;
#else
constexpr uint16_t MAX_DATA_LEN = (CONFIG_MAX_PACKET_DATA_LEN);
#endif

// Controls the size of the pending outgoing commands table.
// User can override. Impacts memory usage and performance
// (due to linear search).
#ifndef CONFIG_MAX_PENDING_COMMANDS
static constexpr uint16_t MAX_PENDING_COMMANDS = 20;
#else
static constexpr uint16_t MAX_CMD_CONTEXTS = (CONFIG_MAX_PENDING_COMMANDS);
#endif


// Sizes before flagging and byte stuffing.
constexpr uint16_t MIN_PACKET_LEN = MIN_PACKET_OVERHEAD;
constexpr uint16_t MAX_PACKET_LEN = MAX_PACKET_OVERHEAD + MAX_DATA_LEN;



// Numeric values are serialized to the wire so do not change.
enum PacketType {
  TYPE_COMMAND = 1,
  TYPE_RESPONSE = 2,
  TYPE_MESSAGE = 3,
};
