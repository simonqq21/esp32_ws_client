#include <Arduino.h>
#include <WiFi.h>
#include <esp_websocket_client.h>
#include "ArduinoJson.h"

JsonDocument doc;
char buff[100];
const String uri = "ws://192.168.4.1:7777/ws";
esp_websocket_client_handle_t client;
JsonObject payloadJSON;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        // log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        // if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
        //     log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
        //     log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
        //     log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        // }
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        } else {
            ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        }
        // If received data contains json structure it succeed to parse
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        // log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        // if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
        //     log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
        //     log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
        //     log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        // }
        break;
    }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Connecting to wifi");
  // attempt to connect to wifi 
  IPAddress localIP;
  IPAddress gateway;
  IPAddress subnet(255,255,255,0);
  IPAddress dns(8,8,8,8);
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin("ESP32_wifi_manager");
  Serial.print("connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected.");

  esp_websocket_client_config_t websocket_cfg = {};
  websocket_cfg.uri = uri.c_str();
  client = esp_websocket_client_init(&websocket_cfg);
  esp_websocket_client_start(client);
  esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*) client);
  delay(1000);
  Serial.printf("isConnected=%d\n", esp_websocket_client_is_connected(client));
  doc["cmd"] = "switch";
  doc["type"] = "relay_states";
  payloadJSON = doc["payload"].to<JsonObject>();
  payloadJSON["index"] = 2;
  payloadJSON["relay_state"] = true;
  serializeJson(doc, buff);
  Serial.print(buff);
  esp_websocket_client_send_text(client, buff, 100, portMAX_DELAY);
}

void loop() {
  payloadJSON["relay_state"] = !payloadJSON["relay_state"];
  serializeJson(doc, buff);
  Serial.print(buff);
  esp_websocket_client_send_text(client, buff, 100, portMAX_DELAY);
  delay(1000);
}
