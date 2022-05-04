// Include the library containing variables, libraries and functions.
#include "common.h"

// Create a new class for the linefollower and the carController.
CarController CC;
SWBattery SWB;

// setup() is called once at the start of the program.
void setup()
{
  Serial.begin(115200);
  // calibrates the Zumo32u4 and waits for the user's input before continuing.
  CC.init();
  lcd.clear();
  runTime = millis();
  clearTime = millis();
}

// loop() is called in an endless loop.
void loop()
{
  float deltaSpeed = SWB.calculateDeltaSpeed(100);

  if (millis() - runTime >= 1000)
  {
    charge = SWB.calculatePowerConsumption(dangerZone);
    runTime = millis();
    Serial.println("Kjørt distanse: " + String(SWB.getCarDistance()));
    Serial.println("Batteriprosent: " + String(charge));
    Serial.println("Gjennomsnittsfart: " + String(SWB.calculateAverageCarSpeed(true)));
    Serial.println("Fart: " + String((deltaSpeed)));
    Serial.println("Maksimal fart: " + String(SWB.calculateMaxCarSpeed()));
    Serial.println("Fart over 70% av maksimal fart: " + String(SWB.calculateSpeedOverPercent(70, true)));
    Serial.println("------------------------------------");
    if (millis() - clearTime >= 2000)
    {
      lcd.clear();
      clearTime = millis();
    }
  }

  if (charge > 0)
  {
    CC.loop();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CC.cc_state);
    lcd.setCursor(0, 1);
    lcd.print(CC.step);
  }

  else if (charge <= 0)
  {
    motors.setSpeeds(0, 0);
  }
}
