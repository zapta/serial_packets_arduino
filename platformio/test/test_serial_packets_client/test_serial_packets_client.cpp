// Unit test of the packet encoder function.

#include <Arduino.h>
#include <unity.h>

#include <memory>
#include <vector>

#include "../common/serial_packets_test_utils.h"
#include "serial_packets.h"

struct Command {
  uint8_t endpoint;
  std::vector<uint8_t> data;
};

struct Response {
  uint32_t cmd_id;
  uint8_t status;
  std::vector<uint8_t> data;
};

struct Message {
  uint8_t endpoint;
  std::vector<uint8_t> data;
};

static std::vector<Command> command_list;
static std::vector<Response> response_list;
static std::vector<Message> message_list;
static std::vector<SeriaPacketsEvent> event_list;

struct FakeResponse {
  void clear() { set(0, {}); }
  void set(uint8_t new_status, const std::vector<uint8_t>& new_data) {
    status = new_status;
    data.clear();
    data.insert(data.end(), new_data.begin(), new_data.end());
  }
  uint8_t status;
  std::vector<uint8_t> data;
};

static FakeResponse fake_response;

void command_handler(byte endpoint, const PacketData& data,
                     byte& response_status, PacketData& response_data) {
  // Record the incoming command.
  Command item;
  item.endpoint = endpoint;
  item.data = copy_data(data);
  command_list.push_back(item);
  // Return a requested fake response.
  response_status = fake_response.status;
  populate_data(response_data, fake_response.data);
}

void response_handler(uint32_t cmd_id, byte response_status,
                      const PacketData& data) {
  Response item;
  item.cmd_id = cmd_id;
  item.status = response_status;
  item.data = copy_data(data);
  response_list.push_back(item);
}

void message_handler(byte endpoint, const PacketData& data) {
  Message item;
  item.endpoint = endpoint;
  item.data = copy_data(data);
  message_list.push_back(item);
}

void eventHandler(SeriaPacketsEvent event) { event_list.push_back(event); }

// static std::unique_ptr<PacketLogger> logger;
static std::unique_ptr<SerialPacketsClient> client;
static std::unique_ptr<SerialPacketsClientInspector> inspector;

void setUp(void) {
  inspector.reset();
  client.reset();
  client = std::make_unique<SerialPacketsClient>(command_handler,
                                                 message_handler, eventHandler);
  inspector = std::make_unique<SerialPacketsClientInspector>(*client);
  Serial2.flush();
  delay(100);
  while (Serial2.available()) {
    Serial2.read();
  }
  client->begin(Serial2, Serial);
  fake_response.clear();
  command_list.clear();
  response_list.clear();
  message_list.clear();
  event_list.clear();
}

void test_constructor() {}

void test_send_message_loop() {
  const std::vector<uint8_t> data = {0x11, 0x22, 0x33};
  PacketData packet_data(30);
  populate_data(packet_data, data);
  TEST_ASSERT_TRUE(client->sendMessage(0x20, packet_data));
  TEST_ASSERT_EQUAL(0, inspector->num_pending_commands());
  loop_client(*client, 200);
  TEST_ASSERT_EQUAL(0, command_list.size());
  TEST_ASSERT_EQUAL(0, response_list.size());
  TEST_ASSERT_EQUAL(1, message_list.size());
  TEST_ASSERT_EQUAL(0, event_list.size());
  const Message& message = message_list.at(0);
  TEST_ASSERT_EQUAL(0x20, message.endpoint);
  assert_vectors_equal(data, message.data);
}

void test_send_command_loop() {
  const std::vector<uint8_t> data = {0x11, 0x22, 0x33};
  PacketData packet_data(30);
  populate_data(packet_data, data);
  fake_response.status = 0x99;
  fake_response.data = {0xaa, 0xbb, 0xcc};
  uint32_t cmd_id = 0;
  TEST_ASSERT_TRUE(
      client->sendCommand(0x20, packet_data, response_handler, cmd_id, 1000));
  TEST_ASSERT_NOT_EQUAL_HEX32(0, cmd_id);
  TEST_ASSERT_EQUAL(1, inspector->num_pending_commands());
  loop_client(*client, 200);
  TEST_ASSERT_EQUAL(1, command_list.size());
  TEST_ASSERT_EQUAL(1, response_list.size());
  TEST_ASSERT_EQUAL(0, message_list.size());
  TEST_ASSERT_EQUAL(0, event_list.size());
  TEST_ASSERT_EQUAL(0, inspector->num_pending_commands());
  const Command& command = command_list.at(0);
  TEST_ASSERT_EQUAL_HEX8(0x20, command.endpoint);
  assert_vectors_equal(data, command.data);
  const Response& response = response_list.at(0);
  TEST_ASSERT_EQUAL_HEX32(cmd_id, response.cmd_id);
  TEST_ASSERT_EQUAL_HEX8(0x99, response.status);
  assert_vectors_equal(response.data, {0xaa, 0xbb, 0xcc});
}

void setup() {
  common_setup_init();

  // IMPORTANT: Wire this serial port in a loop for this test to work.
  Serial2.begin(115200);

  UNITY_BEGIN();

  RUN_TEST(test_constructor);
  RUN_TEST(test_send_message_loop);
  RUN_TEST(test_send_command_loop);

  UNITY_END();
}

void loop() { common_loop_body(); }
