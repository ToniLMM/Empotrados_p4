#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RXD2 33
#define TXD2 4

#define START_LAP 0
#define END_LAP 1
#define OBSTACLE_DETECTED 2
#define LINE_LOST 3
#define PING 4
#define INIT_LINE_SEARCH 5
#define STOP_LINE_SEARCH 6
#define LINE_FOUND 7
#define VISIBLE_LINE 8

const char* ssid = "OnePlus Toni";
const char* password = "tonilazaro";
const char* mqtt_server = "193.147.79.118";
const char* topic = "/SETR/2025/14/";
const int mqtt_port = 21883;
const char* mqttUser = "tonilmm";
const char* mqttPassword = "Mes392eemm";

const char* team_name = "f&f_1327";
const char* team_id = "14";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long last_start_time = 0;
unsigned long last_ping_time = 0;
const unsigned long ping_interval = 4000;
bool start_lap_sent = false;
bool lap_in_progress = false;
bool system_finished = false; 

String get_action(int action) {
  switch (action) {
    case START_LAP: return "START_LAP";
    case END_LAP: return "END_LAP";
    case OBSTACLE_DETECTED: return "OBSTACLE_DETECTED";
    case LINE_LOST: return "LINE_LOST";
    case PING: return "PING";
    case INIT_LINE_SEARCH: return "INIT_LINE_SEARCH";
    case STOP_LINE_SEARCH: return "STOP_LINE_SEARCH";
    case LINE_FOUND: return "LINE_FOUND";
    case VISIBLE_LINE: return "VISIBLE_LINE";
    default: return "ERROR";
  }
}

void send_mqtt_message(int action, unsigned long time = 0, int distance = -1, float value = -1.0) {
  StaticJsonDocument<256> doc;
  doc["team_name"] = team_name;
  doc["id"] = team_id;
  doc["action"] = get_action(action);

  if (action == END_LAP || action == PING) {
    doc["time"] = time;
  } 
  else if (action == OBSTACLE_DETECTED) {
    doc["distance"] = distance;
  } 
  else if (action == VISIBLE_LINE) {
    doc["value"] = value;
  }

  String jsonStr;
  serializeJson(doc, jsonStr);
  
  if (client.publish(topic, jsonStr.c_str())) {
    Serial.print("Message sent: ");
    Serial.println(jsonStr);
  } else {
    Serial.println("Error sending MQTT message");
  }
}

void init_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.print(".");
    delay(500);
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nError connecting to WiFi");
  }
}

void reconnect_mqtt() {
  while (!client.connected()) {    
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failure, rc=");
      Serial.print(client.state());
      Serial.println(" Trying again in 2s");
      delay(2000);
    }
  }
}

bool can_start_lap() {
  return (WiFi.status() == WL_CONNECTED && client.connected());
}

void handle_start_lap() {
  if (!start_lap_sent && can_start_lap() && !system_finished) {
    Serial2.println(1);
    
    unsigned long timeout = millis();
    bool received = false;
    
    while (millis() - timeout < 2000) {
      if (Serial2.available()) {
        String response = Serial2.readStringUntil('\n');
        response.trim();
        if (response == "0") {
          received = true;
          break;
        }
      }
      delay(10);
    }
    
    if (received) {
      send_mqtt_message(START_LAP);
      last_start_time = millis();
      last_ping_time = last_start_time;
      start_lap_sent = true;
      lap_in_progress = true;
      
      send_mqtt_message(PING, 0);
    } else {
      Serial.println("Error: Arduino did not respond to start the lap");
    }
  }
}

void read_serial2_send() {
  while (Serial2.available()) {
    String receivedString = Serial2.readStringUntil('\n');
    receivedString.trim();
    if (receivedString.length() == 0) return;

    int action = receivedString.toInt();

    String valueStr = "";
    if (action == OBSTACLE_DETECTED || action == END_LAP || action == VISIBLE_LINE) {
      unsigned long t_start = millis();
      while (!Serial2.available() && millis() - t_start < 500) {
        delay(5);
      }
      
      if (Serial2.available()) {
        valueStr = Serial2.readStringUntil('\n');
        valueStr.trim();
      }
    }

    switch (action) {
      case END_LAP: {
        if (lap_in_progress) {
          unsigned long elapsedTime = valueStr.toInt();
          send_mqtt_message(END_LAP, elapsedTime);
          
          lap_in_progress = false;
          start_lap_sent = false;
          system_finished = true;
        }
        break;
      }

      case OBSTACLE_DETECTED: { 
        int distVal = valueStr.toInt();
        if (distVal >= 5 && distVal <= 8) {
          send_mqtt_message(OBSTACLE_DETECTED, 0, distVal);
        }
        break;
      }
      
      case LINE_LOST:
        if (lap_in_progress) {
          send_mqtt_message(LINE_LOST);
        }
        break;

      case INIT_LINE_SEARCH:
        if (lap_in_progress) {
          send_mqtt_message(INIT_LINE_SEARCH);
        }
        break;

      case STOP_LINE_SEARCH:
        if (lap_in_progress) {
          send_mqtt_message(STOP_LINE_SEARCH);
        }
        break;

      case LINE_FOUND:
        if (lap_in_progress) {
          send_mqtt_message(LINE_FOUND);
        }
        break;

      case VISIBLE_LINE: {
        float val = valueStr.toFloat();
        send_mqtt_message(VISIBLE_LINE, 0, -1, val);
        break;
      }
      
      default:
        Serial.print("Unknown action: ");
        Serial.println(action);
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  delay(1000);

  init_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setBufferSize(512);
  
  delay(2000);
}

void loop() {
  if (system_finished) {
    delay(1000);
    return;
  }
  
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  read_serial2_send();

  if (lap_in_progress) {
    unsigned long currentTime = millis();
    if (currentTime - last_ping_time >= ping_interval) {
      unsigned long elapsed = currentTime - last_start_time;
      send_mqtt_message(PING, elapsed);
      last_ping_time = currentTime;
    }
  }
  
  if (!start_lap_sent && can_start_lap() && !system_finished) {
    handle_start_lap();
  }

  delay(10);
}