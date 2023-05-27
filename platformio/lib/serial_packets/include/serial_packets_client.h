
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"
#include "serial_packets_data.h"
#include "serial_packets_decoder.h"
#include "serial_packets_encoder.h"
#include "serial_packets_logger.h"
#include "serial_packets_timer.h"

// #include "serial_buffer.h"

// Limit of command timeout values in millis. We want to
// keep timeout values reasonable so we can cleanup the
// command context table fast enough to prevent it from
// being filled.
constexpr uint16_t MAX_CMD_TIMEOUT_MILLIS = 10000;
constexpr uint16_t DEFAULT_CMD_TIMEOUT_MILLIS = 1000;

// enum SeriaPacketsEvent {
//   CONNECTED = 1,
//   DISCONNECTED = 2,
// };

// TODO: Change value names to includes PACKET_ prefix.
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
    byte command_endpoint, const SerialPacketsData& command_data,
    byte& response_status, SerialPacketsData& response_data);

// enum OutcomeCode {
//   OUTCOME_RESPONSE,
//   OUTCOME_TIMEOUT,
//   OUTCOME_INTERNAL_ERROR,
//   OUTCOME_CANCELED,
// };

typedef void (*SerialPacketsCommandResponseHandler)(
    uint32_t cmd_id, byte response_status,
    const SerialPacketsData& response_data);

typedef void (*SerialPacketsIncomingMessageHandler)(
    byte message_endpoint, const SerialPacketsData& message_data);

// typedef void (*SerialPacketsEventHandler)(SeriaPacketsEvent event);

class SerialPacketsClient {
 public:
  SerialPacketsClient(
      SerialPacketsIncomingCommandHandler command_handler = nullptr,
      SerialPacketsIncomingMessageHandler message_handler = nullptr)
      : _logger(SerialPacketsLogger::VERBOSE),
        _command_handler(command_handler),
        _message_handler(message_handler),
        // _event_handler(event_handler),
        // _tmp_data1(MAX_PACKET_DATA_LEN),
        // _tmp_data2(MAX_PACKET_DATA_LEN),
        _packet_encoder(_logger),
        _packet_decoder(_logger) {}

  void begin(Stream& data_stream, Stream& log_stream);
  void begin(Stream& data_stream);

  void loop();

  bool sendCommand(byte endpoint, const SerialPacketsData& data,

                   SerialPacketsCommandResponseHandler response_handler,
                   uint32_t& cmd_id, uint16_t timeout);

  bool sendMessage(byte endpoint, const SerialPacketsData& data);

  int num_pending_commands() {
    int count = 0;
    for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
      if (_command_contexts[i].cmd_id) {
        count++;
      }
    }
    return count;
  }

 private:
  // For testing.
  friend class SerialPacketsClientInspector;
  bool _ignore_rx_for_testing = false;

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

  SerialPacketsLogger _logger;

  // Callback handlers. Set by the constructor.
  SerialPacketsIncomingCommandHandler const _command_handler;
  SerialPacketsIncomingMessageHandler const _message_handler;
  // SerialPacketsEventHandler const _event_handler;

  Stream* _data_stream = nullptr;

  SerialPacketsData _tmp_data1;
  SerialPacketsData _tmp_data2;

  SerialPacketsEncoder _packet_encoder;
  SerialPacketsDecoder _packet_decoder;

  // Used to assign command ids. Wraparound is ok. Skipping zero value.
  uint32_t _cmd_id_counter = 0;
  // Used to insert pre flag when packates are sparse.
  SerialPacketsTimer _pre_flag_timer;
  // Used to periodically clean up timeout pending commands.
  SerialPacketsTimer _cleanup_timer;

  // The max number of in-progress outcoing commands.
  // static constexpr uint16_t MAX_CMD_CONTEXTS = 20;
  // Zero initialized.
  CommandContext _command_contexts[MAX_PENDING_COMMANDS];

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
                                       const SerialPacketsData& data);
  void process_decoded_command_packet(const DecodedCommandMetadata& metadata,
                                      const SerialPacketsData& data);
  void process_decoded_message_packet(const DecodedMessageMetadata& metadata,
                                      const SerialPacketsData& data);

  void force_next_pre_flag() {
    _pre_flag_timer.set(serial_packets_consts::PRE_FLAG_TIMEOUT_MILLIS + 1);
  }
  void loop_rx();
  void loop_cleanup();

  // Returns null if not found.
  CommandContext* find_context_with_cmd_id(uint32_t cmd_id) {
    for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
      if (_command_contexts[i].cmd_id == cmd_id) {
        return &_command_contexts[i];
      }
    }
    return nullptr;
  }

  bool check_pre_flag() {
    _data_stream->flush();
    const bool result = _pre_flag_timer.elapsed_millis() >
                        serial_packets_consts::PRE_FLAG_TIMEOUT_MILLIS;
    _pre_flag_timer.reset();
    return result;
  }
};
