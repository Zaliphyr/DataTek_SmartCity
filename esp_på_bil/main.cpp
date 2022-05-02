#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//* @brief WiFi SSID
const char* ssid = "Karl_sin";
//* @brief WiFi password
const char* password = "1234abcd";
//* @brief MQTT server IP
const char* mqtt_server = "192.168.172.26";

//* @brief WiFi client
WiFiClient espClient;
//* @brief PubSubClient for mqtt communticaton
PubSubClient client(espClient);

/**
 * @brief Function that runs when PubSubClient recieves message on subscribed topics
 * @param topic What topic are we getting messages on
 * @param message What is the message
 * @param length How long is the message
 */
void callback(char* topic, byte* message, unsigned int length);
//* @brief Function that connects to MQTT
void reconnect();
//* @brief Function that starts WiFi
void WiFiSetup();
//* @brief Function checks for messages from car
void check_comm();

void setup() {
  // Start serial
  Serial.begin(115200);
  // Start WiFi
  WiFiSetup();
  // Connect to MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Reconnect if lost connection, will also run the first time
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  check_comm();
}

void WiFiSetup(){
  // Start WiFi
  WiFi.begin(ssid, password);
  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
  }
}

void callback(char* topic, byte* message, unsigned int length){
  // Transforms message to string
  String messageTemp;
  for (int i = 0; i < length; i++){
    messageTemp += (char)message[i];
  }

  // Sends message to car
  if (String(topic) == "esp32/to_sens"){
    Serial.flush();
    Serial.print(messageTemp);
  }
}

void reconnect(){
  while (!client.connected()){
    if (client.connect("ESP8266Client")){
      client.subscribe("esp32/to_sens");
      // Send message to Node-RED to tell it that car is online
      client.publish("esp32/online", "car online");
      // Tell car that MQTT setup is done
      Serial.print("connected");
    } else {
      delay(5000);
    }
  }
}

void check_comm(){
  // Checks if message has arrived from car
  String tempS;
  while (Serial.available()){
    delay(1);
    char c = Serial.read();
    tempS += c;
  }

  // If message has arrivced, send it to Node-RED
  if (tempS.length() > 0){
    client.publish("esp32/from_sens", tempS.c_str());
  }
}


