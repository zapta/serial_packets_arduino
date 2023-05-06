
// https://docs.arduino.cc/learn/contributions/arduino-library-style-guide

#ifndef SERIAL_PACKETS_H
#define SERIAL_PACKETS_H

#include <Arduino.h>

enum SeriaPacketsEvent {
  CONNECTED = 1,
  DISCONNECTED = 2,
};

class SerialPacketsData {
 public:
  SerialPacketsData(uint16_t max_data_size) { alloc_buffer(max_data_size); }
  ~SerialPacketsData() { free_buffer(); }

  // Disable copying and assignment.
  SerialPacketsData(const SerialPacketsData& other) = delete;
  SerialPacketsData& operator=(const SerialPacketsData& other) = delete;

  uint16_t max_data_size() { return _buffer_size; }
  uint16_t data_size() { return _data_size; }

  void dump_data(Stream& s);

  void clear_data() {
    _data_size = 0;
    _next_read_index = 0;
  }

  // Data writing
  bool add_byte(byte v);
  bool add_uint16(uint16_t v);
  bool add_uint32(uint32_t v);
  bool add_bytes_block(byte bytes[], uint32_t num_bytes);

  // Data reading
  uint16_t bytes_read() { return _next_read_index; }
  void reset_read_location() { _next_read_index = 0; }
  bool read_byte(byte& v);
  bool read_uint16(uint16_t& v);
  bool read_uint32(uint32_t& v);
  bool read_bytes_block(byte bytes_buffer[], uint32_t byte_buffer_size,
                        uint32_t& bytes_read);

 private:
  uint16_t _buffer_size = 0;
  uint16_t _data_size = 0;
  uint16_t _next_read_index = 0;
  byte* _buffer = nullptr;

  void free_buffer();
  bool alloc_buffer(uint16_t buffer_size);
};

class SerialPacketsPendingCommandResponse {
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

typedef void (*SerialPacketsCommandHandler)(
    byte command_endpoint, const SerialPacketsData& command_data,
    byte& response_status, SerialPacketsData& response_data);

typedef void (*SerialPacketsMessageHandler)(
    byte message_endpoint, const SerialPacketsData& message_data);

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

  bool sendCommand(byte command_endpoint, const SerialPacketsData& command_data,
                   const SerialPacketsPendingCommandResponse& pending_response);

  bool sendMessage(byte message_endpoint,
                   const SerialPacketsData& message_data);

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