#include <Arduino.h>
#include <serial_packets.h>

#include "io.h"
#include "packet_timer.h"

// Callback handler for incomming commands.
// Called within the call to packets.loop(). Should return immediatly.
void command_handler(byte endpoint, const PacketData& data,
                            byte& response_status, PacketData& response_data) {
  response_status = OK;
  response_data.write_uint32(0x12345678);
  Serial.println("Command Handler");
}

// Callback handler for incomming messages.
// Called within the call to packets.loop().  Should return immediatly.
void message_handler(byte endpoint, const PacketData& data) {
  Serial.printf("Message Handler, endpoint = %02x\n", endpoint);
  data.dump("Message data", Serial);
  const uint8_t v1 = data.read_uint8();
  const uint32_t v2 = data.read_uint32();
  if (data.all_read_ok()) {
        Serial.printf("Read ok: %02hx, %08x\n", v1, v2);
  } else {
        Serial.println("Read errors or extra data.");
  }

    // test_packet_data.add_uint8(0x10);
    //   test_packet_data.add_uint32(0x12345678);

}

// Callback handler for Serial Packets events.
// Called within the call to packets.loop(). Should return immediatly.
// void eventHandler(SeriaPacketsEvent event) { Serial.println("Event Handler"); }

// The serial Packets client. We associate it with a serial port in setup().
static SerialPacketsClient packets(command_handler,
                                   message_handler);

void setup() {
  io::setup();

  // A serial port for debug information.
  Serial.begin(115200);
  // A serial port for the packets data link.
  Serial2.begin(115200);
  // The packets datalink client.
  packets.begin(Serial2, Serial);
}

static PacketTimer test_command_timer;
static PacketData test_packet_data(40);
static uint32_t test_cmd_id = 0;

// Callback handler for incomming test command response.
// Called within the call to packets.loop(). Should return immediatly.
void response_handler(uint32_t cmd_id, byte response_status,
                                   const PacketData& response_data) {
  Serial.printf("Command outcome id=%08x, status=%hd\n", cmd_id,
                response_status);
}

void loop() {
  // Service serial packets loop.
  packets.loop();

  // Periodically send a test command.
  if (test_command_timer.elapsed_millis() > 1000) {
    test_command_timer.reset();
    io::LED.toggle();

    // Send command
    if (0) {
      test_packet_data.clear();
      test_packet_data.write_uint8(0x10);
      test_packet_data.write_uint32(millis());

      if (!packets.sendCommand(0x20, test_packet_data,
                               response_handler, test_cmd_id,
                               1000)) {
        Serial.println("sendCommand() failed");
      }
    }

    // Send message
    if (1) {
      test_packet_data.clear();
      test_packet_data.write_uint8(0x10);
      test_packet_data.write_uint32(millis());
      // data.dump("Command data", Serial);

      if (!packets.sendMessage(0x30, test_packet_data)) {
        Serial.println("sendMessage() failed");
      }
    }
  }
}
