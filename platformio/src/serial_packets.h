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
  bool is_ready() { return false; }
  bool is_ok() { return false; }
  byte response_status() { return 0;}
  SerialPacketsData& response_data() {return _response_data; }
  void cancel() {}

  private:
    SerialPacketsData _response_data;
};

typedef void (*SerialPacketsCommandCallback)(byte endpoint,
                                             const SerialPacketsData& data,
                                             byte& response_status,
                                             SerialPacketsData& response_data);

typedef void (*SerialPacketsMessageCallback)(byte endpoint,
                                             const SerialPacketsData& data);

typedef void (*SerialPacketsEventCallback)(SeriaPacketsEvent event);

class SerialPackets {
 public:
  SerialPackets(SerialPacketsCommandCallback command_callback = nullptr,
                SerialPacketsMessageCallback message_callback = nullptr,
                SerialPacketsEventCallback event_callback = nullptr) {}

  bool Begin() { return false;}
  void Loop() { }
  bool SendCommand(byte endpoint, const SerialPacketsData& data,
                   const SerialPacketsPendingCommand& pending_command) {return false;}
  bool SendMessage(byte endpoint, const SerialPacketsData& data){return false;};

 private:
};

#endif  // SERIAL_PACKETS_H