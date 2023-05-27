#include "serial_packets_client.h"

// TODO: All methods should verify that begin() was called and that the
// client is active.

using serial_packets_consts::TYPE_COMMAND;
using serial_packets_consts::TYPE_MESSAGE;
using serial_packets_consts::TYPE_RESPONSE;

void SerialPacketsClient::begin(Stream& data_stream, Stream& log_stream) {
  if (begun()) {
    _logger.error("ERROR: Serial packets begin() already called, ignoring.\n");
    return;
  }
  _data_stream = &data_stream;
  _logger.set_stream(&log_stream);
  force_next_pre_flag();
}

void SerialPacketsClient::begin(Stream& data_stream) {
  if (begun()) {
    _logger.error("ERROR: Serial packets begin() already called, ignoring.\n");
    return;
  }
  _data_stream = &data_stream;
  _logger.set_stream(nullptr);
  force_next_pre_flag();
}

void SerialPacketsClient::loop() {
  if (!begun()) {
    return;
  }
  loop_rx();
  loop_cleanup();
}

// Process cleanup of timeout commands.
void SerialPacketsClient::loop_cleanup() {
  // A shared, read only empty data.
  // static const SerialPacketsData empty_data(0);

  // Perform a cleanup cycle once every 5 ms.
  if (_cleanup_timer.elapsed_millis() < 2) {
    return;
  }
  _cleanup_timer.reset();

  const uint32_t millis_now = millis();
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    CommandContext* p = &_command_contexts[i];
    // Skip free entries.
    if (!p->cmd_id) {
      continue;
    }
    // Skip if not expired. The computation here takes care of possible
    // millies() overflow.
    int32_t millis_left =
        static_cast<int32_t>(p->expiration_time_millis - millis_now);
    if (millis_left > 0) {
      continue;
    }
    if (_logger.is_verbose()) {
      _logger.verbose("Cleaning command %08x, exp=%08x, now=%08x, left=%d",
                      p->cmd_id, p->expiration_time_millis, millis_now,
                      millis_left);
    }
    _logger.error("Command %08u timeout.", p->cmd_id);
    if (!p->response_handler) {
      _logger.error("No response handler command  %08u.", p->cmd_id);
    } else {
      _tmp_data.clear();
      p->response_handler(p->cmd_id, TIMEOUT, _tmp_data);
    }
    p->clear();
  }
}

// Decode and process incoming packets.
void SerialPacketsClient::loop_rx() {
  // We pull and process to this number of chars per loop.
  static uint8_t rx_transfer_buffer[100];

  // Decode incoming data.
  uint16_t n = _data_stream->available();
  while (n > 0) {
    const uint16_t count = min((uint16_t)sizeof(rx_transfer_buffer), n);
    const uint16_t bytes_read =
        _data_stream->readBytes(rx_transfer_buffer, count);
    n -= bytes_read;
    // TODO: Assert that bytes_read == count, or at least > 0.

    // A special hook for testing. This allows to simulate command
    // timeout.
    if (_ignore_rx_for_testing) {
      return;
    }

    for (int i = 0; i < bytes_read; i++) {
      const uint8_t b = rx_transfer_buffer[i];
      // _logger.verbose("%02x", b);
      const bool has_new_packet = _packet_decoder.decode_next_byte(b);
      // if (status != SerialPacketsDecoder::IN_PACKET) {
      // _logger.log("%02x -> %d %hu", rx_transfer_buffer[i], status,
      // _packet_decoder.len());
      // }
      if (has_new_packet) {
        const PacketType packet_type =
            _packet_decoder.packet_metadata().packet_type;
        switch (packet_type) {
          case TYPE_COMMAND:
            process_decoded_command_packet(
                _packet_decoder.packet_metadata().command,
                _packet_decoder.packet_data());
            break;
          case TYPE_RESPONSE:
            process_decoded_response_packet(
                _packet_decoder.packet_metadata().response,
                _packet_decoder.packet_data());
            break;
          case TYPE_MESSAGE:
            process_decoded_message_packet(
                _packet_decoder.packet_metadata().message,
                _packet_decoder.packet_data());
            break;
          default:
            _logger.error("Unknown incoming packet type: %02hhx", packet_type);
        }
      }
    }
  }
}

// Process an incoming command packet.
void SerialPacketsClient::process_decoded_command_packet(
    const DecodedCommandMetadata& metadata, const SerialPacketsData& data) {
  if (!_optional_command_handler) {
    _logger.error("No handler for incoming command");
    return;
  }

  _tmp_data.clear();
  byte status = OK;
  _optional_command_handler(metadata.endpoint, data, status, _tmp_data);

  // Send response
  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_response_packet(metadata.cmd_id, status,
                                              _tmp_data, insert_pre_flag,
                                              &_tmp_stuffed_packet)) {
    _logger.error("Failed to encode response packet. Dropping.");
    return;
  }

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_stuffed_packet.size();
  if (_data_stream->availableForWrite() < size) {
    _logger.error(
        "Insufficient TX buffer space for sending a response packet "
        "(%hu).",
        size);
    return;
  }

  // NOTE: To increase the TX buffer size, use the build flag
  // -DSERIAL_TX_BUFFER_SIZE=4096.
  const uint16_t written =
      _data_stream->write(_tmp_stuffed_packet._buffer, size);

  if (written < size) {
    force_next_pre_flag();
    _logger.error("Only %hu of %hu of response packet bytes were written",
                  written, size);
  }
}

