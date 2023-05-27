// Serial Packets consts.

#pragma once

#include <Arduino.h>

// Max size of a packet data. User can override. Impacts memory
// usage, so use discretion.
#ifndef CONFIG_MAX_PACKET_DATA_LEN
constexpr uint16_t MAX_PACKET_DATA_LEN = 100;
#else
constexpr uint16_t MAX_PACKET_DATA_LEN = (CONFIG_MAX_PACKET_DATA_LEN);
#endif

// Controls the size of the pending outgoing commands table.
// User can override. Impacts memory usage and performance
// (due to linear search).
#ifndef CONFIG_MAX_PENDING_COMMANDS
static constexpr uint16_t MAX_PENDING_COMMANDS = 20;
#else
static constexpr uint16_t MAX_PENDING_COMMANDS = (CONFIG_MAX_PENDING_COMMANDS);
#endif

// Internal consts that users don't need to access.
namespace serial_packets_consts {

// Flag and escape bytes per HDLC specification.
constexpr uint8_t PACKET_FLAG = 0x7E;
constexpr uint8_t PACKET_ESC = 0X7D;

// When sending a packet, if the time from previous packate was
// longer than these, than we insert a pre packet flag byte.
constexpr uint16_t PRE_FLAG_TIMEOUT_MILLIS = 1000;

// Packet sizes in bytes, with zero data length, and before
// byte stuffing, and flagging.
constexpr uint16_t MIN_PACKET_OVERHEAD = 4;
constexpr uint16_t MAX_PACKET_OVERHEAD = 8;

// Range of packet lengths before flagging and byte stuffing.
constexpr uint16_t MIN_PACKET_LEN = MIN_PACKET_OVERHEAD;
constexpr uint16_t MAX_PACKET_LEN = MAX_PACKET_OVERHEAD + MAX_PACKET_DATA_LEN;

// The value of the packet_type field of the packet. Defines the
// packet type.
enum PacketType {
  TYPE_COMMAND = 1,
  TYPE_RESPONSE = 2,
  TYPE_MESSAGE = 3,
};

}  // namespace serial_packets_consts
