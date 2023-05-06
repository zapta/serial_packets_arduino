#include "serial_packets.h"

void SerialPacketsClient::begin(HardwareSerial& data_serial,
                                Print& debug_printer) {
  if (_data_serial) {
    debug_printer.println(
        "ERROR: Already called SerialPacktsBegin.bein(), ignoring.");
  }
  _data_serial = &data_serial;
  _debug_printer = &debug_printer;
}

void SerialPacketsClient::loop() {}

bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketsData& data,
    const SerialPacketsPendingCommand& pending_command) {
  return false;
}

bool SerialPacketsClient::sendMessage(byte endpoint,
                                      const SerialPacketsData& data) {
  return false;
};