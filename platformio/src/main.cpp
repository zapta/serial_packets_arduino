#include <Arduino.h>
#include <serial_packets.h>

#include "elapsed.h"

String str;

// Handler for incomming commands
void incomingCommandHandler(byte endpoint, const SerialPacketData& data,
                            byte& response_status,
                            SerialPacketData& response_data) {
  Serial.println("Command Handler");
}

// Handler for incomming messages
void incomingMessageHandler(byte endpoint, const SerialPacketData& data) {
  Serial.println("Message Handler");
}

// Handler for Serial Packets events.
void eventHandler(SeriaPacketsEvent event) { Serial.println("Event Handler"); }

// Serial Packets client. We associate it with a serial port in setup().
SerialPacketsClient packets(incomingCommandHandler, incomingMessageHandler,
                            eventHandler);

void setup() {
  // Debug stream.
  Serial.begin(115200);
  // Start the datalink
  Serial2.begin(115200);
  packets.begin(Serial2, Serial);
}

Elapsed command_timer;
SerialPacketData data(40);
uint32_t cmd_id;

void command_response_handler(uint32_t user_data, OutcomeCode outcome,
                              byte response_status,
                              const SerialPacketData& response_data) {
  Serial.println("Command outcome");
}

void loop() {
  // Service serial packets loop.
  packets.loop();

  // Periodically send a command.
  if (command_timer.elapsed_millis() > 1000) {
    command_timer.reset();

    // Send command
    data.clear();
    data.add_uint8(0x10);
    data.add_uint32(0x12345678);
    // data.dump("Command data", Serial);
    if (!packets.sendCommand(0x20, data, command_response_handler, cmd_id, 500)) {
      Serial.println("sendCommand() failed");
    }
  }
}
