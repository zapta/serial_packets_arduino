#include "packet_encoder.h"

#include "packet_consts.h"
#include "serial_packets.h"

bool PacketEncoder::byte_stuffing(const SerialPacketData& in,
                                  SerialPacketData& out,
                                  bool insert_pre_flag) {
  out.clear();
  const uint16_t capacity = out.capacity();
  uint16_t j = 0;

   // If requested, prepend a flag byte.
  if (insert_pre_flag) {
    if (j + 1 >= capacity) {
      return false;
    }
    out._buffer[j++] = PACKET_FLAG;
  }

  // Byte stuff the in bytes.
  for (uint16_t i = 0; i < in._size; i++) {
    const uint8_t b = in._buffer[i];
    if (b == PACKET_FLAG || b == PACKET_ESC) {
      if (j + 2 >= capacity) {
        return false;
      }
      out._buffer[j++] = PACKET_ESC;
      out._buffer[j++] = b ^ 0x20;
    } else {
      if (j + 1 >= capacity) {
        return false;
      }
      out._buffer[j++] = b;
    }
  }

  // Append post flag.
  if (j + 1 >= capacity) {
    return false;
  }
  out._buffer[j++] = PACKET_FLAG;

  // Update outptut data size.
  out._size = j;
  return true;
}

bool PacketEncoder::encode_command_packet(uint32_t cmd_id, uint8_t endpoint,
                                          const SerialPacketData& data,
                                          SerialPacketData& out,
                                          bool insert_pre_flag) {
  // Encode packet in _tmp_data.
  _tmp_data.clear();
  _tmp_data.add_uint8(TYPE_COMMAND);
  _tmp_data.add_uint32(cmd_id);
  _tmp_data.add_data(data);
  _tmp_data.add_uint16(_tmp_data.crc16());
  if (_tmp_data.write_error()) {
    return false;
  }

  // Byte stuffed into the outupt data.
  if (!byte_stuffing(_tmp_data, out,  insert_pre_flag)) {
    return false;
  }

  return true;
}


