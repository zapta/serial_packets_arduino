#include "serial_packets.h"

#include "packet_consts.h"

void SerialPacketsClient::begin(Stream& data_stream, Print& debug_printer) {
  if (_data_stream) {
    debug_printer.println(
        "ERROR: Already called SerialPacktsBegin.bein(), ignoring.");
  }
  _data_stream = &data_stream;
  _debug_printer = &debug_printer;
  // Force pre flag on first packet.
  _pre_flag_timer.set(PRE_FLAG_TIMEOUT_MILLIS + 1);
}

// static uint8_t tx_transfer_buffer[100];

// void SerialPacketsClient::loop_tx() {
//   // Move from tx buffer to serial stream if needed.
//   const uint16_t data_size = _tx_data_serial_buffer.size();
//   if (data_size == 0) {
//     return;
//   }

//   const uint16_t available_to_write = _data_stream->availableForWrite();
//   if (available_to_write == 0) {
//     return;
//   }

//   uint16_t n = sizeof(tx_transfer_buffer);
//   n = min(n, data_size);
//   n = min(n, available_to_write);

//   const bool ok = _tx_data_serial_buffer.pop(tx_transfer_buffer, n);
//   if (!ok) {
//     return;
//   }

//   const int written = _data_stream->write(tx_transfer_buffer, n);

//   // TODO: Assert or print an error if not fully written.
// }

static uint8_t rx_transfer_buffer[100];

void SerialPacketsClient::loop() {
  // If needed, transfer bytes from tx queue to stream.
  // loop_tx();


  // Decode incoming data.
   uint16_t n = _data_stream->available();
  while (n > 0) {
    const uint16_t count = min((uint16_t)sizeof(rx_transfer_buffer), n) ;
    const uint16_t bytes_read = _data_stream->readBytes(rx_transfer_buffer, count);
    // TODO: Assert that bytes_read == count, or at least > 0.
    for (int i = 0; i < bytes_read; i++) {
      // Send the byte to the decoder;_
    }
  }
}


bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketData& data,
    SerialPacketsCommandOutcomeHandler handler, uint32_t& cmd_id,
    uint16_t timeout) {
  // Prepare for failure.
  cmd_id = 0;

  const uint32_t new_cmd_id = assigne_cmd_id();
  // Assign a non zero cmd id. Wraparound ok.

  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_command_packet(new_cmd_id, endpoint, data,
                                             _tmp_data, insert_pre_flag)) {
    return false;
  }


  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_data.size();
  if (_data_stream->available() < size) {
    return false;
  }

  // NOTE: We assume that this will not be blocking since we verified 
  // the number of avilable bytes in the buffer. We force large buffer
  // using a build flag such as -DSERIAL_TX_BUFFER_SIZE=4096.
  const uint16_t written = _data_stream->write(_tmp_data._buffer, size);

  // TOOD: Assert that all was written.  If writes are partial, consider
  // to add our own tx buffer and transfer from it to the serial 
  // stream in the loop().

  cmd_id = new_cmd_id;
  return true;
}

bool SerialPacketsClient::sendMessage(byte endpoint,
                                      const SerialPacketData& data) {
  return false;
};

bool SerialPacketsClient::check_pre_flag() {
  const bool result =
      _pre_flag_timer.elapsed_millis() > PRE_FLAG_TIMEOUT_MILLIS;
  _pre_flag_timer.reset();
  return result;
}