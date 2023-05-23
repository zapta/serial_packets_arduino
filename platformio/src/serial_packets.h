
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>
#include "packet_data.h"
#include "packet_encoder.h"
#include "elapsed.h"
// #include "serial_buffer.h"

constexpr uint16_t MAX_DATA_LEN = 1024;
constexpr uint16_t TX_BUFFER_LEN = 4096;


enum SeriaPacketsEvent {
  CONNECTED = 1,
  DISCONNECTED = 2,
};


typedef void (*SerialPacketsIncomingCommandHandler)(
    byte command_endpoint, const SerialPacketData& command_data,
    byte& response_status, SerialPacketData& response_data);

enum OutcomeCode {
  OUTCOME_RESPONSE,
  OUTCOME_TIMEOUT,
  OUTCOME_INTERNAL_ERROR,
  OUTCOME_CANCELED,
};

typedef void (*SerialPacketsCommandOutcomeHandler)(
    uint32_t cmd_id, OutcomeCode cmd_outcome, byte response_status,
    const SerialPacketData& response_data);

typedef void (*SerialPacketsIncomingMessageHandler)(
    byte message_endpoint, const SerialPacketData& message_data);

typedef void (*SerialPacketsEventHandler)(SeriaPacketsEvent event);

class SerialPacketsClient {
 public:
  SerialPacketsClient(
      SerialPacketsIncomingCommandHandler command_handler = nullptr,
      SerialPacketsIncomingMessageHandler message_handler = nullptr,
      SerialPacketsEventHandler event_handler = nullptr)
      : _command_handler(command_handler),
        _message_handler(message_handler),
        _event_handler(event_handler) {}

  void begin(Stream& data_stream, Print& debug_printer);

  void loop();

  bool sendCommand(byte endpoint, const SerialPacketData& data,

                   SerialPacketsCommandOutcomeHandler handle, uint32_t& cmd_id,
                   uint16_t timeout);

  bool sendMessage(byte message_endpoint,
                   const SerialPacketData& message_data);

 private:
  // Callback handlers. Set by the constructor.
  SerialPacketsIncomingCommandHandler const _command_handler;
  SerialPacketsIncomingMessageHandler const _message_handler;
  SerialPacketsEventHandler const _event_handler;

  // Serial ports. Set in begin()
  Stream* _data_stream = nullptr;
  Print* _debug_printer = nullptr;

  // SerialBuffer _tx_data_serial_buffer;



  SerialPacketData _tmp_data;

  PacketEncoder _packet_encoder;

  // Used to assign command ids. Wraparound is ok. Skipping zero value.
  uint32_t _cmd_id_counter = 0;
  // Used to insert pre flag when packates are sparse.
  Elapsed _pre_flag_timer;

  bool check_pre_flag();

  inline uint32_t assigne_cmd_id() {
    _cmd_id_counter++;
    if (_cmd_id_counter == 0) {
      _cmd_id_counter++;
    }
    return _cmd_id_counter;
  }

  void loop_tx() ;
  void loop_rx() ;

};
