// The main APi of the Serial Packets library package.

// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#pragma once

#include <Arduino.h>

#include "serial_packets_consts.h"
#include "serial_packets_data.h"
#include "serial_packets_decoder.h"
#include "serial_packets_encoder.h"
#include "serial_packets_logger.h"
#include "serial_packets_timer.h"

// We limit the command timeout duration to avoid accomulation of
// pending commands.
constexpr uint16_t MAX_CMD_TIMEOUT_MILLIS = 10000;
constexpr uint16_t DEFAULT_CMD_TIMEOUT_MILLIS = 1000;

// Define status codes of command responses.
enum PacketStatus {
  OK = 0,
  GENERAL_ERROR = 1,
  TIMEOUT = 2,
  UNHANDLED = 3,
  INVALID_ARGUMENT = 4,
  LENGTH_ERROR = 5,
  OUT_OF_RANGE = 6,
  NOT_CONNECTED = 7,

  // Reserved for application codes from here to 255.
  USER_ERRORS_BASE = 100,
};

// A callback type for all incoming commands. Handler should
// set response_status and response_data with the response
// info.
typedef void (*SerialPacketsIncomingCommandHandler)(
    byte command_endpoint, const SerialPacketsData& command_data,
    byte& response_status, SerialPacketsData& response_data);

// A callback type for incoming command response.
typedef void (*SerialPacketsCommandResponseHandler)(
    uint32_t cmd_id, byte response_status,
    const SerialPacketsData& response_data);

// A callback type for incoming messages.
typedef void (*SerialPacketsIncomingMessageHandler)(
    byte message_endpoint, const SerialPacketsData& message_data);

// Constructor.
class SerialPacketsClient {
 public:
  SerialPacketsClient(
      SerialPacketsIncomingCommandHandler command_handler = nullptr,
      SerialPacketsIncomingMessageHandler message_handler = nullptr)
      : _logger(SERIAL_PACKETS_LOG_VERBOSE),
        _optional_command_handler(command_handler),
        _optional_message_handler(message_handler),
        _packet_encoder(_logger),
        _packet_decoder(_logger) {}

  // Initialize the client with a serial stream for data
  // communication and an optional serial stream for debug
  // log.
  void begin(Stream& data_stream, Stream& log_stream);
  void begin(Stream& data_stream);

  // This method should be called frequently from the main
  // loop() of the program. Program should be non blocking
  // (avoid delay()) such that this method is called frequently.
  // It process incoming data, invokes the callback handlers
  // and cleans up timeout commands.
  void loop();

  // Adjust log level. If 
  void setLogLevel(SerialPacketsLogLevel level) {
    _logger.set_level(level);
  }

  // Send a command to given endpoint and with given data. Use the
  // provided response_handler to pass the command response or
  // timeout information. Returns true if the command was sent.
  //
  // TODO: Pass also uint32_t user data.
  bool sendCommand(byte endpoint, const SerialPacketsData& data,
                   SerialPacketsCommandResponseHandler response_handler,
                   uint32_t& cmd_id, uint16_t timeout);

  // TODO: add a cancelCommand() method.

  // Send a message to given endpoint and with given data. Returns
  // true if the message was sent. There is not positive verification
  // that the message was actually recieved at the other side. For
  // this, use a command instead.
  bool sendMessage(byte endpoint, const SerialPacketsData& data);

  // Returns the number of in progress commands that wait for a
  // response or to timeout. The max number of allowed pending
  // messages is configurable.
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

  // Contains the information of a single pending command.
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

  // Logger for diagnostics messages.
  SerialPacketsLogger _logger;

  // User provided command handler. May be null.
  SerialPacketsIncomingCommandHandler const _optional_command_handler;
  // user provided message handler. May be null.
  SerialPacketsIncomingMessageHandler const _optional_message_handler;

  Stream* _data_stream = nullptr;

  SerialPacketsData _tmp_data;
  StuffedPacketBuffer _tmp_stuffed_packet;


  SerialPacketsEncoder _packet_encoder;
  SerialPacketsDecoder _packet_decoder;

  // Used to assign command ids. Wraparound is ok. Skipping zero values.
  uint32_t _cmd_id_counter = 0;
  // Used to insert pre packet flag byte when packates are sparse.
  SerialPacketsTimer _pre_flag_timer;
  // Used to periodically clean pending commands that timeout.
  SerialPacketsTimer _cleanup_timer;

  // A table that contains information about pending commands.
  CommandContext _command_contexts[MAX_PENDING_COMMANDS];

  // Returns true if begun already called.
  inline bool begun() {
    return _data_stream != nullptr;
  }

  // Assign a fresh command id. Guaranteed to be non zero.
  // Wrap arounds are OK since we clea up timeout commands.
  inline uint32_t assign_cmd_id() {
    _cmd_id_counter++;
    if (_cmd_id_counter == 0) {
      _cmd_id_counter++;
    }
    return _cmd_id_counter;
  }

  // Methos that used to process incoming packets that were
  // decoder by the packet decoder.
  void process_decoded_response_packet(const DecodedResponseMetadata& metadata,
                                       const SerialPacketsData& data);
  void process_decoded_command_packet(const DecodedCommandMetadata& metadata,
                                      const SerialPacketsData& data);
  void process_decoded_message_packet(const DecodedMessageMetadata& metadata,
                                      const SerialPacketsData& data);

  // Manipulate the pre flag timer to force a pre packet flag byte
  // before next packet. Invoked when when we detect errors on the line.
  void force_next_pre_flag() {
    _pre_flag_timer.set(serial_packets_consts::PRE_FLAG_TIMEOUT_MILLIS + 1);
  }

  // Subfunctionalties of loop().
  void loop_rx();
  void loop_cleanup();

  // Lookup for a pending command entry with given cmd_id. If cmd_id is
  // zero, it finds a free entry. Returns null if not found. Implemented
  // using a simple linear search.
  CommandContext* find_context_with_cmd_id(uint32_t cmd_id) {
    for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
      if (_command_contexts[i].cmd_id == cmd_id) {
        return &_command_contexts[i];
      }
    }
    return nullptr;
  }

  // Determine if the interval from previous packet is large enough that
  // warrants the insertion of a flag byte beofore next packet.
  bool check_pre_flag() {
    _data_stream->flush();
    const bool result = _pre_flag_timer.elapsed_millis() >
                        serial_packets_consts::PRE_FLAG_TIMEOUT_MILLIS;
    _pre_flag_timer.reset();
    return result;
  }
};
