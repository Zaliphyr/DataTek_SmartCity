#include "common.h"
#include "cc.h"
#include "kom_sys.h"

// Kommunication system
kom_sys ks;
//  Car controller
cc cc;

// Variable for timed events
unsigned long last_time = 0;

void setup() {
  // Start serial
  Serial.begin(115200);

  // Add ks.send_value as a function to car controller
  cc.add_callback(ks.send_value);

  // init com_sys and car controller
  ks.init();
  cc.init();
  lcd.clear();
}

void loop() {
  // run loops
  cc.loop();
  ks.loop();

  // rund timed events
  if (millis() - last_time > 5000){
    last_time = millis();
    Serial.println(ks.last_message);
  }
}
