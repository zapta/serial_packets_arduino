#include "packet_consts.h"
#include "serial_packets.h"

void SerialPacketsClient::begin(Stream& data_stream, Print& debug_printer) {
  if (_data_stream) {
    debug_printer.println(
        "ERROR: Already called SerialPacktsBegin.bein(), ignoring.");
  }
  _data_stream = &data_stream;
  _debug_printer = &debug_printer;
  // Force pre flag on first packet.
  _pre_flag_timer.set(PRE_FLAG_TIMEOUT_MILLIS + 1)
}

void SerialPacketsClient::loop() {}

bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketsData& data,
    SerialPacketsCommandOutcomeHandler handler, uint32_t& cmd_id,
    uint16_t timeout) {
  // Assign a non zero cmd id. Wraparound ok.
  _cmd_id_counter++;
  if (_cmd_id_counter == 0) {
    _cmd_id_counter++;
  }

  // Determine if to insert a flag.
  const bool insert_pre_flag =
      _pre_flag_timer.millis() >= PRE_FLAG_TIMEOUT_MILLIS;

  // uint32_t millis_now

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_command_packet(_cmd_id_counter, endpoint, data,
                                             _tmp_data, insert_pre_flag)) {
    return false;
  }

  // TODO: Add a TX buffer.
  const uint16_t size = _tmp_data.data_size();
  uint16_t written = 0;

  while (written < size) {
    uint16_t availale = _data_stream->availableForWrite();
    const uint16_t pending = size - written;
    const uint16_t count = min(availale, pending);
    @@@@ write count bytes  
    written + count;
  }

  _pre_flag_timer.reset();
  cmd_id = _cmd_id_counter;
  return true;
}

bool SerialPacketsClient::sendMessage(byte endpoint,
                                      const SerialPacketsData& data) {
  return false;
};