
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>

#include "elapsed.h"
#include "packet_data.h"
#include "packet_decoder.h"
#include "packet_encoder.h"
#include "logger.h"

// #include "serial_buffer.h"

// Range of command timeout values in millis.
constexpr uint16_t MIN_CMD_TIMEOUT_MILLIS = 100;
constexpr uint16_t MAX_CMD_TIMEOUT_MILLIS = 10000;
constexpr uint16_t DEFAULT_CMD_TIMEOUT_MILLIS = 1000;

enum SeriaPacketsEvent {
  CONNECTED = 1,
  DISCONNECTED = 2,
};

enum PacketStatus {
  // """Defines status codes. User NAME.value to convert to int.
  // valid values are [0, 255]
  // """
  OK = 0,
  GENERAL_ERROR = 1,
  TIMEOUT = 2,
  UNHANDLED = 3,
  INVALID_ARGUMENT = 4,
  LENGTH_ERROR = 5,
  OUT_OF_RANGE = 6,
  NOT_CONNECTED = 7,

  // # Users can start allocating error codes from
  // # here to 255.
  USER_ERRORS_BASE = 100,
};

typedef void (*SerialPacketsIncomingCommandHandler)(
    byte command_endpoint, const SerialPacketData& command_data,
    byte& response_status, SerialPacketData& response_data);

// enum OutcomeCode {
//   OUTCOME_RESPONSE,
//   OUTCOME_TIMEOUT,
//   OUTCOME_INTERNAL_ERROR,
//   OUTCOME_CANCELED,
// };

typedef void (*SerialPacketsCommandResponseHandler)(
    uint32_t cmd_id, byte response_status,
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

  void begin(Stream& data_stream,  Stream& log_stream);
  void begin(Stream& data_stream);

  void loop();

  bool sendCommand(byte endpoint, const SerialPacketData& data,

                   SerialPacketsCommandResponseHandler response_handler,
                   uint32_t& cmd_id, uint16_t timeout);

  bool sendMessage(byte endpoint, const SerialPacketData& data);


 private:
  struct CommandContext {
    CommandContext() { clear(); }

    // Zero if this command context is non active.
    uint32_t cmd_id;
    SerialPacketsCommandResponseHandler response_handler;
    uint32_t expiration_time_millis;

    void clear() {
      cmd_id = 0;
      response_handler = nullptr;
      expiration_time_millis = 0;
    }
  };

  // static  Logger null_logger;

  // Callback handlers. Set by the constructor.
  SerialPacketsIncomingCommandHandler const _command_handler;
  SerialPacketsIncomingMessageHandler const _message_handler;
  SerialPacketsEventHandler const _event_handler;

  Stream* _data_stream = nullptr;
  Logger _logger;

  SerialPacketData _tmp_data1;
  SerialPacketData _tmp_data2;

  PacketEncoder _packet_encoder;
  PacketDecoder _packet_decoder;

  // Used to assign command ids. Wraparound is ok. Skipping zero value.
  uint32_t _cmd_id_counter = 0;
  // Used to insert pre flag when packates are sparse.
  Elapsed _pre_flag_timer;

  // The max number of in-progress outcoing commands.
  static constexpr uint16_t MAX_CMD_CONTEXTS = 20;
  // Zero initialized.
  CommandContext command_contexts[MAX_CMD_CONTEXTS];

  // bool check_pre_flag();

  // Assign a fresh command id. Guaranteed to be non zero.
  // Wrap arounds are OK since it's longer than max command
  // expiration time.
  inline uint32_t assign_cmd_id() {
    _cmd_id_counter++;
    if (_cmd_id_counter == 0) {
      _cmd_id_counter++;
    }
    return _cmd_id_counter;
  }

  // Once the decoder reports a new decoded packet, this is called
  // to process it.
  void process_decoded_response_packet(const DecodedResponseMetadata& metadata,
                                       const SerialPacketData& data);
  void process_decoded_command_packet(const DecodedCommandMetadata& metadata,
                                      const SerialPacketData& data);
                                      void process_decoded_message_packet(
    const DecodedMessageMetadata& metadata, const SerialPacketData& data);

                      void force_next_pre_flag() {
                          _pre_flag_timer.set(PRE_FLAG_TIMEOUT_MILLIS + 1);

                      }

  // Returns null if not found.
  CommandContext* find_context_with_cmd_id(uint32_t cmd_id) {
    for (int i = 0; i < MAX_CMD_CONTEXTS; i++) {
      if (command_contexts[i].cmd_id == cmd_id) {
        return &command_contexts[i];
      }
    }
    return nullptr;
  }

  bool check_pre_flag() {
  _data_stream->flush();
  const bool result =
      _pre_flag_timer.elapsed_millis() > PRE_FLAG_TIMEOUT_MILLIS;
  _pre_flag_timer.reset();
  return result;
}
};