// Process an incoming response packet.
void SerialPacketsClient::process_decoded_response_packet(
    const DecodedResponseMetadata& metadata, const SerialPacketsData& data) {
  if (!metadata.cmd_id) {
    _logger.error("Incoming response packet has cmd_id = 0.");
    return;
  }
  CommandContext* context = find_context_with_cmd_id(metadata.cmd_id);
  if (!context) {
    _logger.error(
        "Incoming response packet has no pending command %08x. May timeout.",
        metadata.cmd_id);
    return;
  }
  if (!context->response_handler) {
    _logger.error("Pending command has a null response handler.");
    return;
  }
  context->response_handler(metadata.cmd_id, metadata.status, data);
  // Setting the cmd_id to zero frees the context.
  context->clear();
  return;
}

// Process an incoming message packet.
void SerialPacketsClient::process_decoded_message_packet(
    const DecodedMessageMetadata& metadata, const SerialPacketsData& data) {
  if (!_optional_message_handler) {
    _logger.error("No message handler to dispatch an incoming message.");
    return;
  }

  _optional_message_handler(metadata.endpoint, data);
}

bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketsData& data,
    SerialPacketsCommandResponseHandler response_handler, uint32_t& cmd_id,
    uint16_t timeout_millis = DEFAULT_CMD_TIMEOUT_MILLIS) {
  // Prepare for failure.
  cmd_id = 0;

  if (!begun()) {
    return false;
  }

  if (!response_handler) {
    _logger.error("Trying to send a command without a response handler.");
    return false;
  }

  // Verify timeout.
  if (timeout_millis > MAX_CMD_TIMEOUT_MILLIS) {
    _logger.error("Invalid command timeout %hu ms, should be at most %d ms",
                  timeout_millis, MAX_CMD_TIMEOUT_MILLIS);
    return false;
  }

  // Find a free command context.
  CommandContext* cmd_context = find_context_with_cmd_id(0);

  if (!cmd_context) {
    _logger.error("Can't send a command, too many commands in progress (%d)",
                  MAX_PENDING_COMMANDS);
    return false;
  }

  const uint32_t new_cmd_id = assign_cmd_id();

  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // io::TEST2.on();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_command_packet(
          new_cmd_id, endpoint, data, insert_pre_flag, &_tmp_stuffed_packet)) {
    _logger.error("Failed to encode command packet");
    return false;
  }
  // io::TEST2.off();

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_stuffed_packet.size();
  const int available = _data_stream->availableForWrite();
  _logger.verbose("Available to send: %d, command packet size: %hu", available,
                  size);
  if (available < size) {
    _logger.error(
        "Can't send a command packet of size %hu, TX buffer has only %d "
        "bytes free",
        size, available);
    return false;
  }

  // NOTE: We assume that this will not be blocking since we verified
  // the number of avilable bytes in the buffer. We force large buffer
  // using a build flag such as -DSERIAL_TX_BUFFER_SIZE=4096.
  // io::TEST2.on();
  const uint16_t written =
      _data_stream->write(_tmp_stuffed_packet._buffer, size);
  // io::TEST2.off();
  _logger.verbose("Written %hu of %hu command packet bytes", written, size);

  if (written < size) {
    _logger.error("Only %hu of %hu of a command packet bytes were written",
                  written, size);
    force_next_pre_flag();
    return false;
  }

  // Set up the cmd context. Setting a non zero cmd_id allocates it.
  cmd_context->cmd_id = new_cmd_id;
  // Wrap around is ok.
  cmd_context->expiration_time_millis = millis() + timeout_millis;
  cmd_context->response_handler = response_handler;

  cmd_id = new_cmd_id;
  _logger.verbose("Command packet written ok, cmd_id = %08x", cmd_id);
  return true;
}

bool SerialPacketsClient::sendMessage(byte endpoint,
                                      const SerialPacketsData& data) {
  if (!begun()) {
    return false;
  }

  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_message_packet(endpoint, data, insert_pre_flag,
                                             &_tmp_stuffed_packet)) {
    _logger.error("Failed to encode message packet, data_size=%hu",
                  data.size());
    return false;
  }

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_stuffed_packet.size();
  const int vailable_to_write = _data_stream->availableForWrite();
  if (vailable_to_write < size) {
    _logger.error(
        "Failed to send a message, packet_size: %hu, available to write: %d",
        size, vailable_to_write);
    return false;
  }
  _logger.verbose("Sending message packet, size: %hu, available to write: %d",
                  size, vailable_to_write);

  // NOTE: We assume that this will not be blocking since we verified
  // the number of avilable bytes in the buffer. We force large buffer
  // using a build flag such as -DSERIAL_TX_BUFFER_SIZE=4096.
  const uint16_t written =
      _data_stream->write(_tmp_stuffed_packet._buffer, size);

  _logger.verbose("Written: %hu of %hu message packet bytes", written, size);

  if (written < size) {
    _logger.error("Only %hu of %hu of a message packet bytes were written",
                  written, size);
    force_next_pre_flag();
  }

  // cmd_id = new_cmd_id;
  return true;
}
