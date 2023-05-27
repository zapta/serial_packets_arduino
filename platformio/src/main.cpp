#include <Arduino.h>

#include "serial_packets_client.h"

// A callback for all incoming commands. Configured once during initialization.
void command_handler(byte endpoint, const SerialPacketsData& data,
                     byte& response_status, SerialPacketsData& response_data) {
  // Typically dispatch to sub-handler by endpoint.
  Serial.printf("Command Handler called, endpoint=%02x, data=%hu\n", endpoint,
                data.size());
  // Command response to send back
  response_status = OK;
  response_data.write_uint8(0x12);
  response_data.write_uint16(0x3456);
}

// A callback for all incoming messages. Configured once during initialization.
void message_handler(byte endpoint, const SerialPacketsData& data) {
  // Typically dispatch to sub-handler by endpoint.
  Serial.printf("Message Handler called, endpoint=%02x, data=%hu\n", endpoint,
                data.size());
  // Parse the command data.
  const uint8_t v1 = data.read_uint8();
  const uint32_t v2 = data.read_uint32();
  Serial.printf("v1=%02x, v2=%08x, ok=%s\n", v1, v2,
                data.all_read_ok() ? "yes" : "no");
}

// A callback to handle responses of outgoing commands. Configured per
// command, when calling sendCommand().
void response_handler(uint32_t cmd_id, byte response_status,
                      const SerialPacketsData& response_data) {
  Serial.printf("Response Handler called, cmd_id=%08x, status=%hu, size=%hu\n",
                cmd_id, response_status, response_data.size());
}

// The serial Packets client.
static SerialPacketsClient packets(command_handler, message_handler);

void setup() {
  // A serial port for packet data communication.
  Serial2.begin(115200);

  // A serial port for debug log.
  Serial.begin(115200);

  // Start the packets client.
  packets.setLogLevel(SERIAL_PACKETS_LOG_INFO);
  packets.begin(Serial2, Serial);
}

static uint32_t last_send_time_millis = 0;
static uint32_t test_cmd_id = 0;
// These buffers can be large, depending on the configuration
// so we avoid allocating them on the stack.
static SerialPacketsData test_packet_data;


void loop() {
  // Service serial packets loop. Callbacks are called within
  // this call.
  packets.loop();

  // Periodically send a test command and a message.
  if (millis() - last_send_time_millis > 1000) {
    last_send_time_millis = millis();

    // Send a command
    test_packet_data.clear();
    test_packet_data.write_uint8(0x10);
    test_packet_data.write_uint32(millis());

    if (!packets.sendCommand(0x20, test_packet_data, response_handler,
                             test_cmd_id, 1000)) {
      Serial.println("sendCommand() failed");
    }

    // Send a message
    test_packet_data.clear();
    test_packet_data.write_uint8(0x10);
    test_packet_data.write_uint32(millis());
    if (!packets.sendMessage(0x30, test_packet_data)) {
      Serial.println("sendMessage() failed");
    }
  }
}
