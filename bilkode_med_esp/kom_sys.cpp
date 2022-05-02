#include "common.h"
#include "kom_sys.h"
#include <Arduino.h>

void kom_sys::init()
{
    // Starts Serial 1
    Serial1.begin(115200);
    // Waits until ESP is online and ready
    while (!Serial1.available())
    {
        
    }
    Serial1.flush();
}

void kom_sys::loop()
{
    // Checks if message is arriving on Serial
    String tempS = "";
    while (Serial1.available()){
        delay(1);
        char c = Serial1.read();
        tempS += c;
    }
    // If message has arrived, update last_message
    if (tempS != last_message && tempS != ""){
        last_message = tempS;
        Serial1.flush();
    }
}

void kom_sys::send_value(String topic, String value)
{
    // Makes a string with comma between
    String message = topic + "," + value;
    // Sends string to ESP
    Serial1.print(message);
}