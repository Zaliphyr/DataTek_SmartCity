#include "SW_Battery.h"
#include "EEPROM.h"
#include "common.h"

static float distanceLeft = 0;
static float distanceRight = 0;
static float maxSpeed = 0;
static int lastDistance = 0;
static float lastValue = 0;

static unsigned long deltaTime = 0;

float SWBattery::getCarDistance()
{
    distanceLeft += float(encoders.getCountsAndResetLeft()) / 81;
    distanceRight += float(encoders.getCountsAndResetRight()) / 81;
    float avgDistance = (distanceLeft + distanceRight) / 2;
    return avgDistance;
}

float SWBattery::calculatePowerConsumption(int currentPower, int dangerZone)
{
    float powerConsumption = ((getCarDistance() - lastDistance) / carDrivingDistance);
    // Serial.println(powerConsumption);
    // Serial.println(currentPower);
    lastDistance = getCarDistance();
    float batteryChargeLeft = currentPower - powerConsumption;
    if (batteryChargeLeft <= dangerZone)
    {
        updateDisplayInformation(String(batteryChargeLeft));
    }

    return batteryChargeLeft;
}

float SWBattery::calculateDeltaSpeed(float timeFrame)
{
    if (millis() - deltaTime >= timeFrame)
    {
        float deltaSpeed = (((getCarDistance()) - lastValue)) / ((millis() - deltaTime) / 1000);
        lastValue = getCarDistance();
        deltaTime = millis();
        return deltaSpeed;
    }
}

float SWBattery::calculateAverageCarSpeed()
{
    static float runTime = millis() / 1000;
    float carSpeed = ((getCarDistance()) / (runTime));
    return carSpeed;
}

float SWBattery::calculateMaxCarSpeed()
{
    float carSpeed = calculateDeltaSpeed(100);
    if (maxSpeed < carSpeed)
    {
        maxSpeed = carSpeed;
    }
    return maxSpeed;
}

float SWBattery::calculateSpeedOverPercent(int percent, bool onlyWhenDriving)
{
    static unsigned long millisCompensation = 0;
    static float timeOverDelta = 0;
    static float drivingTime = 0;
    static float percentAboveSpeed = 0;

    int currentSpeed = (calculateDeltaSpeed(100));

    if ((calculateMaxCarSpeed() * (percent / 100)) < currentSpeed)
    {
        timeOverDelta += millis() - millisCompensation;
    }

    if (currentSpeed > 0)
    {
        drivingTime += millis() - millisCompensation;
    }

    millisCompensation = millis();
    if (onlyWhenDriving)
    {
        percentAboveSpeed = ((timeOverDelta / drivingTime) * 100);
    }
    else if (!onlyWhenDriving)
    {
        percentAboveSpeed = ((timeOverDelta / millisCompensation) * 100);
    }

    return percentAboveSpeed;
}

void chargingTimes(String status)
{
    static int chargingTimes = 0;

    if (status == "charging")
    {
        chargingTimes++;
        EEPROM.write(0, chargingTimes);
    }
}

float batteryHealth()
{
    static const int normalMaxCharge = 100;

    int chargingTimes = int(EEPROM.read(0));

    float maxCharge = normalMaxCharge - pow(0.9f, chargingTimes);

    return maxCharge;
}

void SWBattery::updateDisplayInformation(String displayText)
{
    lcd.setCursor(0, 0);
    lcd.print(String(displayText));
}
