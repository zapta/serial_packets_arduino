#include "serial_packets.h"

#include "packet_consts.h"

// TODO: All methods should verify that data stream is available (and that
// client is in active state)

void SerialPacketsClient::begin(Stream& data_stream, Stream& log_stream) {
  if (_data_stream) {
    log_stream.printf(
        "ERROR: Already called SerialPacktsBegin.bein(), ignoring.\n");
    return;
  }
  _data_stream = &data_stream;
  _logger.set_stream(&log_stream);
  force_next_pre_flag();
  // TODO: Clear variables such as context table.
}

void SerialPacketsClient::begin(Stream& data_stream) {
  if (_data_stream) {
    // Already started. Ignore silently.
    return;
  }
  _data_stream = &data_stream;
  _logger.set_stream(nullptr);
  force_next_pre_flag();
  // TODO: Clear variables such as context table.
}

static uint8_t rx_transfer_buffer[100];

void SerialPacketsClient::loop() {
  // Decode incoming data.
  uint16_t n = _data_stream->available();
  while (n > 0) {
    const uint16_t count = min((uint16_t)sizeof(rx_transfer_buffer), n);
    const uint16_t bytes_read =
        _data_stream->readBytes(rx_transfer_buffer, count);
    n -= bytes_read;
    // TODO: Assert that bytes_read == count, or at least > 0.
    for (int i = 0; i < bytes_read; i++) {
      const bool has_new_packet =
          _packet_decoder.decode_next_byte(rx_transfer_buffer[i]);
      // if (status != PacketDecoder::IN_PACKET) {
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
    const DecodedCommandMetadata& metadata, const SerialPacketData& data) {
  if (!_command_handler) {
    _logger.error("No handler for incoming command");
    return;
  }

  _tmp_data1.clear();
  byte status = OK;
  _command_handler(metadata.endpoint, data, status, _tmp_data1);

  // Send response
  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_response_packet(
          metadata.cmd_id, status, _tmp_data1, insert_pre_flag, &_tmp_data2)) {
    _logger.error("Failed to encode response packet. Dropping.");
    return;
  }

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_data2.size();
  if (_data_stream->availableForWrite() < size) {
    _logger.error(
        "Insufficient TX buffer space for sending a response packet "
        "(%hu).",
        size);
    return;
  }

  // NOTE: To increase the TX buffer size, use the build flag
  // -DSERIAL_TX_BUFFER_SIZE=4096.
  const uint16_t written = _data_stream->write(_tmp_data2._buffer, size);

  if (written < size) {
    force_next_pre_flag();
    _logger.error("Only %hu of %hu of response packet bytes were written",
                written, size);
  }
}

// Process an incoming response packet.
void SerialPacketsClient::process_decoded_response_packet(
    const DecodedResponseMetadata& metadata, const SerialPacketData& data) {
  if (!metadata.cmd_id) {
    _logger.error("Incoming response packet has cmd_id = 0.");
    return;
  }
  CommandContext* context = find_context_with_cmd_id(metadata.cmd_id);
  if (!context) {
    _logger.error(
        "Incoming response packet has no pending command. May timeout.");
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
    const DecodedMessageMetadata& metadata, const SerialPacketData& data) {
  // if (!metadata.cmd_id) {
  //   _logger.log("ERROR: incoming response packet has cmd_id = 0.");
  //   return;
  // }
  // CommandContext* context = find_context_with_cmd_id(metadata.cmd_id);
  // if (!context) {
  //   _logger.log(
  //       "ERROR: incoming response packet has no pending command. May
  //       timeout.");
  //   return;
  // }
  if (!_message_handler) {
    _logger.error("No message handler to dispatch an incoming message.");
    return;
  }

  _message_handler(metadata.endpoint, data);
  // context->response_handler(metadata.cmd_id, metadata.status, data);
  // // Setting the cmd_id to zero frees the context.
  // context->clear();
  // return;
}

bool SerialPacketsClient::sendCommand(
    byte endpoint, const SerialPacketData& data,
    SerialPacketsCommandResponseHandler response_handler, uint32_t& cmd_id,
    uint16_t timeout_millis = DEFAULT_CMD_TIMEOUT_MILLIS) {
  // Prepare for failure.
  cmd_id = 0;

  if (!response_handler) {
    _logger.error("Trying to send a command without a response handler.");
    return false;
  }

  // Verify timeout.
  if (timeout_millis < MIN_CMD_TIMEOUT_MILLIS ||
      timeout_millis > MAX_CMD_TIMEOUT_MILLIS) {
    _logger.error("Invalid command timeout %hu, should be in [%d, %d]",
                timeout_millis, MIN_CMD_TIMEOUT_MILLIS, MAX_CMD_TIMEOUT_MILLIS);
    return false;
  }

  // Find a free command context. At this point we still leave it
  // with cmd_id = 0. in case of an early return due to an error.
  CommandContext* cmd_context = find_context_with_cmd_id(0);
  if (!cmd_context) {
    _logger.error("Can't send a command, too many commands in progress (%d)",
                MAX_CMD_CONTEXTS);
    return false;
  }

  const uint32_t new_cmd_id = assign_cmd_id();

  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_command_packet(new_cmd_id, endpoint, data,
                                             insert_pre_flag, &_tmp_data1)) {
    _logger.error("Failed to encode command packet");
    return false;
  }

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_data1.size();
  const int available = _data_stream->availableForWrite();
  _logger.verbose("Available: %d, size: %hu", available, size);
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
  const uint16_t written = _data_stream->write(_tmp_data1._buffer, size);
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
                                      const SerialPacketData& data) {
  // Prepare for failure.
  // cmd_id = 0;

  // if (!response_handler) {
  //   return false;
  // }

  // Verify timeout.
  // if (timeout_millis < MIN_CMD_TIMEOUT_MILLIS ||
  //     timeout_millis > MAX_CMD_TIMEOUT_MILLIS) {
  //   return false;
  // }

  // Find a free command context. At this point we still leave it
  // with cmd_id = 0. in case of an early return due to an error.
  // CommandContext* cmd_context = find_context_with_cmd_id(0);
  // if (!cmd_context) {
  //   // Too many in progress outgoing commands.
  //   return false;
  // }

  // const uint32_t new_cmd_id = assign_cmd_id();

  // Determine if to insert a packet flag.
  const bool insert_pre_flag = check_pre_flag();

  // Encode the packet in wire format.
  if (!_packet_encoder.encode_message_packet(endpoint, data, insert_pre_flag,
                                            & _tmp_data1)) {
    return false;
  }

  // Push to TX buffer. It's all or nothing, no partial push.
  const uint16_t size = _tmp_data1.size();
  if (_data_stream->availableForWrite() < size) {
    return false;
  }

  // NOTE: We assume that this will not be blocking since we verified
  // the number of avilable bytes in the buffer. We force large buffer
  // using a build flag such as -DSERIAL_TX_BUFFER_SIZE=4096.
  const uint16_t written = _data_stream->write(_tmp_data1._buffer, size);

  if (written < size) {
    _logger.error("Only %hu of %hu of a message packet bytes were written",
                written, size);
    force_next_pre_flag();
  }

  // Set up the cmd context. Setting a non zero cmd_id allocates it.
  // cmd_context->cmd_id = new_cmd_id;
  // Wrap around is ok.
  // cmd_context->expiration_time_millis = millis() + timeout_millis;
  // cmd_context->response_handler = response_handler;

  // cmd_id = new_cmd_id;
  return true;
}
