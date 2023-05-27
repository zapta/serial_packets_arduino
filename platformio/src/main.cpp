#include <Arduino.h>

#include "serial_packets_client.h"

// Callback for incoming commands.
void command_handler(byte endpoint, const SerialPacketsData& data,
                     byte& response_status, SerialPacketsData& response_data) {
  // Put here code to dispatch the incoming command by its endpoint.
  Serial.printf("Command Handler called, endpoint=%02x, data=%hu\n", endpoint,
                data.size());
  response_status = OK;
  response_data.write_uint8(0x12);
  response_data.write_uint16(0x3456);
}

// Callback for command response.
void response_handler(uint32_t cmd_id, byte response_status,
                      const SerialPacketsData& response_data) {
  Serial.printf("Response Handler called, cmd_id=%08x, status=%hu, size=%hu\n",
                cmd_id, response_status, response_data.size());
}

// Callback for incoming messages.
void message_handler(byte endpoint, const SerialPacketsData& data) {
  Serial.printf("Message Handler called, endpoint=%02x, data=%hu\n", endpoint,
                data.size());
  const uint8_t v1 = data.read_uint8();
  const uint32_t v2 = data.read_uint32();
  Serial.printf("v1=%02x, v2=%08x, ok=%s\n", v1, v2,
                data.all_read_ok() ? "yes" : "no");
}

// The serial Packets client.
static SerialPacketsClient packets(command_handler, message_handler);

void setup() {
  // io::setup();

  // We use two serial ports, one for packets communication and one for
  // debugging.
  Serial2.begin(115200);
  Serial.begin(115200);
  packets.begin(Serial2, Serial);
}

static uint32_t last_send_time_millis = 0;
static SerialPacketsData test_packet_data;
static uint32_t test_cmd_id = 0;

void loop() {
  // Service serial packets loop.
  packets.loop();

  // Periodically send a test command.
  if (millis() - last_send_time_millis > 1000) {
    last_send_time_millis = millis();

    // Send command
    test_packet_data.clear();
    test_packet_data.write_uint8(0x10);
    test_packet_data.write_uint32(millis());

    if (!packets.sendCommand(0x20, test_packet_data, response_handler,
                             test_cmd_id, 1000)) {
      Serial.println("sendCommand() failed");
    }

    // Send message
    test_packet_data.clear();
    test_packet_data.write_uint8(0x10);
    test_packet_data.write_uint32(millis());
    if (!packets.sendMessage(0x30, test_packet_data)) {
      Serial.println("sendMessage() failed");
    }
  }
}
