#pragma once

// Define the class
#define SW_Battery_h

// Includes needed for this file:
#include "common.h"

//* Controls the car:
/* Finds the distance between.
 Calculates the power consmuption of the car.
 Updates the car's display to show information.
*/
class SWBattery
{
public:
    /**
     * @brief Finds the distance the car has traversed with the help of the Zumo32u4's built in encoders.
     *
     * @return the average distance driven in meter.
     */
    float getCarDistance();

    /**
     * @brief Calculates the power consumption of the car.
     *
     * @param currentPower the current power of the car.
     * @param dangerZone When the user should be notified about the battery level.
     * @return Battery charge remaining in %.
     */
    float calculatePowerConsumption(int currentPower, int dangerZone);

    /**
     * @brief Calculates the average speed over a time frame.
     *
     * @param timeFrame The time frame of the calculation in ms.
     * @return The average speed over the time frame.
     */
    float calculateDeltaSpeed(float timeFrame);

    /**
     * @brief Calculates the average speed of the car.
     *
     * @return the average speed of the car in cm/s.
     */
    float calculateAverageCarSpeed();

    /**
     * @brief Calculates the maximum speed of the car.
     *
     * @return The maximum speed of the car in cm/s.
     */
    float calculateMaxCarSpeed();

    /**
     * @brief Calculates how many seconds the car drives over a set percentage of max speed.
     *
     * @param percent The percentage of the driving distance.
     * @param onlyWhenDriving If the percentage only accounts for drivning and not standstil.
     * @return Time the car drove over the percent of maximum speed in seconds.
     */
    float calculateSpeedOverPercent(int percent, bool onlyWhenDriving);

    /**
     * @brief Updates the amount of times the car has charged.
     *
     * @param status the status of the car.
     */
    void chargingTimes(String status);

    /**
     * @brief Calculates the maximum of charge the car can hold.
     *
     * @return return max charge.
     */
    float batteryHealth();

    /**
     * @brief Updates the display with set information.
     *
     * @param String The displayed information.
     */
    void updateDisplayInformation(String displayText);
};