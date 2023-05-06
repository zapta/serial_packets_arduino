#include <Arduino.h>
#include <serial_packets.h>


String str;

// Handler for incomming commands
void commandHandler(byte endpoint, const SerialPacketsData& data,
                    byte& response_status, SerialPacketsData& response_data) {
  Serial.println("Command Handler");
}

// Handler for incomming messages
void messageHandler(byte endpoint, const SerialPacketsData& data) {
  Serial.println("Message Handler");
}

// Handler for Serial Packets events.
void eventHandler(SeriaPacketsEvent event) { Serial.println("Event Handler"); }

// Serial Packets client. We associate it with a serial port in setup().
SerialPacketsClient SerialPackets(commandHandler, messageHandler, eventHandler);


SerialPacketsData data(100);

void setup() {
  // We use serial2 for data communication and serial1 for debug messages.
  Serial2.begin(115200);
  Serial1.begin(115200);
  SerialPackets.begin(Serial2, Serial1);
}

void loop() {
  // put your main code here, to run repeatedly:
}