// Unit test of the packet encoder function.

#include <Arduino.h>
#include <unity.h>

#include <memory>
#include <vector>

#include "../common/serial_packets_test_utils.h"
#include "serial_packets_client.h"

// The serial port we use for debug messages.
#define DEBUG_SERIAL (Serial)

// The serial port we use for testing packet communication. RX and TX should
// be connected in a loop for the tests to pass.
#define DATA_SERIAL (Serial2)

struct Command {
  uint32_t time_millis;
  uint8_t endpoint;
  std::vector<uint8_t> data;
};

struct Response {
  uint32_t time_millis;
  uint32_t cmd_id;
  uint8_t status;
  std::vector<uint8_t> data;
};

struct Message {
  uint32_t time_millis;
  uint8_t endpoint;
  std::vector<uint8_t> data;
};

static std::vector<Command> command_list;
static std::vector<Response> response_list;
static std::vector<Message> message_list;
// static std::vector<SeriaPacketsEvent> event_list;

// Contains a fake response that the test setup for the command
// handler to return.
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

void command_handler(byte endpoint, const SerialPacketsData& data,
                     byte& response_status, SerialPacketsData& response_data) {
  // Record the incoming command.
  Command item;
  item.time_millis = millis();
  item.endpoint = endpoint;
  item.data = copy_data(data);
  command_list.push_back(item);
  // Return a requested fake response.
  response_status = fake_response.status;
  populate_data(response_data, fake_response.data);
}

void response_handler(uint32_t cmd_id, byte response_status,
                      const SerialPacketsData& data) {
  Response item;
  item.time_millis = millis();
  item.cmd_id = cmd_id;
  item.status = response_status;
  item.data = copy_data(data);
  response_list.push_back(item);
}

void message_handler(byte endpoint, const SerialPacketsData& data) {
  Message item;
  item.time_millis = millis();
  item.endpoint = endpoint;
  item.data = copy_data(data);
  message_list.push_back(item);
}

static std::unique_ptr<SerialPacketsClient> client;
static std::unique_ptr<SerialPacketsClientInspector> inspector;

// This buffer can be large so we avoid allocating it on the stack.
static SerialPacketsData packet_data;

void setUp(void) {
  packet_data.clear();
  inspector.reset();
  client.reset();
  client =
      std::make_unique<SerialPacketsClient>(command_handler, message_handler);
  inspector = std::make_unique<SerialPacketsClientInspector>(*client);
  DATA_SERIAL.flush();
  delay(100);
  while (DATA_SERIAL.available()) {
    DATA_SERIAL.read();
  }
  client->begin(DATA_SERIAL, DEBUG_SERIAL);
  fake_response.clear();
  command_list.clear();
  response_list.clear();
  message_list.clear();
}

// Simple test that the data link is looped for the tests.
void test_simple_serial_loop() {
  // Send a string.
  TEST_ASSERT_EQUAL(0, DATA_SERIAL.available());
  const char str[] = "xyz";
  const int len = strlen(str);
  DATA_SERIAL.write(str, len);

  // Verify it was looped back.
  delay(100);
  TEST_ASSERT_GREATER_OR_EQUAL(len, DATA_SERIAL.available());
  for (int i = 0; i < len; i++) {
    const uint8_t b = DATA_SERIAL.read();
    TEST_ASSERT_EQUAL_HEX8(str[i], b);
  }
}

void test_constructor() {}

void test_send_message_loop() {
  const std::vector<uint8_t> data = {0x11, 0x22, 0x33};
  // SerialPacketsData packet_data;
  populate_data(packet_data, data);
  TEST_ASSERT_TRUE(client->sendMessage(0x20, packet_data));
  TEST_ASSERT_EQUAL(0, client->num_pending_commands());
  loop_client(*client, 200);
  TEST_ASSERT_EQUAL(0, command_list.size());
  TEST_ASSERT_EQUAL(0, response_list.size());
  TEST_ASSERT_EQUAL(1, message_list.size());
  const Message& message = message_list.at(0);
  TEST_ASSERT_EQUAL(0x20, message.endpoint);
  assert_vectors_equal(data, message.data);
}

void test_send_command_loop() {
  const std::vector<uint8_t> data = {0x11, 0x22, 0x33};
  // SerialPacketsData packet_data;
  populate_data(packet_data, data);
  fake_response.status = 0x99;
  fake_response.data = {0xaa, 0xbb, 0xcc};
  uint32_t cmd_id = 0;
  TEST_ASSERT_TRUE(
      client->sendCommand(0x20, packet_data, response_handler, cmd_id, 1000));
  TEST_ASSERT_NOT_EQUAL_HEX32(0, cmd_id);
  TEST_ASSERT_EQUAL(1, client->num_pending_commands());
  loop_client(*client, 200);
  TEST_ASSERT_EQUAL(1, command_list.size());
  TEST_ASSERT_EQUAL(1, response_list.size());
  TEST_ASSERT_EQUAL(0, message_list.size());
  TEST_ASSERT_EQUAL(0, client->num_pending_commands());
  const Command& command = command_list.at(0);
  TEST_ASSERT_EQUAL_HEX8(0x20, command.endpoint);
  assert_vectors_equal(data, command.data);
  const Response& response = response_list.at(0);
  TEST_ASSERT_EQUAL_HEX32(cmd_id, response.cmd_id);
  TEST_ASSERT_EQUAL_HEX8(0x99, response.status);
  assert_vectors_equal(response.data, {0xaa, 0xbb, 0xcc});
}

// Supresses RX to simulate timeout and verifies that the command
// was canceled and response handler was called with TIMEOUT status.
void test_command_timeout() {
  const std::vector<uint8_t> data = {0x11, 0x22, 0x33};
  // SerialPacketsData packet_data;
  populate_data(packet_data, data);
  // Supress RX in the client to simulate a command timeout.
  inspector->ignore_rx_for_testing(true);
  const uint32_t start_time_millis = millis();
  uint32_t cmd_id = 0;
  TEST_ASSERT_TRUE(
      client->sendCommand(0x20, packet_data, response_handler, cmd_id, 200));
  TEST_ASSERT_NOT_EQUAL_HEX32(0, cmd_id);
  TEST_ASSERT_EQUAL(1, client->num_pending_commands());
  loop_client(*client, 500);
  // The client should call the response handler with TIMEOUT status.
  TEST_ASSERT_EQUAL(0, command_list.size());
  TEST_ASSERT_EQUAL(1, response_list.size());
  TEST_ASSERT_EQUAL(0, message_list.size());
  TEST_ASSERT_EQUAL(0, client->num_pending_commands());
  const Response& response = response_list.at(0);
  TEST_ASSERT_EQUAL_HEX32(cmd_id, response.cmd_id);
  TEST_ASSERT_EQUAL_HEX8(TIMEOUT, response.status);
  TEST_ASSERT_TRUE(response.data.empty());
  // Verify time to cancalation. The cleanup is done every 5 ms.
  const uint32_t elapsed_millis = response.time_millis - start_time_millis;
  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(200, elapsed_millis);
  TEST_ASSERT_LESS_OR_EQUAL_UINT32(200, elapsed_millis);
}

void setup() {
  common_setup_init();

  // IMPORTANT: Wire this serial port in a loop for this test to work.
  DATA_SERIAL.begin(115200);

  UNITY_BEGIN();

  RUN_TEST(test_simple_serial_loop);
  RUN_TEST(test_constructor);
  RUN_TEST(test_send_message_loop);
  RUN_TEST(test_send_command_loop);
  RUN_TEST(test_command_timeout);

  UNITY_END();
}

void loop() { common_loop_body(); }
