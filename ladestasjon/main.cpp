// Include libs
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_I2CDevice.h>

// Screen setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi setup
const char* ssid = "Karl_sin";
const char* password = "1234abcd";
const char* mqtt_server = "192.168.172.26";

// MQTT setup
WiFiClient espClient;
PubSubClient client(espClient);

// Price list
int standard_prices[5] = {100, 50, 20, 10, 5}; 
int prices[5] = {100, 50, 20, 10, 5};
int chg_values[5] = {100, 50, 20, 10, 5};

// Variables
String status = "no car";
int ID = 0;
int battery = 0;
int money = 0;
int selection = 10;
int goal = 0;

// Timer
unsigned long last_time = 0;
unsigned long last_price_update = 0;

// Functions
void WiFiSetup();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();
void send_price();
void sendValue(String topic, String value);
void simpleText(String text);
void randomice_price();

void setup() {
  // Setup serial
  Serial.begin(9600);
  Serial2.begin(9600);

  // Connect screen
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Screen setup
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Connect to WiFi
  simpleText("Connecting to WiFi");
  WiFiSetup();

  // Connect to MQTT
  simpleText("Connecting to MQTT");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Check if connected to MQTT
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  if (millis() - last_price_update > 30000){
    last_time = millis();
    randomice_price();
    send_price();
  }

  // Check if car is in charging modus
  if (status == "charging"){
    // Charge 1% every 500ms
    if (millis() - last_time > 500){
      last_time = millis();

      battery++;
      sendValue("batt", String(battery));

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Charging");
      display.println("Charge: " + String(battery) + "%");
      display.println("Goal: " + String(goal) + "%");
      display.display();
      if (battery >= goal){
        status = "waiting";
        sendValue("status", status);
        sendValue("batt", String(battery));
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("ID: " + String(ID));
        display.println("Batt: " + String(battery));
        display.println("Money: " + String(money));
        display.display();
      }
    }
  }
}

// Function that handles WiFi connection
void WiFiSetup(){
  Serial.print("Establishing connection to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }

  Serial.println("connected");
  Serial.println(WiFi.localIP());
}

// Function that handles MQTT callback
void callback(char* topic, byte* message, unsigned int length){
  String messageTemp;

  // Convert message to string
  for (int i = 0; i < length; i++){
    messageTemp += (char)message[i];
  }

  // chg_start handles what to do when starting a charge
  // Also checks if user can afford selected charge
  if (String(topic) == "esp32/chg_start"){
    if (status == "waiting"){
      int selection = messageTemp.toInt();
      if (money - prices[selection] > 0){
        goal = battery + chg_values[selection];
        if (goal > 100){
          goal = 100;
        }
        money -= prices[selection];
        status = "charging";
        sendValue("status", status);
        sendValue("money", String(money));
      }
    }
  }

  // chg_arrive handles what to do when car arrives at charging station
  if (String(topic) == "esp32/chg_arrive"){
    if (status == "no car"){ 
      // Splits message into ID, money and battery
      int id = 0;
      int tmp_lst[3];
      String tmp;
      for (int i = 0; i < messageTemp.length(); i++){
      if (messageTemp[i] == ','){
        tmp_lst[id] = tmp.toInt();
        id++;
        tmp = "";
      } else {
        tmp += messageTemp[i];
      }
    }

      // Set variables
      status = "waiting";
      ID = tmp_lst[0];
      money = tmp_lst[1];
      battery = tmp_lst[2];

      // send values to node-red
      sendValue("status", status);
      sendValue("id", String(ID));
      sendValue("batt", String(battery));
      sendValue("money", String(money));
    
      Serial.println(tmp_lst[0]);
      Serial.println(tmp_lst[1]);
      Serial.println(tmp_lst[2]);

      // Display on screen
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("ID: " + String(ID));
      display.println("Batt: " + String(battery));
      display.println("Money: " + String(money));
      display.display();
    }
  }

  // chg_leave handles what to do done is selected in node-red
  // also resets all values
  if (String(topic) == "esp32/chg_leave"){
    if (status == "waiting"){

      status = "no car";
      ID = 0;
      battery = 0;
      money = 0;
      selection = 10;
      goal = 0;

      Serial.println("car leave");
      sendValue("status", status);
      simpleText("no car");
    }
  }
}

// Function that handles MQTT connection
void reconnect(){
  Serial.println("MQTT reconnecting");
  while (!client.connected()){
    if (client.connect("ESP8266Client")){
      Serial.println("MQTT connected");

      // Sends message to node-red that an esp is connected
      client.publish("esp32/online", "charger online");

      // subscribes to charger topics
      client.subscribe("esp32/chg_start");
      client.subscribe("esp32/chg_arrive");
      client.subscribe("esp32/chg_leave");

      send_price();
      simpleText("Connected to MQTT");
    } else {
      simpleText("MQTT failed, retrying");
      delay(5000);
    }
  }
}

// Function that sends value on topic for charger
void sendValue(String topic, String value){
  String tmp = topic;
  tmp += ",";
  tmp += value;
  client.publish("esp32/from_charger", tmp.c_str());
}

// function that sends price list
void send_price(){
  sendValue("p0", String(prices[0]));
  sendValue("p1", String(prices[1]));
  sendValue("p2", String(prices[2]));
  sendValue("p3", String(prices[3]));
  sendValue("p4", String(prices[4]));
}

// Function that displays one line of text on screen
void simpleText(String text){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(text);
  display.display();
}

void randomice_price(){
  for (int i = 0; i < 5; i++){
    int newVal = random(-10, 11);
    prices[i] = standard_prices[i] + standard_prices[i] * 0.01 * newVal;
  }
}
