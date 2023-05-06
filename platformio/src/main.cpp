#include <Arduino.h>
#include <serial_packets.h>


void commandHandler(byte endpoint, const SerialPacketsData& data,
                    byte& response_status, SerialPacketsData& response_data) {
  Serial.println("Command Handler");
}

void messageHandler(byte endpoint, const SerialPacketsData& data) {
  Serial.println("Message Handler");
}

void eventHandler(SeriaPacketsEvent event) { Serial.println("Event Handler"); }

SerialPacketsClient SerialPackets(commandHandler, messageHandler, eventHandler);

void setup() {
  // We use serial2 for data communication and serial1 for debug messages.
  Serial2.begin(115200);
  Serial1.begin(115200);
  SerialPackets.begin(Serial2, Serial1);

  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}