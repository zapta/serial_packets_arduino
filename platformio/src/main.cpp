#include <Arduino.h>
#include <serial_packets.h>

#include "elapsed.h"

String str;

// Callback handler for incomming commands.
// Called within the call to packets.loop(). Should return immediatly.
void incomingCommandHandler(byte endpoint, const SerialPacketData& data,
                            byte& response_status,
                            SerialPacketData& response_data) {
  Serial.println("Command Handler");
}

// Callback handler for incomming messages.
// Called within the call to packets.loop().  Should return immediatly.
void incomingMessageHandler(byte endpoint, const SerialPacketData& data) {
  Serial.println("Message Handler");
}

// Callback handler for Serial Packets events.
// Called within the call to packets.loop(). Should return immediatly.
void eventHandler(SeriaPacketsEvent event) { Serial.println("Event Handler"); }

// The serial Packets client. We associate it with a serial port in setup().
static SerialPacketsClient packets(incomingCommandHandler, incomingMessageHandler,
                            eventHandler);

void setup() {
  // A serial port for debug information.
  Serial.begin(115200);
  // A serial port for the packets data link.
  Serial2.begin(115200);
  // The packets datalink client.
  packets.begin(Serial2, Serial);
}

static Elapsed test_command_timer;
static SerialPacketData test_packet_data(40);
static uint32_t test_cmd_id = 0;

// Callback handler for incomming test command response.
// Called within the call to packets.loop(). Should return immediatly.
void test_command_response_handler(uint32_t user_data, 
                              byte response_status,
                              const SerialPacketData& response_data) {
  Serial.println("Command outcome");
}

void loop() {
  // Service serial packets loop.
  packets.loop();

  // Periodically send a test command.
  if (test_command_timer.elapsed_millis() > 1000) {
    test_command_timer.reset();

    // Send command
    test_packet_data.clear();
    test_packet_data.add_uint8(0x10);
    test_packet_data.add_uint32(0x12345678);
    // data.dump("Command data", Serial);
    if (!packets.sendCommand(0x20, test_packet_data, test_command_response_handler, test_cmd_id, 500)) {
      Serial.println("sendCommand() failed");
    }
  }
}
