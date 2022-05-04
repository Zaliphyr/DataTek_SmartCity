/**
 * @file SW_Battery.cpp
 * @author Tobias Midthård
 * @brief This file contains the implementation of the SW_Battery class. It calculates distance driven, speed and battery.
 * @version 1.0
 * @date 2022-05-03
 */

#include "common.h"
#include "SW_Battery.h"
#include "EEPROM.h"

//*@brief the distance the left wheel has driven.
static float distanceLeft = 0;
//*@brief the distance the right wheel has driven.
static float distanceRight = 0;
//*@brief The highest mesured speed of the car.
static float maxSpeed = 0;
//*@brief The previous measured driven distance for the powerConsumption.
static float lastDistance = 0;
//*@brief The previous measured driven distance for the deltaSpeed.
static float lastValue = 0;
//*@brief The average speed in specified timeframe.
static float deltaSpeed = 0;

//*@brief The previous measured runtime of the program, used in deltaSpeed.
static unsigned long deltaTime = 0;
//*@ The previous measured runtime of the program, used in averageSpeed.
static unsigned long runTime = 0;

float SWBattery::getCarDistance()
{
    distanceLeft += float(encoders.getCountsAndResetLeft()) / 81;
    distanceRight += float(encoders.getCountsAndResetRight()) / 81;
    // Finds the average distance driven.
    float avgDistance = (distanceLeft + distanceRight) / 2;
    return avgDistance;
}

float SWBattery::calculatePowerConsumption(int dangerZone)
{
    static float batteryChargeLeft = 100;
    float currentDistance = getCarDistance();
    float powerConsumption = ((currentDistance - lastDistance) / carDrivingDistance);
    lastDistance = currentDistance;
    float batteryChargeLeft = currentPower - powerConsumption;
    if (batteryChargeLeft <= dangerZone)
    {
        updateDisplayInformation(String(batteryChargeLeft));
    }

    return batteryChargeLeft;
}

float SWBattery::calculateDeltaSpeed(float timeFrame)
{
    float millisAsFloat = millis();
    if (millis() - deltaTime >= timeFrame)
    {
        deltaSpeed = (((getCarDistance()) - lastValue)) / ((millisAsFloat - deltaTime)) / 1000);
        lastValue = getCarDistance();
        deltaTime = millis();
        return deltaSpeed;
    }
}

float SWBattery::calculateAverageCarSpeed(bool onlyWhenDriving)
{
    static float millisCompensation = millis() - 1000;
    static float drivingTime = 0;
    static float carSpeed = 0;

    if (deltaSpeed > 0)
    {
        drivingTime += millis() - millisCompensation;
    }

    millisCompensation = millis();

    if (onlyWhenDriving)
    {
        carSpeed = (getCarDistance() / (drivingTime / 1000));
    }

    else if (!onlyWhenDriving)
    {
        float runTime = millis() / 1000;
        carSpeed = (getCarDistance() / (runTime));
    }

    return carSpeed;
}

float SWBattery::calculateMaxCarSpeed()
{
    if (maxSpeed < deltaSpeed)
    {
        maxSpeed = deltaSpeed;
    }
    return maxSpeed;
}

float SWBattery::calculateSpeedOverPercent(int percent, bool onlyWhenDriving)
{
    static unsigned long millisCompensation = 0;
    static float timeOverDelta = 0;
    static float drivingTime = 0;
    static float percentAboveSpeed = 0;

    if ((calculateMaxCarSpeed() * (percent / 100)) < deltaSpeed)
    {
        timeOverDelta += millis() - millisCompensation;
    }

    if (deltaSpeed > 0)
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
