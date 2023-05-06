
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#ifndef SERIAL_PACKETS_H
#define SERIAL_PACKETS_H

#include <Arduino.h>

enum SeriaPacketsEvent {

};

class SerialPacketsData {};

// class SerialPacketsCommandResponse {};

class SerialPacketsPendingCommand {
  // Ready
  //
  bool isReady() { return false; }
  bool isOk() { return false; }
  byte responseStatus() { return 0; }
  SerialPacketsData& responseData() { return _response_data; }
  void cancel() {}

 private:
  SerialPacketsData _response_data;
};

typedef void (*SerialPacketsCommandHandler)(byte endpoint,
                                            const SerialPacketsData& data,
                                            byte& response_status,
                                            SerialPacketsData& response_data);

typedef void (*SerialPacketsMessageHandler)(byte endpoint,
                                            const SerialPacketsData& data);

typedef void (*SerialPacketsEventHandler)(SeriaPacketsEvent event);

class SerialPacketsClient {
 public:
  SerialPacketsClient(SerialPacketsCommandHandler command_handler = nullptr,
                      SerialPacketsMessageHandler message_handler = nullptr,
                      SerialPacketsEventHandler event_handler = nullptr)
      : _command_handler(command_handler),
        _message_handler(message_handler),
        _event_handler(event_handler) {}

  void begin(HardwareSerial& data_serial, Print& debug_printer);

  void loop();

  bool sendCommand(byte endpoint, const SerialPacketsData& data,
                   const SerialPacketsPendingCommand& pending_command);

  bool sendMessage(byte endpoint, const SerialPacketsData& data);

 private:
  // Callback handlers. Set by the constructor.
  SerialPacketsCommandHandler const _command_handler;
  SerialPacketsMessageHandler const _message_handler;
  SerialPacketsEventHandler const _event_handler;

  // Serial ports. Set in begin()
  HardwareSerial* _data_serial = nullptr;
  Print* _debug_printer = nullptr;
};

#endif  // SERIAL_PACKETS_H